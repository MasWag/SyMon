#ifndef mem_fun_ref
#define mem_fun_ref mem_fn
#endif

#include <boost/test/tools/old/interface.hpp>
#include <fstream>
#include <ppl.hh>
using namespace Parma_Polyhedra_Library::IO_Operators;
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../src/common_types.hh"
#include "../src/automaton.hh"
#include "../src/symon_parser.hh"
#include "../src/non_symbolic_string_constraint.hh"
#include "../src/non_symbolic_number_constraint.hh"
#include "../src/non_symbolic_update.hh"
#include "timing_constraint.hh"

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
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
        }

        BOOST_AUTO_TEST_CASE(atomic) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
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
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(timingConstraint) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))%(< 5)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().odr, TimingConstraint::Order::lt);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().c, 5);
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
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
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
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transitions.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(orEmpty) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))?";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);

            // The other states should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
            BOOST_TEST(automaton.states[2]->next.empty());

            // The resulting automaton must accept the empty word
            BOOST_TEST(acceptsEmptyWord(automaton));
        }

        BOOST_AUTO_TEST_CASE(plus) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))+";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));

            // The initial state should have two transitions to the initial and final states labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(0).target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(1).target.lock().get(), automaton.states.at(0).get());

            // The transitions should have no string constraints, no number constraint, no guard, and no updates.
            // The transition to the final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The self-loop
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);

            // The final state should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
        }

        BOOST_AUTO_TEST_CASE(one_or_more) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} one_or_more {update(id, value)}";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));

            // The initial state should have two transitions to the initial and final states labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(0).target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(1).target.lock().get(), automaton.states.at(0).get());

            // The transitions should have no string constraints, no number constraint, no guard, and no updates.
            // The transition to the final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The self-loop
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);

            // The final state should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
        }

        BOOST_AUTO_TEST_CASE(star) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))*";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.at(1), automaton.states.at(2));

            // The initial state should have two transitions to the initial and final states labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(0).target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(1).target.lock().get(), automaton.states.at(0).get());

            // The transitions should have no string constraints, no number constraint, no guard, and no updates.
            // The transition to the final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The self-loop
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);

            // The other states should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
            BOOST_TEST(automaton.states[2]->next.empty());

            // The resulting automaton must accept the empty word
            BOOST_TEST(acceptsEmptyWord(automaton));
        }

        BOOST_AUTO_TEST_CASE(zero_or_more) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} zero_or_more{update(id, value)}";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
            // The automaton should have two states: initial and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.at(1), automaton.states.at(2));

            // The initial state should have two transitions to the initial and final states labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(0).target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].at(1).target.lock().get(), automaton.states.at(0).get());

            // The transitions should have no string constraints, no number constraint, no guard, and no updates.
            // The transition to the final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The self-loop
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);

            // The other states should have no transitions
            BOOST_TEST(automaton.states[1]->next.empty());
            BOOST_TEST(automaton.states[2]->next.empty());

            // The resulting automaton must accept the empty word
            BOOST_TEST(acceptsEmptyWord(automaton));
        }

        BOOST_AUTO_TEST_CASE(orEmptyConcat) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value))?;update(id, value)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
            // The automaton should have three states: initial, intermediate and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            BOOST_CHECK_NE(automaton.states.at(0).get(), automaton.states.at(1).get());
            BOOST_CHECK_NE(automaton.states.at(1).get(), automaton.states.at(2).get());
            BOOST_CHECK_NE(automaton.states.at(2).get(), automaton.states.at(0).get());
            // The initial state is not a match state, and the final state is a match state.
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(!automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.at(1), automaton.states.at(1));

            // The initial state should have one transition to the intermediate state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
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
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transitions.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(timingConstraintConcat) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value); (update(id, value))%(> 5)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The intermediate state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, no guard, and no updates.
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().odr, TimingConstraint::Order::gt);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().c, 5);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transitions.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(optionalWithTimingConstraintLT) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value)?)%(< 3)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            
            // The resulting automaton must accept the empty word
            BOOST_TEST(acceptsEmptyWord(automaton));

            // The automaton should have two states: initial and final
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            
            // Check states' match status
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            
            // For optional operations with timing constraints, there are two initial states
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.at(0), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.at(1), automaton.states.at(2));

            // The initial state should have transitions for the update operation
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next.at(0).size(), 1);
            
            // There should be a transition from initial state to final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            
            // The transition should have the timing constraint < 3
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().odr, TimingConstraint::Order::lt);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().c, 3);
            
            // The final states should have no transitions
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(optionalWithTimingConstraintGT) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} (update(id, value)?)%(> 3)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            
            // The resulting automaton must not accept the empty word
            BOOST_TEST(!acceptsEmptyWord(automaton));

            // The automaton should have three states: initial, intermediate, and final
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            
            // Check states' match status
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(automaton.states.at(1)->isMatch);
            
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            
            // The initial state should have transitions for the update operation
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next.at(0).size(), 1);
            
            // There should be transitions from initial state to intermediate state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            
            // The transition should not reset the clock
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            
            // The transition should have timing constraint > 3
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().odr, TimingConstraint::Order::gt);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().c, 3);
            
            // The intermediate state should have transitions for the update operation
            BOOST_TEST(automaton.states[1]->next.empty());
        }

        BOOST_AUTO_TEST_CASE(concatOptionalWithTimingConstraint) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value); (update(id, value)?)%(< 3)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            
            // The automaton should have three states: initial, intermediate, and final
            BOOST_CHECK_EQUAL(automaton.states.size(), 4);
            
            // Check states' match status
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(!automaton.states.at(1)->isMatch);
            BOOST_TEST(automaton.states.at(2)->isMatch);
            BOOST_TEST(automaton.states.at(3)->isMatch);
            
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            
            // The initial state should have transitions for the update operation
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next.at(0).size(), 2);
            
            // There should be transitions from initial state to intermediate state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            
            // The transition should reset the clock
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.front(), 0);
            
            // There should be another transition from initial state to the final state
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().target.lock().get(), automaton.states.at(3).get());
            
            // The transition should reset the clock
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.front(), 0);

            // The intermediate state should have transitions for the update operation
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(0) != automaton.states[1]->next.end()));
            
            // There should be transitions from intermediate state to final state
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().target.lock().get(), automaton.states.at(2).get());
            
            // The transition should have timing constraint < 3
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().odr, TimingConstraint::Order::lt);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().guard.front().c, 3);
            
            // The final state should have no transitions
            
        }
        
        BOOST_AUTO_TEST_CASE(timingConstraintWithBothBounds) {
            // Test that when handling time_restriction with both upper and lower bounds,
            // only the upper bound is added to all transitions
            
            // Create a parser
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            
            // Parse a simple automaton with a timing constraint that has both upper and lower bounds
            std::string content = R"(
                signature a {
                }
                
                a() % [2, 5]
            )";
            
            std::istringstream iss(content);
            parser.parse(iss);
            
            // Get the automaton
            auto automaton = parser.getAutomaton();
            
            // Check that the automaton has the expected structure
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            
            // Check that the transition has the expected guard
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            
            // The guard should have 2 constraints
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 2);
            
            // Verify that we have both the original constraints
            bool hasLowerBound = false;
            bool hasUpperBound = false;
            
            for (const auto& constraint : automaton.states[0]->next[0].front().guard) {
                if (constraint.odr == ::TimingConstraint::Order::ge && constraint.c == 2) {
                    // This is the original lower bound
                    hasLowerBound = true;
                } else if (constraint.odr == ::TimingConstraint::Order::le && constraint.c == 5) {
                    // This is the original upper bound
                    hasUpperBound = true;
                }
            }
            
            // Verify that we found both constraints
            BOOST_CHECK(hasLowerBound);
            BOOST_CHECK(hasUpperBound);
            // The final state should have no transitions
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(inits) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            std::ifstream ifs(PROJECT_ROOT "/example/inits.symon");
            // This should throw an exception because initial constraints are not supported in non-symbolic automata
            BOOST_CHECK_THROW(parser.parse(ifs), std::runtime_error);
        }
        
        BOOST_AUTO_TEST_CASE(extractUpperBoundTest) {
            // Test that the extractUpperBound function correctly extracts the upper bound from a timing constraint
            
            // Create a timing constraint with both upper and lower bounds
            std::vector<TimingConstraint> guard;
            
            // Add a lower bound: x >= 2
            guard.push_back(TimingConstraint{VariableID{0}, TimingConstraint::Order::ge, 2});
            
            // Add an upper bound: x <= 5
            guard.push_back(TimingConstraint{VariableID{0}, TimingConstraint::Order::le, 5});
            
            // Extract the upper bound
            auto upperBound = SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update>::extractUpperBound(guard);
            
            // Verify that only the upper bound was extracted
            BOOST_CHECK_EQUAL(upperBound.size(), 1);
            BOOST_CHECK_EQUAL(upperBound[0].x, VariableID{0});
            BOOST_CHECK_EQUAL(upperBound[0].odr, TimingConstraint::Order::le);
            BOOST_CHECK_EQUAL(upperBound[0].c, 5);
        }
        
        BOOST_AUTO_TEST_CASE(extractUpperBoundEquivalenceTest) {
            // Test that extractUpperBound for parametric and non-parametric cases are equivalent
            
            // Create a non-parametric timing constraint with both upper and lower bounds
            std::vector<TimingConstraint> nonParametricGuard;
            
            // Add a lower bound: x >= 2
            nonParametricGuard.push_back(TimingConstraint{VariableID{0}, TimingConstraint::Order::ge, 2});
            
            // Add an upper bound: x <= 5
            nonParametricGuard.push_back(TimingConstraint{VariableID{0}, TimingConstraint::Order::le, 5});
            
            // Extract the upper bound from the non-parametric guard
            auto nonParametricUpperBound = SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update>::extractUpperBound(nonParametricGuard);
            
            // Verify that only the upper bound was extracted
            BOOST_CHECK_EQUAL(nonParametricUpperBound.size(), 1);
            BOOST_CHECK_EQUAL(nonParametricUpperBound[0].x, VariableID{0});
            BOOST_CHECK_EQUAL(nonParametricUpperBound[0].odr, TimingConstraint::Order::le);
            BOOST_CHECK_EQUAL(nonParametricUpperBound[0].c, 5);
            
            // Create an equivalent parametric timing constraint
            ParametricTimingConstraint parametricGuard(1, Parma_Polyhedra_Library::UNIVERSE);
            
            // Add a lower bound: x >= 2
            Parma_Polyhedra_Library::Variable x(0);
            parametricGuard.add_constraint(x >= 2);
            
            // Add an upper bound: x <= 5
            parametricGuard.add_constraint(x <= 5);
            
            // Extract the upper bound from the parametric guard
            auto parametricUpperBound = SymonParser<StringConstraint, NumberConstraint<int>, ParametricTimingConstraint, Update>::extractUpperBound(parametricGuard);
            
            // Verify that the parametric upper bound contains only the upper bound constraint
            // We can't directly compare the constraints, but we can check that:
            // 1. The parametric upper bound is not the universe (it has constraints)
            // 2. The parametric upper bound is not empty (it's satisfiable)
            // 3. The parametric upper bound allows x = 3 (which satisfies x <= 5)
            // 4. The parametric upper bound doesn't allow x = 6 (which violates x <= 5)
            
            BOOST_CHECK(!parametricUpperBound.is_universe());
            BOOST_CHECK(!parametricUpperBound.is_empty());
            
            Parma_Polyhedra_Library::NNC_Polyhedron testPoint1(1);
            testPoint1.add_constraint(x == 3);
            testPoint1.intersection_assign(parametricUpperBound);
            BOOST_CHECK(!testPoint1.is_empty());
            
            Parma_Polyhedra_Library::NNC_Polyhedron testPoint2(1);
            testPoint2.add_constraint(x == 6);
            testPoint2.intersection_assign(parametricUpperBound);
            BOOST_CHECK(testPoint2.is_empty());
        }

        BOOST_AUTO_TEST_CASE(case20250703_1) {
            SymonParser<StringConstraint, NumberConstraint<int>, std::vector<TimingConstraint>, Update> parser;
            const std::string content = "signature A {id: string;} signature B {id: string;} ((A( id | id != \"Bob\" ) || B( id | id != \"Bob\" ))%(<= 5);B (id | id == \"Bob\"))%(> 5)";
            parser.parse(content);

            const NonParametricTA<int> automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            
            // The resulting automaton must not accept the empty word
            BOOST_TEST(!acceptsEmptyWord(automaton));

            // The automaton should have three states: initial, intermediate, and final
            BOOST_CHECK_EQUAL(automaton.states.size(), 4);
            
            // Check states' match status
            BOOST_TEST(!automaton.states.at(0)->isMatch);
            BOOST_TEST(!automaton.states.at(1)->isMatch);
            BOOST_TEST(!automaton.states.at(2)->isMatch);
            BOOST_TEST(automaton.states.at(3)->isMatch);
            
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            
            // Transition for the first A
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next.at(0).size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(2).get());
            // This transition should not reset the clock
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            // The transition should have timing constraint <= 5
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().odr, TimingConstraint::Order::le);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.front().c, 5);
            
            // Transition for the first B
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(1) != automaton.states[1]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[1]->next.at(1).size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().target.lock().get(), automaton.states.at(2).get());
            // This transition should not reset the clock
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().resetVars.size(), 0);
            // The transition should have timing constraint <= 5
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().guard.front().odr, TimingConstraint::Order::le);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[1].front().guard.front().c, 5);

            // Transition for the second B
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 1);
            BOOST_TEST((automaton.states[2]->next.find(1) != automaton.states[2]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[2]->next.at(1).size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().target.lock().get(), automaton.states.at(3).get());
            // This transition should not reset the clock
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().resetVars.size(), 0);
            // The transition should have timing constraint > 5
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().guard.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().guard.front().x, VariableID{0});
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().guard.front().odr, TimingConstraint::Order::gt);
            BOOST_CHECK_EQUAL(automaton.states[2]->next[1].front().guard.front().c, 5);
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
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
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
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
        }

        BOOST_AUTO_TEST_CASE(updates) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "var {count: number; count2: number;} signature update {id: string;value: number;} update(id, value | | count := count + 1; count2 := count2 + 2 ; count := count + count 2)";
            parser.parse(content);

            const auto automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 2);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 0);
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
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 3);
        }

        BOOST_AUTO_TEST_CASE(timingConstraintConcat) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "signature update {id: string;value: number;} update(id, value); (update(id, value))%(> 5)";
            parser.parse(content);

            const ParametricTA automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
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
            // The transition should have no string constraints, no number constraint, but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_TEST(automaton.states[0]->next[0].front().guard.is_universe());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().guard.space_dimension(), automaton.clockVariableSize);
            BOOST_TEST(automaton.states[0]->next[0].front().guard.is_universe());
            // The intermediate state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().target.lock().get(), automaton.states.back().get());
            // The transition should have no string constraints, no number constraint, but should have a guard.
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().numConstraints.size(), 0);
            BOOST_TEST(!automaton.states[1]->next[0].front().guard.is_universe());
            // Check that the guard is x0 > 5
            using namespace Parma_Polyhedra_Library;
            Constraint_System expected;
            expected.insert(Constraint{Linear_Expression{Variable(0)} > 5});
            ParametricTimingConstraint expectedGuard(expected);
            BOOST_TEST(!automaton.states[1]->next[0].front().guard.is_empty());
            BOOST_TEST((automaton.states[1]->next[0].front().guard == expectedGuard));
            BOOST_TEST(automaton.states[1]->next[0].front().guard.space_dimension() == automaton.clockVariableSize);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.stringUpdate.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transitions.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(parametricTimingConstraint) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "var {p: param;} signature A {} A()%(< p)";
            parser.parse(content);

            const ParametricTA automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.parameterSize, 1);
            // The automaton should have three states: initial, intermediate and final.
            BOOST_CHECK_EQUAL(automaton.states.size(), 2);
            BOOST_CHECK_NE(automaton.states.at(0).get(), automaton.states.at(1).get());
            // The initial state is not a match state, and the final state is a match state.
            BOOST_CHECK_EQUAL(automaton.states.at(0)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(1)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 1);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            // The transition should have no string constraints, no number constraint, but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[0]->next[0].front().guard.space_dimension());
            std::stringstream ss;
            ss << automaton.states[0]->next[0].front().guard.constraints();
            BOOST_CHECK_EQUAL("A - B > 0", ss.str());
            BOOST_CHECK_EQUAL(0, automaton.states[0]->next[0].front().resetVars.size());
            // BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The final state should have no transition.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(parametricTimingConstraintStar) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "var {p: param;} signature A {} (A()%(< p))*";
            parser.parse(content);

            const ParametricTA automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.parameterSize, 1);
            // The automaton should have four states: initial, loop, final from loop, and final without loop.
            BOOST_CHECK_EQUAL(automaton.states.size(), 3);
            BOOST_CHECK_NE(automaton.states.at(0).get(), automaton.states.at(1).get());
            BOOST_CHECK_NE(automaton.states.at(1).get(), automaton.states.at(2).get());
            BOOST_CHECK_NE(automaton.states.at(2).get(), automaton.states.at(0).get());
            // The initial and loop states are not match states, and the other states are match states.
            BOOST_CHECK_EQUAL(automaton.states.at(0)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(1)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.states.at(2)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 2);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.at(0));
            BOOST_CHECK_EQUAL(automaton.initialStates.back(), automaton.states.at(2));
            // The initial state should have two transitions, the loop and the transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().target.lock().get(), automaton.states.at(0).get());
            // The transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[0]->next[0].front().guard.space_dimension());
            std::stringstream ss;
            ss << automaton.states[0]->next[0].front().guard.constraints();
            BOOST_CHECK_EQUAL("A - B > 0", ss.str());
            ss.str("");
            BOOST_CHECK_EQUAL(0, automaton.states[0]->next[0].front().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The second transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[0]->next[0].back().guard.space_dimension());
            ss << automaton.states[0]->next[0].back().guard.constraints();
            BOOST_CHECK_EQUAL("A - B > 0", ss.str());
            ss.str("");
            BOOST_CHECK_EQUAL(1, automaton.states[0]->next[0].back().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);
            // The final states should have no transition.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
        }

        BOOST_AUTO_TEST_CASE(parametricTimingConstraintStarConcat) {
            SymonParser<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> parser;
            const std::string content = "var {p: param;} signature A {} A();(A()%(< p))*";
            parser.parse(content);

            const ParametricTA automaton = parser.getAutomaton();
            BOOST_CHECK_EQUAL(automaton.numberVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.stringVariableSize, 0);
            BOOST_CHECK_EQUAL(automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(automaton.parameterSize, 1);
            // The automaton should have four states: initial, loop, final from loop, and final without loop.
            BOOST_CHECK_EQUAL(automaton.states.size(), 4);
            BOOST_CHECK_NE(automaton.states.at(0).get(), automaton.states.at(1).get());
            BOOST_CHECK_NE(automaton.states.at(1).get(), automaton.states.at(2).get());
            BOOST_CHECK_NE(automaton.states.at(2).get(), automaton.states.at(3).get());
            BOOST_CHECK_NE(automaton.states.at(3).get(), automaton.states.at(0).get());
            // The initial and loop states are not match states, and the other states are match states.
            BOOST_CHECK_EQUAL(automaton.states.at(0)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(1)->isMatch, 0);
            BOOST_CHECK_EQUAL(automaton.states.at(2)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.states.at(3)->isMatch, 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton.initialStates.front(), automaton.states.front());
            // The initial state should have one transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[0]->next.size(), 1);
            BOOST_TEST((automaton.states[0]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().target.lock().get(), automaton.states.at(1).get());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().target.lock().get(), automaton.states.at(3).get());
            // The transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[0]->next[0].front().guard.space_dimension());
            BOOST_TEST(automaton.states[0]->next[0].front().guard.is_universe());
            BOOST_CHECK_EQUAL(1, automaton.states[0]->next[0].front().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].front().update.numberUpdate.size(), 0);
            // The second transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[0]->next[0].back().guard.space_dimension());
            BOOST_TEST(automaton.states[0]->next[0].back().guard.is_universe());
            BOOST_CHECK_EQUAL(1, automaton.states[0]->next[0].back().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[0]->next[0].back().update.numberUpdate.size(), 0);
            // The loop state should have two transitions, loop and the transition to the final state labeled with the signature.
            BOOST_CHECK_EQUAL(automaton.states[1]->next.size(), 1);
            BOOST_TEST((automaton.states[1]->next.find(0) != automaton.states[0]->next.end()));
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].size(), 2);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().target.lock().get(), automaton.states.at(2).get());
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].back().target.lock().get(), automaton.states.at(1).get());
            // The transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[1]->next[0].front().guard.space_dimension());
            std::stringstream ss;
            ss << automaton.states[1]->next[0].front().guard.constraints();
            BOOST_CHECK_EQUAL("A - B > 0", ss.str());
            BOOST_CHECK_EQUAL(0, automaton.states[1]->next[0].front().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].front().update.numberUpdate.size(), 0);
            // The second transition should have no string constraints, no number constraint, and no guard but should reset the clock.
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].back().stringConstraints.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].back().numConstraints.size(), 0);
            BOOST_CHECK_EQUAL(2, automaton.states[1]->next[0].back().guard.space_dimension());
            ss.str("");
            ss << automaton.states[1]->next[0].back().guard.constraints();
            BOOST_CHECK_EQUAL("A - B > 0", ss.str());
            BOOST_CHECK_EQUAL(1, automaton.states[1]->next[0].back().resetVars.size());
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].back().resetVars.front(), 0);
            BOOST_CHECK_EQUAL(automaton.states[1]->next[0].back().update.numberUpdate.size(), 0);
            // The final states should have no transition.
            BOOST_CHECK_EQUAL(automaton.states[2]->next.size(), 0);
            BOOST_CHECK_EQUAL(automaton.states[3]->next.size(), 0);
        }

    BOOST_AUTO_TEST_SUITE_END() // Parametric
BOOST_AUTO_TEST_SUITE_END() // SymonParserTests
