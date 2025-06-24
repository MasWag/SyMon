#pragma once

#include <istream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <sstream>

#include "common_types.hh"
#include "tree_sitter/api.h"
#include "tree_sitter/tree-sitter-symon.h"

#include "automata_operation.hh"
#include "signature.hh"

namespace boost {
    using ::operator>>;
    using ::operator<<;
    using namespace Parma_Polyhedra_Library::IO_Operators;
}

namespace std {
    using namespace Parma_Polyhedra_Library::IO_Operators;
}

#include "io_operators.hh"
#include "automaton_parser.hh"

#include <boost/lexical_cast.hpp>

inline std::string makeErrorMessage(const char message[], const std::string &content, const TSNode& node) {
    std::stringstream ss;
    ss << message;
    // Compute the line and position
    const uint32_t startByte = ts_node_start_byte(node);
    const uint32_t endByte = ts_node_end_byte(node);
    const uint32_t startLine = ts_node_start_point(node).row + 1; // Tree-sitter uses 0-based indexing
    const uint32_t startColumn = ts_node_start_point(node).column + 1; // Tree-sitter uses 0-based indexing
    const uint32_t endLine = ts_node_end_point(node).row + 1; // Tree-sitter uses 0-based indexing
    const uint32_t endColumn = ts_node_end_point(node).column + 1; // Tree-sitter uses 0-based indexing
    ss << " at line " << startLine << ", column " << startColumn;
    if (startLine != endLine || startColumn != endColumn) {
        ss << " to line " << endLine << ", column " << endColumn;
    }

    ss << " (bytes " << startByte << ", " << endByte << ")";

    return ss.str();
}

// Helper to skip over comments when iterating child nodes
static inline TSNode nextNonCommentChild(const TSNode &parent, uint32_t &idx) {
    uint32_t count = ts_node_child_count(parent);
    while (idx < count) {
        TSNode c = ts_node_child(parent, idx);
        std::string t = ts_node_type(c);
        if (t != "comment" && t != "line_comment") {
            return c;
        }
        ++idx;
    }
    throw std::runtime_error("Expected a non-comment child but reached end");
}

template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
class SymonParser {
public:
    using Automaton = TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update>;
    using State = AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>;

    struct RawSignature {
        std::string name;
        std::vector<std::string> stringVariables;
        std::vector<std::string> numberVariables;
    };

    static Signature asSignature(std::vector<RawSignature> rawSignatures) {
        std::unordered_map<std::string, std::size_t> idMap;
        std::unordered_map<std::string, std::size_t> stringSizeMap;
        std::unordered_map<std::string, std::size_t> numberSizeMap;
        std::size_t id = 0;
        for (const auto &[name, stringVariables, numberVariables]: rawSignatures) {
            idMap[name] = id++;
            stringSizeMap[name] = stringVariables.size();
            numberSizeMap[name] = numberVariables.size();
        }

        return Signature{idMap, stringSizeMap, numberSizeMap};
    }

    void parse(std::istream &istream) {
        const std::istreambuf_iterator begin(istream);
        constexpr std::istreambuf_iterator<char> end;
        std::string content(begin, end);
        this->parse(content);
    }

    void parse(const std::string &content) {
        TSParser *inParser = ts_parser_new();
        ts_parser_set_language(inParser, tree_sitter_symon());
        const TSTree *tree = ts_parser_parse_string(inParser, nullptr, content.c_str(), content.length());
        const TSNode rootNode = ts_tree_root_node(tree);
        const uint32_t rootSize = ts_node_child_count(rootNode);
        for (uint32_t i = 0; i < rootSize; i++) {
            TSNode child = ts_node_child(rootNode, i);
            if (ts_node_type(child) == std::string("variables")) {
                this->parseVariables(content, child);
            } else if (ts_node_type(child) == std::string("signature")) {
                this->signatures.push_back(makeSignature(content, child));
            } else if (ts_node_type(child) == std::string("initial_constraints")) {
                this->parseInits(content, child);
            } else if (ts_node_type(child) == std::string("def_expr")) {
                const TSNode idNode = ts_node_child(child, 1);
                auto id = std::string(content.begin() + ts_node_start_byte(idNode),
                                      content.begin() + ts_node_end_byte(idNode));
                TSNode innerNode = ts_node_child(child, 3);
                this->automata[id] = this->parseExpr(content, innerNode);
            } else if (ts_node_type(child) == std::string("expr")) {
                this->expr = this->parseExpr(content, child);
            }
        }

        // Handle initial constraints by creating a new initial state and unobservable transitions
        if ((!this->initialStringConstraints.empty() || !this->initialNumberConstraints.empty()) && this->expr) {
            auto newInitialState = std::make_shared<State>(false);
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> >
                    newTransitions;
            newTransitions.reserve(this->initialStringConstraints.size());
            for (const auto &initialState: this->expr->initialStates) {
                AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> newTransition;
                newTransition.target = initialState;
                newTransition.stringConstraints = this->initialStringConstraints;
                newTransition.numConstraints = this->initialNumberConstraints;
                newTransitions.push_back(std::move(newTransition));
            }
            this->expr->states.push_back(newInitialState);
            this->expr->initialStates = {newInitialState};
            // The magic number 127 is used to represent unobservable transitions
            newInitialState->next[127] = std::move(newTransitions);
        }
    }

