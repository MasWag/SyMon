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

    BOOST_AUTO_TEST_SUITE_END() // Parametric
BOOST_AUTO_TEST_SUITE_END() // SymonParserTests
