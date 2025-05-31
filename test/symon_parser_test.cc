#include <boost/test/tools/old/interface.hpp>
#include <fstream>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../src/common_types.hh"
#include "../src/automaton.hh"
#include "../src/symon_parser.hh"
#include "../src/non_symbolic_string_constraint.hh"
#include "../src/non_symbolic_number_constraint.hh"
#include "../src/non_symbolic_update.hh"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT ".."
#endif

BOOST_AUTO_TEST_SUITE(SymonParserTests)
    BOOST_AUTO_TEST_SUITE(NonSymbolicTests)
        using namespace NonSymbolic;

        struct CopyParserFixture {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string path = PROJECT_ROOT "/example/copy/copy.symon";

            CopyParserFixture() {
                std::ifstream ifs(path);
                parser.parse(ifs);
            }
        };

        BOOST_FIXTURE_TEST_CASE(signature, CopyParserFixture) {
            const Signature signature = parser.makeSignature();
            BOOST_CHECK_EQUAL(signature.size(), 1);
            BOOST_CHECK_EQUAL(signature.getId("update"), 0);
            BOOST_CHECK_EQUAL(signature.getStringSize("update"), 1);
            BOOST_CHECK_EQUAL(signature.getNumberSize("update"), 1);
        }

        BOOST_FIXTURE_TEST_CASE(declarations, CopyParserFixture) {
            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
        }

        BOOST_AUTO_TEST_CASE(atomic) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_CHECK_EQUAL(automaton.states.front()->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.back()->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(stringConstraint) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value | id == \"y\")";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_CHECK_EQUAL(automaton.states.front()->isMatch, false);
            BOOST_CHECK_EQUAL(automaton.states.back()->isMatch, true);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.front().kind, StringConstraint::kind_t::EQ);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.front().children[0], StringAtom{VariableID{0}});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.front().children[1], StringAtom{"y"});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(concat) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value); update(id, value)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            // The automaton should have three states: initial, intermediate and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            BOOST_CHECK_NE(automaton.states.at(0).get(), automaton.states.at(1).get());
            BOOST_CHECK_NE(automaton.states.at(1).get(), automaton.states.at(2).get());
            BOOST_CHECK_NE(automaton.states.at(2).get(), automaton.states.at(0).get());
            // The initial state is not a match state, and the final state is a match state.
            BOOST_CHECK_EQUAL(automaton.states.at(0)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(1)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(2)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the intermediate state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The intermediate state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transitions.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(star) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))*";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.at(1), automaton.states.at(2));

            // The initial state should have one transition to the original final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);

            // The other states should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
            BOOST_TEST(automaton.states[2]->next.empty());
        }

        BOOST_AUTO_TEST_CASE(inits) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            std::ifstream ifs(PROJECT_ROOT "/example/inits.symon");
            // This should throw an exception because initial constraints are not supported in non-symbolic automata
            BOOST_CHECK_THROW(parser.parse(ifs), std::runtime_error);
        }

    BOOST_AUTO_TEST_SUITE_END() // NonSymbolicTests

    BOOST_AUTO_TEST_SUITE(Parametric)
        using namespace Symbolic;

        struct ParametricCopyParserFixture {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string path = PROJECT_ROOT "/example/copy/copy.symon";

            ParametricCopyParserFixture() {
                std::ifstream ifs(path);
                parser.parse(ifs);
            }
        };

        BOOST_FIXTURE_TEST_CASE(signature, ParametricCopyParserFixture) {
            const Signature signature = parser.makeSignature();
            BOOST_CHECK_EQUAL(signature.size(), 1);
            BOOST_CHECK_EQUAL(signature.getId("update"), 0);
            BOOST_CHECK_EQUAL(signature.getStringSize("update"), 1);
            BOOST_CHECK_EQUAL(signature.getNumberSize("update"), 1);
        }

        BOOST_FIXTURE_TEST_CASE(declarations, ParametricCopyParserFixture) {
            const ParametricTA automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
        }

        BOOST_AUTO_TEST_CASE(inits) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            std::ifstream ifs(PROJECT_ROOT "/example/inits.symon");
            parser.parse(ifs);
            const auto automaton = parser.getAutomaton();
            // An automaton with initial constraints should have one initial state
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            // The initial state should have only unobservable transitions
            BOOST_CHECK_EQUAL(automaton.initialStates[0]->next.size(), 1);
            BOOST_TEST((automaton.initialStates[0]->next.find(127) != automaton.initialStates[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 1);
        }

        BOOST_AUTO_TEST_CASE(updates) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "var {count: number; count2: number;} init {count == 0 && count2 && 0} signature update {id: string;value: number;} update(id, value | | count := count + 1; count2 := count2 + 2 ; count := count + count 2)";
            parser.parse(content);

            const auto automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 2);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_CHECK_EQUAL(automaton.states.front()->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.back()->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_TEST(automaton.states[0]->next[0].front().guard.is_universe());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 3);
        }

    BOOST_AUTO_TEST_SUITE_END() // Parametric
BOOST_AUTO_TEST_SUITE_END() // SymonParserTests