    [[nodiscard]] Signature makeSignature() const {
        if (this->signatures.empty()) {
            throw std::runtime_error("Signature not set");
        }
        return asSignature(this->signatures);
    }

    Automaton getAutomaton() const {
        if (this->expr.has_value()) {
            return this->expr.value();
        }
        throw std::runtime_error("Automaton not set");
    }

    void setGlobalData(Automaton &automaton) const {
        automaton.stringVariableSize = this->globalStringVariables.size();
        automaton.numberVariableSize = this->globalNumberVariables.size();
        // Set parameterSize only if automaton.parameterSize exists
        if constexpr (std::is_same_v<TimingConstraint, ParametricTimingConstraint>) {
            automaton.parameterSize = this->parameters.size();
        }
    }

public:
    // Helper function to extract the upper bound from a non-parametric timing constraint
    static std::vector<::TimingConstraint> extractUpperBound(const std::vector<::TimingConstraint>& guard) {
        std::vector<::TimingConstraint> upperBound;
        
        // Check each constraint in the guard
        for (const auto& constraint : guard) {
            // If the constraint is an upper bound (lt or le), add it to the result
            if (constraint.odr == ::TimingConstraint::Order::lt ||
                constraint.odr == ::TimingConstraint::Order::le) {
                upperBound.push_back(constraint);
            }
        }
        
        return upperBound;
    }
    
