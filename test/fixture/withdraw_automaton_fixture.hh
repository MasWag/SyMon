#pragma once

#include <memory>
#include <sstream>
#include <vector>

#include "automaton.hh"
#include "common_types.hh"
#include "signature.hh"
#include "symbolic_string_constraint.hh"

/*
  @brief This automaton accepts "withdraw" behavior.
*/
struct WithdrawFixture {
    WithdrawFixture() {
        // Construct signature
        std::stringstream sigStream;
        sigStream << "withdraw\t1\t1";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton
        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<NonParametricTAState<int, double> >(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 1;
        automaton.numberVariableSize = 1;

        // Create timing constraints
        std::vector<TimingConstraint<double>> lessThanEqual30;
        lessThanEqual30.push_back(ConstraintMaker(0) <= 30.);

        std::vector<TimingConstraint<double>> greaterThan30;
        greaterThan30.push_back(ConstraintMaker(0) > 30.);

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update<int> update;
            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[0]
            };
        }

        // Transition from state 0 to state 1
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;

            NonSymbolic::Update<int> update;
            update.stringUpdate.emplace_back(VariableID{0}, NonSymbolic::StringAtom{VariableID{1}});
            update.numberUpdate.emplace_back(VariableID{0}, VariableID{1});

            std::vector<VariableID> resetVars = {0};

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(3);

        // First transition at state 1 (string not equal)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) != VariableID{1});

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update<int> update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Second transition at state 1 (string equal)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == VariableID{1});

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;

            NonSymbolic::Update<int> update;
            update.numberUpdate.emplace_back(VariableID{0}, VariableID{1});

            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Third transition from state 1 to state 2
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            numConstraints.push_back(NonSymbolic::NCMakerVar<int>(0) > 10000);

            NonSymbolic::Update<int> update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                greaterThan30,
                automaton.states[2]
            };
        }
    }

    NonParametricTA<int, double> automaton;
    std::unique_ptr<Signature> signature;
};


struct DataParametricWithdrawFixture {
    DataParametricWithdrawFixture() {
        using namespace Parma_Polyhedra_Library;
        using namespace Symbolic;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "withdraw\t1\t1";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton
        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<DataParametricTAState<double>>(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 1;
        automaton.numberVariableSize = 1;

        // Create timing constraints
        std::vector<TimingConstraint<double>> lessThanEqual30;
        lessThanEqual30.push_back(ConstraintMaker(0) <= 30.);

        std::vector<TimingConstraint<double>> greaterThan30;
        greaterThan30.push_back(ConstraintMaker(0) > 30.);

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[0]
            };
        }

        // Transition from state 0 to state 1
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.stringUpdate.emplace_back(VariableID{0}, StringAtom{VariableID{1}});
            update.numberUpdate.emplace_back(VariableID{0}, NumberExpression(Variable(1)));

            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(3);

        // First transition at state 1 (string not equal)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != VariableID{1});

            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Second transition at state 1 (string equal)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == VariableID{1});

            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.numberUpdate.emplace_back(VariableID{0}, NumberExpression{Variable{0} + Variable{1}});

            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Third transition from state 1 to state 2
        {
            std::vector<StringConstraint> stringConstraints;

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(NumberConstraint{Variable{0} > 10000});

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                greaterThan30,
                automaton.states[2]
            };
        }
    }

    DataParametricTA<double> automaton;
    std::unique_ptr<Signature> signature;
};


struct ParametricWithdrawFixture {
    ParametricWithdrawFixture() {
        using namespace Parma_Polyhedra_Library;
        using namespace Symbolic;

        std::stringstream sigStream;
        sigStream << "withdraw\t1\t1";
        signature = std::make_unique<Signature>(sigStream);

        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<PTAState>(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 1;
        automaton.numberVariableSize = 1;
        automaton.parameterSize = 0; // No parameters for this example

        // Create timing constraints
        ParametricTimingConstraint lessThanEqual30 = ParametricTimingConstraint(1);
        lessThanEqual30.add_constraint(Variable(0) <= 30);

        ParametricTimingConstraint greaterThan30 = ParametricTimingConstraint(1);
        greaterThan30.add_constraint(Variable(0) > 30);

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[0]
            };
        }

        // Transition from state 0 to state 1
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.stringUpdate.emplace_back(VariableID{0}, StringAtom{VariableID{1}});
            update.numberUpdate.emplace_back(VariableID{0}, NumberExpression(Variable(1)));

            std::vector<VariableID> resetVars = {0};

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(3);

        // First transition at state 1
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != VariableID{1});

            std::vector<NumberConstraint> numConstraints;

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Second transition at state 1
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == VariableID{1});

            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.numberUpdate.emplace_back(VariableID{0}, NumberExpression{Variable{0} + Variable{1}});

            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual30,
                automaton.states[1]
            };
        }

        // Third transition from state 1 to state 2
        {
            std::vector<StringConstraint> stringConstraints;

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(NumberConstraint{Variable{0} > 10000});

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                greaterThan30,
                automaton.states[2]
            };
        }
    }

    ParametricTA automaton;
    std::unique_ptr<Signature> signature;
};