    // Helper function to extract the upper bound from a parametric timing constraint
    static ParametricTimingConstraint extractUpperBound(const ParametricTimingConstraint& guard, const VariableID clockIndex = 0) {
        // Check if the clock index is within the bounds of the constraint's space dimension
        if (clockIndex >= guard.space_dimension()) {
            // If the clock index is out of bounds, return the UNIVERSE Polyhedron
            return ParametricTimingConstraint(guard.space_dimension(), Parma_Polyhedra_Library::UNIVERSE);
        }
        
        // Create a new polyhedron for the upper bounds
        ParametricTimingConstraint upperBound(guard.space_dimension(), Parma_Polyhedra_Library::UNIVERSE);
        
        // Get the constraint system from the guard
        Parma_Polyhedra_Library::Constraint_System cs = guard.constraints();
        
        // Iterate through the constraints
        for (Parma_Polyhedra_Library::Constraint_System::const_iterator it = cs.begin(); it != cs.end(); ++it) {
            const Parma_Polyhedra_Library::Constraint& c = *it;
            
            // Check if it's a less-than-or-equal constraint (upper bound)
            // The clock variable index is determined by the innerExpr.clockVariableSize
            if (c.is_inequality() && c.coefficient(Parma_Polyhedra_Library::Variable(clockIndex)) < 0) {
                // This is an upper bound constraint (standard form: ax + by + ... <= c)
                // where the coefficient of x is negative (which means x <= ...)
                upperBound.add_constraint(c);
            }
        }
        
        return upperBound;
    }
    
private:
    // template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
    /*!
     * @brief Returns if the last clock variable is reset at any transition.
     */
    static bool noResetLastClock(const Automaton& automaton) {
        const VariableID lastClockId = automaton.clockVariableSize - 1;
        for (const auto &state: automaton.states) {
            for (const auto &[_, transitions]: state->next) {
                for (const auto &transition: transitions) {
                    if (std::find(transition.resetVars.begin(), transition.resetVars.end(), lastClockId) != transition.resetVars.end()) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    static std::string parseDeclaration(const std::string &content, const TSNode &declarationNode) {
        if (ts_node_type(declarationNode) != std::string("string_definition") && ts_node_type(declarationNode) !=
            std::string("number_definition") && ts_node_type(declarationNode) != std::string("parameter_definition")) {
            throw std::runtime_error("Expected declaration node");
        }
        const uint32_t nodeSize = ts_node_child_count(declarationNode);
        for (uint32_t i = 0; i < nodeSize; i++) {
            if (const TSNode child = ts_node_child(declarationNode, i);
                ts_node_type(child) == std::string("identifier")) {
                return std::string(content.begin() + ts_node_start_byte(child),
                                   content.begin() + ts_node_end_byte(child));
            }
        }
        throw std::runtime_error("Declaration node does not contain identifier");
    }

    void parseVariables(const std::string &content, const TSNode &declarationNode) {
        if (ts_node_type(declarationNode) != std::string("variables")) {
            throw std::runtime_error("Expected variable declaration block node");
        }
        const uint32_t nodeSize = ts_node_child_count(declarationNode);
        for (uint32_t i = 0; i < nodeSize; i++) {
            if (ts_node_type(ts_node_child(declarationNode, i)) == std::string("string_definition")) {
                this->globalStringVariables.push_back(parseDeclaration(content, ts_node_child(declarationNode, i)));
            } else if (ts_node_type(ts_node_child(declarationNode, i)) == std::string("number_definition")) {
                this->globalNumberVariables.push_back(parseDeclaration(content, ts_node_child(declarationNode, i)));
            } else if (ts_node_type(ts_node_child(declarationNode, i)) == std::string("parameter_definition")) {
                this->parameters.push_back(parseDeclaration(content, ts_node_child(declarationNode, i)));
            }
        }
    }

    // Extract the signature from the parse tree.
    static RawSignature makeSignature(const std::string &content, const TSNode &signatureNode) {
        if (ts_node_type(signatureNode) != std::string("signature")) {
            throw std::runtime_error(makeErrorMessage("Expected signature node", content, signatureNode));
        }
        const uint32_t nodeSize = ts_node_child_count(signatureNode);
        std::string name;
        std::vector<std::string> stringVariables;
        std::vector<std::string> numberVariables;
        for (uint32_t i = 0; i < nodeSize; i++) {
            const TSNode child = ts_node_child(signatureNode, i);
            std::string childType = ts_node_type(child);
            if (childType == "identifier") {
                name = std::string(content.begin() + ts_node_start_byte(child),
                                   content.begin() + ts_node_end_byte(child));
            } else if (childType == "string_definition") {
                stringVariables.push_back(parseDeclaration(content, child));
            } else if (childType == "number_definition") {
                numberVariables.push_back(parseDeclaration(content, child));
            } else if (childType == "signature" || childType == "{" || childType == "}") {
                continue;
            } else if (childType == "ERROR") {
                throw std::runtime_error(makeErrorMessage("Syntax error", content, child));
            } else {
                throw std::runtime_error(makeErrorMessage(("Expected identifier, string_definition, or number_definition node in signature but got: " + childType).c_str(), content, child));
            }
        }

        return RawSignature{name, stringVariables, numberVariables};
    }

    static std::optional<TSNode> ts_node_child_by_type(const TSNode &initialConstraintsNode, const std::string &type) {
        const uint32_t nodeSize = ts_node_child_count(initialConstraintsNode);
        for (uint32_t i = 0; i < nodeSize; i++) {
            if (const TSNode child = ts_node_child(initialConstraintsNode, i); ts_node_type(child) == type) {
                return child;
            }
        }
        return std::nullopt;
    }

    void processStringAtomic(std::string &atomic, const std::string &type) const {
        if (type == std::string("identifier")) {
            auto it = std::find(this->globalStringVariables.begin(), globalStringVariables.end(), atomic);
            if (it == this->globalStringVariables.end()) {
                if (this->localStringVariables) {
                    it = std::find(this->localStringVariables->begin(), this->localStringVariables->end(), atomic);
                    if (it != this->localStringVariables->end()) {
                        atomic = "x" + std::to_string(
                                     this->globalStringVariables.size() + std::distance(
                                         this->localStringVariables->cbegin(), it));
                        return;
                    }
                }
                throw std::runtime_error("Undeclared string variable: " + atomic);
            }
            atomic = "x" + std::to_string(std::distance(this->globalStringVariables.cbegin(), it));
        } else {
            // Replace double quotes with single quotes in the string literal
            for (char &c: atomic) {
                if (c == '"') {
                    c = '\'';
                }
            }
        }
    }

    void processNumericAtomic(std::string &atomic, const std::string &type) const {
        if (type == std::string("identifier")) {
            auto it = std::find(this->globalNumberVariables.begin(), this->globalNumberVariables.end(), atomic);
            if (it == this->globalNumberVariables.end()) {
                if (this->localNumberVariables) {
                    it = std::find(this->localNumberVariables->begin(), this->localNumberVariables->end(), atomic);
                    if (it != this->localNumberVariables->end()) {
                        atomic = "x" + std::to_string(
                                     this->globalNumberVariables.size() + std::distance(
                                         this->localNumberVariables->cbegin(), it));
                        return;
                    }
                }
                throw std::runtime_error("Undeclared number variable: " + atomic);
            }
            atomic = "x" + std::to_string(std::distance(this->globalNumberVariables.cbegin(), it));
        }
    }

    std::string parseNumericExpr(const std::string &content, const TSNode &constraintNode) const {
        const uint32_t nodeSize = ts_node_child_count(constraintNode);
        if (nodeSize == 0) {
            auto atomic = std::string(content.begin() + ts_node_start_byte(constraintNode),
                                      content.begin() + ts_node_end_byte(constraintNode));
            this->processNumericAtomic(atomic, ts_node_type(constraintNode));
            return atomic;
        }
        if (nodeSize == 1) {
            return this->parseNumericExpr(content, ts_node_child(constraintNode, 0));
        }
        if (nodeSize == 3 && ts_node_type(ts_node_child(constraintNode, 0)) == std::string("(")) {
            // The case with parentheses, e.g., "(a + b)"
            const TSNode exprNode = ts_node_child(constraintNode, 1);
            const std::string expr = this->parseNumericExpr(content, exprNode);
            return "(" + expr + ")";
        }
        if (nodeSize == 3) {
            const TSNode lhsNode = ts_node_child(constraintNode, 0);
            const TSNode opNode = ts_node_child(constraintNode, 1);
            const TSNode rhsNode = ts_node_child(constraintNode, 2);
            const std::string lhs = this->parseNumericExpr(content, lhsNode);
            const auto op = std::string(content.begin() + ts_node_start_byte(opNode),
                                        content.begin() + ts_node_end_byte(opNode));
            if (op != "+" && op != "-" && op != "*" && op != "/") {
                throw std::runtime_error("Expected operator to be '+' or '-' but got: " + op);
            }
            const std::string rhs = this->parseNumericExpr(content, rhsNode);
            return lhs + " " + op + " " + rhs;
        }
        throw std::runtime_error(
            "Expected numeric expression to have 0 or 3 children, but got: " + std::to_string(nodeSize));
    }

    StringConstraint parseStringConstraint(const std::string &content, const TSNode &constraintNode) const {
        if (ts_node_type(constraintNode) != std::string("string_constraint")) {
            throw std::runtime_error("Expected string_constraint node");
        }
        if (ts_node_child_count(constraintNode) != 3) {
            throw std::runtime_error("Expected string_constraint to have exactly 3 children");
        }
        const TSNode lhsNode = ts_node_child(constraintNode, 0);
        const TSNode opNode = ts_node_child(constraintNode, 1);
        const TSNode rhsNode = ts_node_child(constraintNode, 2);

        std::string lhs = std::string(content.begin() + ts_node_start_byte(lhsNode),
                                      content.begin() + ts_node_end_byte(lhsNode));
        this->processStringAtomic(lhs, ts_node_type(lhsNode));
        std::string op = std::string(content.begin() + ts_node_start_byte(opNode),
                                     content.begin() + ts_node_end_byte(opNode));
        if (op != "==" && op != "!=") {
            throw std::runtime_error("Expected operator to be '==' or '!=' but got: " + op);
        }
        std::string rhs = std::string(content.begin() + ts_node_start_byte(rhsNode),
                                      content.begin() + ts_node_end_byte(rhsNode));
        this->processStringAtomic(rhs, ts_node_type(rhsNode));
        return boost::lexical_cast<StringConstraint>(lhs + " " + op + " " + rhs);
    }

    NumberConstraint parseNumericConstraint(const std::string &content, const TSNode &constraintNode) const {
        if (ts_node_type(constraintNode) != std::string("numeric_constraint")) {
            throw std::runtime_error("Expected numeric_constraint node");
        }
        if (ts_node_child_count(constraintNode) != 3) {
            std::cout << ts_node_child_count(constraintNode) << std::endl;
            std::string innerContent =
                    std::string(content.begin() + ts_node_start_byte(constraintNode),
                                content.begin() + ts_node_end_byte(constraintNode));
            std::cout << "Inner content: " << innerContent << std::endl;
            throw std::runtime_error("Expected numeric_constraint to have exactly 3 children");
        }
        const TSNode lhsNode = ts_node_child(constraintNode, 0);
        const TSNode opNode = ts_node_child(constraintNode, 1);
        const TSNode rhsNode = ts_node_child(constraintNode, 2);

        const std::string lhs = this->parseNumericExpr(content, lhsNode);
        auto op = std::string(content.begin() + ts_node_start_byte(opNode),
                              content.begin() + ts_node_end_byte(opNode));
        if (op != "=" && op != "<>" && op != "<" && op != "<=" && op != ">" && op != ">=") {
            throw std::runtime_error("Expected operator to be '=', '<>', '<', '<=', '>', or '>=' but got: " + op);
        }
        if (op == "=") {
            op = "==";
        } else if (op == "<>") {
            op = "!=";
        }
        const std::string rhs = this->parseNumericExpr(content, rhsNode);
        return boost::lexical_cast<NumberConstraint>(lhs + " " + op + " " + rhs);
    }

    void processTimeAtomic(std::string &atomic, const std::string &type) const {
        if (type == std::string("identifier")) {
            auto it = std::find(this->parameters.begin(), this->parameters.end(), atomic);
            if (it == this->parameters.end()) {
                throw std::runtime_error("Undeclared timing parameter: " + atomic);
            }
            atomic = "p" + std::to_string(std::distance(this->parameters.begin(), it));
        }
    }

    std::string parseTimeExpr(const std::string &content, const TSNode &constraintNode) const {
        const uint32_t nodeSize = ts_node_child_count(constraintNode);
        if (nodeSize == 0) {
            auto atomic = std::string(content.begin() + ts_node_start_byte(constraintNode),
                                      content.begin() + ts_node_end_byte(constraintNode));
            this->processTimeAtomic(atomic, ts_node_type(constraintNode));
            return atomic;
        }
        if (nodeSize == 1) {
            return parseTimeExpr(content, ts_node_child(constraintNode, 0));
        }
        if (nodeSize == 3 && ts_node_type(ts_node_child(constraintNode, 0)) == std::string("(")) {
            // The case with parentheses, e.g., "(a + b)"
            const TSNode exprNode = ts_node_child(constraintNode, 1);
            const std::string expr = this->parseTimeExpr(content, exprNode);
            return "(" + expr + ")";
        }
        if (nodeSize == 3) {
            const TSNode lhsNode = ts_node_child(constraintNode, 0);
            const TSNode opNode = ts_node_child(constraintNode, 1);
            const TSNode rhsNode = ts_node_child(constraintNode, 2);
            const std::string lhs = this->parseTimeExpr(content, lhsNode);
            const auto op = std::string(content.begin() + ts_node_start_byte(opNode),
                                        content.begin() + ts_node_end_byte(opNode));
            if (op != "+" && op != "-") {
                throw std::runtime_error("Expected operator to be '+' or '-' but got: " + op);
            }
            const std::string rhs = this->parseTimeExpr(content, rhsNode);
            return lhs + " " + op + " " + rhs;
        }
        throw std::runtime_error(
            "Expected numeric expression to have 0 or 3 children, but got: " + std::to_string(nodeSize));
    }

    void parseConstraintList(const std::string &content, TSNode parent,
                             std::vector<StringConstraint> &stringConstraints,
                             std::vector<NumberConstraint> &numberConstraints) const {
        if (ts_node_type(parent) != std::string("constraint_list")) {
            throw std::runtime_error("Expected constraint_list node");
        }
        uint32_t nodeSize = ts_node_child_count(parent);
        while (true) {
            if (nodeSize == 0) {
                break;
            }
            const TSNode child = ts_node_child(parent, 0);
            if (ts_node_type(child) != std::string("constraint")) {
                throw std::runtime_error("Expected constraint node as first child of constraint_list");
            }
            if (TSNode constraint = ts_node_child(child, 0);
                ts_node_type(constraint) == std::string("string_constraint")) {
                stringConstraints.push_back(this->parseStringConstraint(content, constraint));
            } else if (ts_node_type(constraint) == std::string("numeric_constraint")) {
                numberConstraints.push_back(this->parseNumericConstraint(content, constraint));
            }

            if (nodeSize == 3) {
                parent = ts_node_child(parent, 2);
                nodeSize = ts_node_child_count(parent);
            } else {
                break;
            }
        }
    }

    void parseInits(const std::string &content, const TSNode &initialConstraintsNode) {
        if (ts_node_type(initialConstraintsNode) != std::string("initial_constraints")) {
            throw std::runtime_error("Expected initial constraints node");
        }
        if constexpr (!std::is_same_v<TimingConstraint, ParametricTimingConstraint>) {
            throw std::runtime_error("Giving initial constraints is only supported for parametric timing constraints");
        }
        // Find a child node with type "constraint_list"
        std::optional<TSNode> parent = ts_node_child_by_type(initialConstraintsNode, "constraint_list");
        if (!parent.has_value()) {
            return;
        }
        this->parseConstraintList(content, parent.value(),
                                  this->initialStringConstraints, this->initialNumberConstraints);
    }

    TimingConstraint parseTimingConstraint(const std::string &content, const TSNode &constraintNode,
                                           const std::size_t clockIndex) const {
        if (ts_node_type(constraintNode) != std::string("timing_constraint")) {
            throw std::runtime_error("Expected timing_constraint node");
        }
        TSNode parent = ts_node_child(constraintNode, 0);
        if (ts_node_type(parent) == std::string("intervals")) {
            std::string lowerBound = this->parseTimeExpr(content, ts_node_child(parent, 1));
            std::string upperBound = this->parseTimeExpr(content, ts_node_child(parent, 3));
            bool isLowerInclusive = ts_node_type(ts_node_child(parent, 0)) == std::string("[");
            bool isUpperInclusive = ts_node_type(ts_node_child(parent, 4)) == std::string("]");

            if constexpr (std::is_same_v<TimingConstraint, ParametricTimingConstraint>) {
                auto lowerGuard = boost::lexical_cast<ParametricTimingConstraintHelper>(
                    "x" + std::to_string(clockIndex) + " " + (isLowerInclusive ? ">=" : ">") + " " + lowerBound);
                auto upperGuard = boost::lexical_cast<ParametricTimingConstraintHelper>(
                    "x" + std::to_string(clockIndex) + " " + (isUpperInclusive ? "<=" : "<") + " " + upperBound);
                Parma_Polyhedra_Library::Constraint lowerConstraint, upperConstraint;
                lowerGuard.extract(this->parameters.size(), lowerConstraint);
                upperGuard.extract(this->parameters.size(), upperConstraint);
                ParametricTimingValuation result{this->parameters.size() + clockIndex + 1};
                result.add_constraint(lowerConstraint);
                result.add_constraint(upperConstraint);
                return result;
            } else {
                return {
                    boost::lexical_cast<::TimingConstraint>(
                        "x" + std::to_string(clockIndex) + " " + (isLowerInclusive ? ">=" : ">") + " " + lowerBound),
                    boost::lexical_cast<::TimingConstraint>(
                        "x" + std::to_string(clockIndex) + " " + (isUpperInclusive ? "<=" : "<") + " " + upperBound)
                };
            }
        }
        if (ts_node_type(parent) == std::string("half_guard")) {
            const TSNode comparatorNode = ts_node_child(parent, 1);
            auto comparator = std::string(content.begin() + ts_node_start_byte(comparatorNode),
                                          content.begin() + ts_node_end_byte(comparatorNode));
            if (comparator != "<" && comparator != "<=" && comparator != ">" && comparator != ">=" && comparator != "="
                && comparator != "<>") {
                throw std::runtime_error(
                    "Expected comparator to be '<', '<=', '>', '>=', '=', or '<>', but got: " + comparator);
            }
            if (comparator == "=") {
                comparator = "==";
            } else if (comparator == "<>") {
                comparator = "!=";
            }
            const std::string expr = this->parseTimeExpr(content, ts_node_child(parent, 2));
            if constexpr (std::is_same_v<TimingConstraint, ParametricTimingConstraint>) {
                auto guard = boost::lexical_cast<ParametricTimingConstraintHelper>(
                    "x" + std::to_string(clockIndex) + " " + comparator + " " + expr);
                Parma_Polyhedra_Library::Constraint constraint;
                guard.extract(this->parameters.size(), constraint);
                ParametricTimingValuation result{this->parameters.size() + clockIndex + 1};
                result.add_constraint(constraint);
                return result;
            } else {
                return {
                    boost::lexical_cast<::TimingConstraint>(
                        "x" + std::to_string(clockIndex) + " " + comparator + " " + expr)
                };
            }
        }
        throw std::runtime_error("Expected half_guard node or intervals node, but got: " + std::string(ts_node_type(parent)));
    }

    std::vector<Action> parseActionList(const std::string &content, const TSNode &identifierListNode) const {
        const auto size = (ts_node_child_count(identifierListNode) + 1) / 2;
        std::vector<Action> actions;
        actions.reserve(size);
        for (int i = 0; i < size; ++i) {
            TSNode identityNode = ts_node_child(identifierListNode, i * 2);
            auto identifier =
                std::string(content.begin() + ts_node_start_byte(identityNode),
                            content.begin() + ts_node_end_byte(identityNode));
            auto it =
                std::find_if(this->signatures.begin(), this->signatures.end(),
                             [&identifier](const RawSignature &sig) {
                               return sig.name == identifier;
                             });
            if (it == this->signatures.end()) {
                throw std::runtime_error("Undeclared action: " + identifier);
            }
            actions.emplace_back(std::distance(this->signatures.begin(), it));
        }

        return actions;
    }

    Automaton parseExpr(const std::string &content, const TSNode &exprNode) {
        if (ts_node_type(exprNode) != std::string("expr")) {
            throw std::runtime_error(makeErrorMessage((std::string("Expected expr node but found ") + ts_node_type(exprNode)).c_str(), content, exprNode));
        }
        if (const uint32_t nodeSize = ts_node_child_count(exprNode); nodeSize != 1) {
            throw std::runtime_error(
                "Expected expr node to have exactly one child, but got: " + std::to_string(nodeSize));
        }

        const TSNode child = ts_node_child(exprNode, 0);
        std::string kind = ts_node_type(child);
        if (kind == "identifier") {
            const auto identifier = std::string(content.begin() + ts_node_start_byte(child),
                                                content.begin() + ts_node_end_byte(child));
            auto it = this->automata.find(identifier);
            if (it == this->automata.end()) {
                throw std::runtime_error("Undeclared automaton: " + identifier);
            }
            return it->second.deepCopy();
        } else if (kind == "atomic") {
            const TSNode identifierNode = ts_node_child(child, 0);
            if (ts_node_type(identifierNode) != std::string("identifier")) {
                throw std::runtime_error("Expected atomic node to have identifier child");
            }
            auto signatureName = std::string(content.begin() + ts_node_start_byte(identifierNode),
                                             content.begin() + ts_node_end_byte(identifierNode));

            bool isUnobservable = (signatureName == "unobservable");
            auto guardNodeOpt = this->ts_node_child_by_type(child, "guard_block");
            bool hasGuardContent = false;
            if (guardNodeOpt.has_value()) {
                hasGuardContent = this->ts_node_child_by_type(*guardNodeOpt, "constraint_list").has_value() ||
                                   this->ts_node_child_by_type(*guardNodeOpt, "assignment_list").has_value();
            }
            if (isUnobservable && !hasGuardContent) {
                Automaton result;
                this->setGlobalData(result);
                result.states.reserve(1);
                auto state = std::make_shared<State>(true);
                result.states.push_back(state);
                result.initialStates.push_back(state);
                return result;
            }

            RawSignature *signature = nullptr;
            std::size_t signatureId = 0;
            if (!isUnobservable) {
                for (auto &sig: this->signatures) {
                    if (sig.name == signatureName) {
                        signature = &sig;
                        break;
                    }
                    signatureId++;
                }
                if (!signature) {
                    throw std::runtime_error("Undeclared automaton signature: " + signatureName);
                }
                this->localStringVariables = &signature->stringVariables;
                this->localNumberVariables = &signature->numberVariables;
            } else {
                signatureId = 127;
                static std::vector<std::string> emptyStringVars;
                static std::vector<std::string> emptyNumberVars;
                this->localStringVariables = &emptyStringVars;
                this->localNumberVariables = &emptyNumberVars;
            }

            Automaton result = Automaton();
            this->setGlobalData(result);
            result.states.reserve(2);
            auto initialState = std::make_shared<State>(false);
            auto finalState = std::make_shared<State>(true);
            result.states.push_back(initialState);
            result.states.push_back(finalState);
            result.initialStates.push_back(initialState);
            AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> transition;
            std::optional<TSNode> argNode = this->ts_node_child_by_type(child, "arg_list");
            std::optional<TSNode> constraintNode;
            std::optional<TSNode> updateNode;
            if (std::optional<TSNode> guardNode = ts_node_child_by_type(child, "guard_block"); guardNode.has_value()) {
                constraintNode = ts_node_child_by_type(guardNode.value(), "constraint_list");
                updateNode = ts_node_child_by_type(guardNode.value(), "assignment_list");
                if (constraintNode.has_value()) {
                    this->parseConstraintList(content, constraintNode.value(),
                                              transition.stringConstraints, transition.numConstraints);
                }
                if (updateNode.has_value()) {
                    const std::size_t updateSize = ts_node_child_count(*updateNode);
                    for (int i = 0; i < updateSize; ++i ) {
                        TSNode update = ts_node_child(*updateNode, i);
                        if (ts_node_type(update) != std::string("assignment")) {
                            continue;
                        }
                        // Obtain the identifier as string
                        TSNode identifierNode = ts_node_child(update, 0);
                        if (ts_node_type(identifierNode) != std::string("identifier")) {
                            throw std::runtime_error("Expected assignment node to have identifier child");
                        }
                        auto identifier = std::string(content.begin() + ts_node_start_byte(identifierNode),
                                                      content.begin() + ts_node_end_byte(identifierNode));
                        // Check if the identifier is a string or number variable. Here, the assigned value must be global.
                        if (auto it = std::find(this->globalStringVariables.begin(), this->globalStringVariables.end(), identifier); it != this->globalStringVariables.end()) {
                            // String variable assignment
                            auto id = std::distance(this->globalStringVariables.begin(), it);
                            TSNode valueNode = ts_node_child(update, 2);
                            auto value = std::string(content.begin() + ts_node_start_byte(valueNode),
                                                     content.begin() + ts_node_end_byte(valueNode));
                            this->processStringAtomic(value, ts_node_type(valueNode));
                            std::string asString = "x" + std::to_string(id) + " := " + value;
                            transition.update.stringUpdate.push_back(boost::lexical_cast<std::remove_reference_t<decltype(transition.update.stringUpdate.front())> >(asString));
                        } else if (auto it = std::find(this->globalNumberVariables.begin(), this->globalNumberVariables.end(), identifier); it != this->globalNumberVariables.end()) {
                            // Number variable assignment
                            auto id = std::distance(this->globalNumberVariables.begin(), it);
                            TSNode valueNode = ts_node_child(update, 2);
                            auto value = this->parseNumericExpr(content, valueNode);
                            std::string asString = "x" + std::to_string(id) + " := " + value;
                            transition.update.numberUpdate.push_back(boost::lexical_cast<std::remove_reference_t<decltype(transition.update.numberUpdate.front())> >(asString));
                        } else {
                            throw std::runtime_error("Undeclared variable in assignment: " + identifier);
                        }
                    }
                }
            }
            transition.target = finalState;
            initialState->next[signatureId] = {std::move(transition)};

            // Clean up the local environment
            this->localStringVariables = nullptr;
            this->localNumberVariables = nullptr;

            return result;
        } else if (kind == "concat") {
            uint32_t p = 0;
            TSNode lhsNode = nextNonCommentChild(child, p);
            ++p;
            TSNode opNode = nextNonCommentChild(child, p);
            if (std::string(ts_node_type(opNode)) != ";") {
                throw std::runtime_error(makeErrorMessage("Expected ';' after left-hand side of concat", content, child));
            }
            ++p;
            TSNode rhsNode = nextNonCommentChild(child, p);
            Automaton lhs = this->parseExpr(content, lhsNode);
            Automaton rhs = this->parseExpr(content, rhsNode);
            return concatenate(std::move(lhs), std::move(rhs));
        } else if (kind == "conjunction") {
            uint32_t p = 0;
            TSNode lhsNode = nextNonCommentChild(child, p);
            ++p;
            TSNode opNode = nextNonCommentChild(child, p);
            if (std::string(ts_node_type(opNode)) != "&&") {
                throw std::runtime_error(makeErrorMessage("Expected '&&' after left-hand side of conjunction", content, child));
            }
            ++p;
            TSNode rhsNode = nextNonCommentChild(child, p);
            Automaton lhs = this->parseExpr(content, lhsNode);
            Automaton rhs = this->parseExpr(content, rhsNode);
            return conjunction(std::move(lhs), std::move(rhs));
        } else if (kind == "all_of") {
            uint32_t p = 0;
            // Expect "all_of"
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "all_of") {
                throw std::runtime_error(makeErrorMessage("Expected 'all_of' keyword", content, child));
            }
            ++p;
            // Expect "{"
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "{") {
                throw std::runtime_error(makeErrorMessage("Expected '{' after 'all_of'", content, child));
            }
            ++p;
            // parse exprs
            Automaton acc;
            bool first = true;
            while (p < ts_node_child_count(child)) {
                TSNode n = nextNonCommentChild(child, p);
                std::string nType = ts_node_type(n);
                if (nType == "}" || nType == "and" || nType == "{") {
                    ++p;
                    continue;
                } else if (nType != "expr") {
                    throw std::runtime_error(makeErrorMessage(
                        (std::string("Expected expr in all_of but got ") + nType).c_str(), content, child));
                }
                Automaton e = this->parseExpr(content, n);
                if (first) {
                  acc = std::move(e);
                  first = false;
                } else {
                  acc = conjunction(std::move(acc), std::move(e));
                }
                ++p;
            }
            return acc;
        } else if (kind == "disjunction") {
            uint32_t p = 0;
            TSNode lhsNode = nextNonCommentChild(child, p);
            ++p;
            TSNode opNode = nextNonCommentChild(child, p);
            if (std::string(ts_node_type(opNode)) != "||") {
                throw std::runtime_error(makeErrorMessage("Expected '||' after left-hand side of disjunction", content, child));
            }
            ++p;
            TSNode rhsNode = nextNonCommentChild(child, p);
            Automaton lhs = this->parseExpr(content, lhsNode);
            Automaton rhs = this->parseExpr(content, rhsNode);
            return disjunction(std::move(lhs), std::move(rhs));
        } else if (kind == "one_of") {
            uint32_t p = 0;
            // Expect "one_of"
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "one_of") {
                throw std::runtime_error(makeErrorMessage("Expected 'one_of' keyword", content, child));
            }
            ++p;
            // Expect "{"
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "{") {
                throw std::runtime_error(makeErrorMessage("Expected '{' after 'one_of'", content, child));
            }
            ++p;
            // parse exprs
            Automaton acc;
            bool first = true;
            while (p < ts_node_child_count(child)) {
                TSNode n = nextNonCommentChild(child, p);
                std::string nType = ts_node_type(n);
                if (nType == "}" || nType == "or" || nType == "{") {
                    ++p;
                    continue;
                } else if (nType != "expr") {
                    throw std::runtime_error(makeErrorMessage(
                        (std::string("Expected expr in one_of but got ") + nType).c_str(), content, child));
                }
                Automaton e = this->parseExpr(content, n);
                if (first) {
                    acc = std::move(e);
                    first = false;
                } else {
                    acc = disjunction(std::move(acc), std::move(e));
                }
                ++p;
            }
            return acc;
        } else if (kind == "optional" || kind == "optional_block") {
            uint32_t p = 0;
            TSNode n = nextNonCommentChild(child, p);
            while (std::string(ts_node_type(n)) != "expr") {
                ++p;
                n = nextNonCommentChild(child, p);
            }
            Automaton e = this->parseExpr(content, n);
            return emptyOr(std::move(e));
        } else if (kind == "kleene_star" || kind == "zero_or_more") {
            uint32_t p = 0;
            TSNode n = nextNonCommentChild(child, p);
            while (std::string(ts_node_type(n)) != "expr") {
                ++p;
                n = nextNonCommentChild(child, p);
            }
            Automaton e = this->parseExpr(content, n);
            return star(std::move(e));
        } else if (kind == "kleene_plus" || kind == "one_or_more") {
            uint32_t p = 0;
            TSNode n = nextNonCommentChild(child, p);
            while (std::string(ts_node_type(n)) != "expr") {
                ++p;
                n = nextNonCommentChild(child, p);
            }
            Automaton e = this->parseExpr(content, n);
            return plus(std::move(e));
        } else if (kind == "within" || kind == "time_restriction") {
            uint32_t p = 0;
            TSNode innerNode;
            TSNode intervalNode;
            while (p < ts_node_child_count(child)) {
                TSNode n = nextNonCommentChild(child, p);
                if (std::string(ts_node_type(n)) == "expr") {
                    innerNode = n;
                } else if (std::string(ts_node_type(n)) == "timing_constraint") {
                    intervalNode = n;
                }
                ++p;
            }
            if (ts_node_is_null(innerNode) || ts_node_is_null(intervalNode)) {
                throw std::runtime_error(makeErrorMessage("Expected 'expr' and 'timing_constraint' in within or time_restriction", content, child));
            }
            Automaton innerExpr = this->parseExpr(content, innerNode);
            // Optimization: if the last clock variable is not reset, we reuse the last clock variable
            if (innerExpr.clockVariableSize > 0 && noResetLastClock(innerExpr)) {
                innerExpr.clockVariableSize--;
            }

            TimingConstraint guard = this->parseTimingConstraint(content, intervalNode,
                                                                 innerExpr.clockVariableSize);

            // Apply the time restriction operation
            Automaton result = timeRestriction(std::move(innerExpr), guard);

            // Extract the upper bound from the timing constraint and add it to all transitions
            if constexpr (std::is_same_v<TimingConstraint, std::vector<::TimingConstraint>>) {
                // For non-parametric timing constraints
                auto upperBound = extractUpperBound(guard);
                if (!upperBound.empty()) {
                    // It has an upper bound, add it to all transitions
                    addConstraintToAllTransitions(result, upperBound);
                }
            } else if constexpr (std::is_same_v<TimingConstraint, ParametricTimingConstraint>) {
                // For parametric timing constraints
                auto upperBound = extractUpperBound(guard, innerExpr.clockVariableSize);
                addConstraintToAllTransitions(result, upperBound);
            }

            return result;
        } else if (kind == "paren_expr") {
            uint32_t p = 0;
            // Expect '('
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "(") {
                throw std::runtime_error(makeErrorMessage("Expected '(' at the beginning of paren_expr", content, child));
            }
            ++p;
            TSNode n = nextNonCommentChild(child, p);
            return this->parseExpr(content, n);
        } else if (kind == "ignore") {
            uint32_t p = 0;
            // Expect 'ignore'
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "ignore") {
                throw std::runtime_error(makeErrorMessage("Expected 'ignore' keyword", content, child));
            }
            ++p;
            // Expect identifier_list
            TSNode idList = nextNonCommentChild(child, p);
            if (ts_node_type(idList) != std::string("identifier_list")) {
                throw std::runtime_error(makeErrorMessage("Expected identifier_list after 'ignore'", content, child));
            }
            ++p;
            // Expect '{'
            nextNonCommentChild(child, p);
            if (std::string(ts_node_type(ts_node_child(child, p))) != "{") {
                throw std::runtime_error(makeErrorMessage("Expected '{' after 'ignore'", content, child));
            }
            ++p;
            Automaton inner = this->parseExpr(content, nextNonCommentChild(child, p));
            auto actions = this->parseActionList(content, idList);
            return ignoreActions(std::move(inner), actions);
        } else {
            std::cout << "Parsing automaton node: " << ts_node_type(child) << std::endl;
            throw std::runtime_error("Expected automaton node as child of expr node");
        }
    }

    std::vector<RawSignature> signatures;
    std::vector<std::string> parameters;
    std::vector<std::string> globalStringVariables;
    std::vector<std::string> globalNumberVariables;
    std::vector<std::string> *localStringVariables = nullptr;
    std::vector<std::string> *localNumberVariables = nullptr;
    std::vector<StringConstraint> initialStringConstraints;
    std::vector<NumberConstraint> initialNumberConstraints;
    std::unordered_map<std::string, Automaton> automata;
    std::optional<Automaton> expr;
};
