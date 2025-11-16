#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../src/common_types.hh"
#include "../src/automaton.hh"
#include "../src/automaton_parser.hh"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT ".."
#endif

std::ostream &operator<<(std::ostream &os, const std::vector<ClockVariables> &resetVars);

BOOST_AUTO_TEST_SUITE(AutomatonParserTests)
  BOOST_AUTO_TEST_SUITE(LexicalCastTests)
    using boost::lexical_cast;
    BOOST_AUTO_TEST_SUITE(NonParametricUpdatesTests)

      BOOST_AUTO_TEST_CASE(update) {
        std::string str = "x0 := x1";
        using type = std::pair<VariableID, VariableID>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.first, 0);
        BOOST_CHECK_EQUAL(result.second, 1);
      }

      BOOST_AUTO_TEST_CASE(updates) {
        std::string str = "{x0 := x1}";
        using type = std::vector<std::pair<VariableID, VariableID>>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.size(), 1ul);
        BOOST_CHECK_EQUAL(result.front().first, 0);
        BOOST_CHECK_EQUAL(result.front().second, 1);
      }

    BOOST_AUTO_TEST_SUITE_END() // NonParametricUpdatesTests

    BOOST_AUTO_TEST_SUITE(SymbolicUpdatesTests)

      BOOST_AUTO_TEST_CASE(updateVariable) {
        std::string str = "x0 := x1";
        using type = std::pair<VariableID, Symbolic::NumberExpression>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.first, 0);
        Parma_Polyhedra_Library::Linear_Expression expectedExpression(Parma_Polyhedra_Library::Variable(1));
        BOOST_TEST(result.second.is_equal_to(expectedExpression));
      }

      BOOST_AUTO_TEST_CASE(updateExpression) {
        std::string str = "x1 := x0 + x1";
        using type = std::pair<VariableID, Symbolic::NumberExpression>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.first, 1);
        Parma_Polyhedra_Library::Linear_Expression expectedExpression(
                Parma_Polyhedra_Library::Variable(0) + Parma_Polyhedra_Library::Variable(1));
        BOOST_TEST(result.second.is_equal_to(expectedExpression));
      }

      BOOST_AUTO_TEST_CASE(updateVariables) {
        std::string str = "{x0 := x1}";
        using type = std::vector<std::pair<VariableID, Symbolic::NumberExpression>>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.size(), 1ul);
        BOOST_CHECK_EQUAL(result.front().first, 0);
        Parma_Polyhedra_Library::Linear_Expression expectedExpression(Parma_Polyhedra_Library::Variable(1));
        BOOST_TEST(result.front().second.is_equal_to(expectedExpression));
      }

    BOOST_AUTO_TEST_SUITE_END() // SymbolicUpdatesTests

    BOOST_AUTO_TEST_SUITE(NonParametricNumberConstraintTests)

      BOOST_AUTO_TEST_CASE(variable) {
        std::string str = "x12";
        using type = NonSymbolic::NumberExpression<int>;
        type result = lexical_cast<type>(str);
        BOOST_TEST((result.kind == type::kind_t::ATOM));
        BOOST_CHECK_EQUAL(result.child.index(), 0);
        BOOST_CHECK_EQUAL(std::get<VariableID>(result.child), 12ul);
      }

      BOOST_AUTO_TEST_CASE(plus) {
        std::string str = "x12 + x0";
        using type = NonSymbolic::NumberExpression<int>;
        type result = lexical_cast<type>(str);
        BOOST_TEST((result.kind == type::kind_t::PLUS));
        BOOST_CHECK_EQUAL(result.child.index(), 1);
      }

      BOOST_AUTO_TEST_CASE(minus) {
        std::string str = "x12 - x0";
        using type = NonSymbolic::NumberExpression<int>;
        type result = lexical_cast<type>(str);
        BOOST_TEST((result.kind == type::kind_t::MINUS));
        BOOST_CHECK_EQUAL(result.child.index(), 1);
      }

      BOOST_AUTO_TEST_CASE(NeConstraint) {
        std::string str = "x0 - x1 != 0";
        using type = NonSymbolic::NumberConstraint<int>;
        type result = lexical_cast<type>(str);
        BOOST_TEST((result.kind == type::kind_t::NE));
        // std::array<StringAtom, 2> children;
        // enum class kind_t {EQ, NE} kind;
      }

      BOOST_AUTO_TEST_CASE(NeConstraints) {
        std::string str = "{x0 - x1 != 0}";
        using type = std::vector<NonSymbolic::NumberConstraint<int>>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.size(), 1ul);
      }

    BOOST_AUTO_TEST_SUITE_END() // NonParametricNumberConstraintTests

    BOOST_AUTO_TEST_SUITE(SymbolicNumberConstraintTests)

      BOOST_AUTO_TEST_CASE(variable) {
        std::string str = "x12";
        using type = Symbolic::NumberExpression;
        type result = lexical_cast<type>(str);
        Parma_Polyhedra_Library::Linear_Expression expected{Parma_Polyhedra_Library::Variable(12)};
        BOOST_TEST(result.is_equal_to(expected));
      }

      BOOST_AUTO_TEST_CASE(plus) {
        std::string str = "x12 + x0";
        using type = Symbolic::NumberExpression;
        type result = lexical_cast<type>(str);
        Parma_Polyhedra_Library::Linear_Expression expected{
                Parma_Polyhedra_Library::Variable(12) + Parma_Polyhedra_Library::Variable(0)};
        BOOST_TEST(result.is_equal_to(expected));
      }

      BOOST_AUTO_TEST_CASE(minus) {
        std::string str = "x12 - x0";
        using type = Symbolic::NumberExpression;
        type result = lexical_cast<type>(str);
        Parma_Polyhedra_Library::Linear_Expression expected{
                Parma_Polyhedra_Library::Variable(12) - Parma_Polyhedra_Library::Variable(0)};
        BOOST_TEST(result.is_equal_to(expected));
      }

      BOOST_AUTO_TEST_CASE(GtConstraint) {
        std::string str = "x0 - x1 > 0";
        using type = Symbolic::NumberConstraint;
        type result = lexical_cast<type>(str);
        Parma_Polyhedra_Library::Constraint expected{
                Parma_Polyhedra_Library::Variable(0) > Parma_Polyhedra_Library::Variable(1)};
        BOOST_TEST(result.is_equal_to(expected));
      }

      BOOST_AUTO_TEST_CASE(GtConstant) {
        std::string str = "x0 > 10000";
        using type = Symbolic::NumberConstraint;
        type result = lexical_cast<type>(str);
        Parma_Polyhedra_Library::Constraint expected{
                Parma_Polyhedra_Library::Variable(0) > 10000};
        BOOST_TEST(result.is_equal_to(expected));
      }

      BOOST_AUTO_TEST_CASE(LtConstraints) {
        std::string str = "{x0 - x1 < 0}";
        using type = std::vector<Symbolic::NumberConstraint>;
        type result = lexical_cast<type>(str);
        BOOST_CHECK_EQUAL(result.size(), 1ul);
        Parma_Polyhedra_Library::Constraint expected{
                Parma_Polyhedra_Library::Variable(0) < Parma_Polyhedra_Library::Variable(1)};
        BOOST_TEST(result.front().is_equal_to(expected));
      }

    BOOST_AUTO_TEST_SUITE_END() // SymbolicNumberConstraintTests

  BOOST_AUTO_TEST_SUITE_END()

  BOOST_AUTO_TEST_SUITE(ParseBoostTATests)
    BOOST_AUTO_TEST_SUITE(NonParametricTimedAutomaton)

      BOOST_AUTO_TEST_CASE(Copy) {
        NonParametricBoostTA<int> BoostTA;
        std::ifstream file(PROJECT_ROOT "/example/copy/copy.dot");
        parseBoostTA(file, BoostTA);

        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_clock_variable_size), 1);
        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_string_variable_size), 0);
        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_number_variable_size), 1);

        BOOST_REQUIRE_EQUAL(boost::num_vertices(BoostTA), 4);
        BOOST_TEST(!BoostTA[0].isMatch);
        BOOST_TEST(!BoostTA[1].isMatch);
        BOOST_TEST(!BoostTA[2].isMatch);
        BOOST_TEST(BoostTA[3].isMatch);

        BOOST_TEST(BoostTA[0].isInit);
        BOOST_TEST(!BoostTA[1].isInit);
        BOOST_TEST(!BoostTA[2].isInit);
        BOOST_TEST(!BoostTA[3].isInit);

        // We cannot check in_degree because our graph is not bidirectional
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(0, BoostTA), BoostTA), 1);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(1, BoostTA), BoostTA), 4);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(2, BoostTA), BoostTA), 3);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(3, BoostTA), BoostTA), 1);

        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(0, BoostTA), BoostTA), 2ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(1, BoostTA), BoostTA), 3ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(2, BoostTA), BoostTA), 4ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(3, BoostTA), BoostTA), 0ul);
      }

    BOOST_AUTO_TEST_SUITE_END() // NonParametricTA

    BOOST_AUTO_TEST_SUITE(DataParametricTimedAutomaton)

      BOOST_AUTO_TEST_CASE(Copy) {
        DataParametricBoostTA BoostTA;
        std::ifstream file(PROJECT_ROOT "/example/copy/copy_data_parametric.dot");
        parseBoostTA(file, BoostTA);

        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_clock_variable_size), 1);
        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_string_variable_size), 0);
        BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_number_variable_size), 1);

        BOOST_REQUIRE_EQUAL(boost::num_vertices(BoostTA), 4);
        BOOST_TEST(!BoostTA[0].isMatch);
        BOOST_TEST(!BoostTA[1].isMatch);
        BOOST_TEST(!BoostTA[2].isMatch);
        BOOST_TEST(BoostTA[3].isMatch);

        BOOST_TEST(BoostTA[0].isInit);
        BOOST_TEST(!BoostTA[1].isInit);
        BOOST_TEST(!BoostTA[2].isInit);
        BOOST_TEST(!BoostTA[3].isInit);

        // We cannot check in_degree because our graph is not bidirectional
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(0, BoostTA), BoostTA), 1);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(1, BoostTA), BoostTA), 4);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(2, BoostTA), BoostTA), 3);
        // BOOST_CHECK_EQUAL(boost::in_degree(boost::vertex(3, BoostTA), BoostTA), 1);

        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(0, BoostTA), BoostTA), 2ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(1, BoostTA), BoostTA), 4ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(2, BoostTA), BoostTA), 5ul);
        BOOST_CHECK_EQUAL(boost::out_degree(boost::vertex(3, BoostTA), BoostTA), 0ul);
      }

    BOOST_AUTO_TEST_SUITE_END() // DataParametricTimedAutomaton
  BOOST_AUTO_TEST_SUITE_END() // ParseBoostTATests

  BOOST_AUTO_TEST_SUITE(ConvBoostTATests)
    BOOST_AUTO_TEST_SUITE(NonParametricTimedAutomaton)

      BOOST_AUTO_TEST_CASE(Copy) {
        NonParametricBoostTA<int> BoostTA;
        NonParametricTA<int> TA;
        // NonParametricTA<int> TA;
        std::ifstream file(PROJECT_ROOT "/example/copy/copy.dot");
        parseBoostTA(file, BoostTA);
        convBoostTA(BoostTA, TA);

        BOOST_CHECK_EQUAL(TA.clockVariableSize, 1);
        BOOST_CHECK_EQUAL(TA.stringVariableSize, 0);
        BOOST_CHECK_EQUAL(TA.numberVariableSize, 1);

        BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
        BOOST_CHECK_EQUAL(TA.initialStates.front(), TA.states[0]);

        BOOST_CHECK_EQUAL(TA.states.size(), 4);
        BOOST_TEST(!TA.states[0]->isMatch);
        BOOST_TEST(!TA.states[1]->isMatch);
        BOOST_TEST(!TA.states[2]->isMatch);
        BOOST_TEST(TA.states[3]->isMatch);

        // #### FROM STATE 0 ####
        BOOST_CHECK_EQUAL(TA.states[0]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).size(), 2);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(0).target.lock(), TA.states[0]);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().kind,
                          NonSymbolic::StringConstraint::kind_t::EQ);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[1].value.index(), 1);
        BOOST_CHECK_EQUAL(
                std::get<std::string>(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[1].value), "y");
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).numConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().first, 0);
        BOOST_CHECK_EQUAL(std::get<VariableID>(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().second.child), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).guard.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.front(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).target.lock(), TA.states[1]);

        // #### FROM STATE 1 ####
        BOOST_CHECK_EQUAL(TA.states[1]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).size(), 3);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().kind,
                          NonSymbolic::StringConstraint::kind_t::EQ);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value.index(), 1);
        BOOST_CHECK_EQUAL(
                std::get<std::string>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value), "x");
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).numConstraints.size(), 1);
        BOOST_TEST((TA.states[1]->next.at(0).at(0).numConstraints.front().kind ==
                    NonSymbolic::NumberConstraint<int>::kind_t::NE));
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).numConstraints.front().left.kind,
                          NonSymbolic::NumberExpression<int>::kind_t::MINUS);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child.index(), 1);
        BOOST_CHECK_EQUAL(std::get<1>(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child)[0]->kind,
                          NonSymbolic::NumberExpression<int>::kind_t::ATOM);
        BOOST_CHECK_EQUAL(
                std::get<1>(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child)[0]->child.index(),
                0);
        BOOST_CHECK_EQUAL(
                std::get<0>(std::get<1>(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child)[0]->child),
                0);
        BOOST_CHECK_EQUAL(std::get<1>(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child)[1]->kind,
                          NonSymbolic::NumberExpression<int>::kind_t::ATOM);
        BOOST_CHECK_EQUAL(
                std::get<0>(std::get<1>(TA.states[1]->next.at(0).at(0).numConstraints.front().left.child)[1]->child),
                1);
        BOOST_CHECK_EQUAL(std::get<int>(TA.states[1]->next.at(0).at(0).numConstraints.front().right.child), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.numberUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().x, 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().odr, TimingConstraint::Order::lt);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().c, 3);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).target.lock(), TA.states[1]);

        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).target.lock(), TA.states[1]);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).target.lock(), TA.states[2]);

        // #### FROM STATE 2 ####
        BOOST_CHECK_EQUAL(TA.states[2]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).size(), 4);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(0).target.lock(), TA.states[2]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(1).target.lock(), TA.states[2]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(2).target.lock(), TA.states[1]);

        // #### FROM STATE 3 ####
        BOOST_CHECK_EQUAL(TA.states[3]->next.size(), 0);
      }

    BOOST_AUTO_TEST_SUITE_END() // NonParametricTA

    BOOST_AUTO_TEST_SUITE(DataParametricTimedAutomaton)

      BOOST_AUTO_TEST_CASE(Copy) {
        using namespace Parma_Polyhedra_Library;
        DataParametricBoostTA BoostTA;
        DataParametricTA TA;
        std::ifstream file(PROJECT_ROOT "/example/copy/copy_data_parametric.dot");
        parseBoostTA(file, BoostTA);
        convBoostTA(BoostTA, TA);

        BOOST_CHECK_EQUAL(TA.clockVariableSize, 1);
        BOOST_CHECK_EQUAL(TA.stringVariableSize, 0);
        BOOST_CHECK_EQUAL(TA.numberVariableSize, 1);

        BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
        BOOST_CHECK_EQUAL(TA.initialStates.front(), TA.states[0]);
        BOOST_CHECK_EQUAL(TA.states.size(), 4);
        BOOST_TEST(!TA.states[0]->isMatch);
        BOOST_TEST(!TA.states[1]->isMatch);
        BOOST_TEST(!TA.states[2]->isMatch);
        BOOST_TEST(TA.states[3]->isMatch);

        // #### FROM STATE 0 ####
        BOOST_CHECK_EQUAL(TA.states[0]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).size(), 2);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(0).target.lock(), TA.states[0]);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().kind,
                          Symbolic::StringConstraint::kind_t::EQ);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[1].value.index(), 1);
        BOOST_CHECK_EQUAL(
                std::get<std::string>(TA.states[0]->next.at(0).at(1).stringConstraints.front().children[1].value), "y");
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).numConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().first, 0);
        BOOST_TEST(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().second.is_equal_to(Variable(1)));
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).guard.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.front(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).target.lock(), TA.states[1]);

        // #### FROM STATE 1 ####
        BOOST_CHECK_EQUAL(TA.states[1]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).size(), 4);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().kind,
                          Symbolic::StringConstraint::kind_t::EQ);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value.index(), 1);
        BOOST_CHECK_EQUAL(
                std::get<std::string>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value), "x");
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).numConstraints.size(), 1);
        BOOST_TEST(TA.states[1]->next.at(0).at(0).numConstraints.at(0).is_equal_to(Variable(0) > Variable(1)));


        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.numberUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().x, 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().odr, ::TimingConstraint::Order::lt);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().c, 3);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).target.lock(), TA.states[1]);

        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).target.lock(), TA.states[1]);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).target.lock(), TA.states[1]);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(3).target.lock(), TA.states[2]);

        // #### FROM STATE 2 ####
        BOOST_CHECK_EQUAL(TA.states[2]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).size(), 5);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(0).target.lock(), TA.states[2]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(1).target.lock(), TA.states[2]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(2).target.lock(), TA.states[1]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(3).target.lock(), TA.states[1]);
        BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(4).target.lock(), TA.states[3]);



        // #### FROM STATE 3 ####
        BOOST_CHECK_EQUAL(TA.states[3]->next.size(), 0);
      }

      BOOST_AUTO_TEST_CASE(withdraw) {
        using namespace Parma_Polyhedra_Library;
        DataParametricBoostTA BoostTA;
        DataParametricTA TA;
        std::ifstream file(PROJECT_ROOT "/example/withdraw/withdraw.dot");
        parseBoostTA(file, BoostTA);
        convBoostTA(BoostTA, TA);

        BOOST_CHECK_EQUAL(TA.clockVariableSize, 1);
        BOOST_CHECK_EQUAL(TA.stringVariableSize, 1);
        BOOST_CHECK_EQUAL(TA.numberVariableSize, 1);

        BOOST_CHECK_EQUAL(TA.initialStates.size(), 1);
        BOOST_CHECK_EQUAL(TA.initialStates.front(), TA.states[0]);
        BOOST_CHECK_EQUAL(TA.states.size(), 3);
        BOOST_TEST(!TA.states[0]->isMatch);
        BOOST_TEST(!TA.states[1]->isMatch);
        BOOST_TEST(TA.states[2]->isMatch);

        // #### FROM STATE 0 ####
        BOOST_CHECK_EQUAL(TA.states[0]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).size(), 2);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(0).target.lock(), TA.states[0]);

        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).stringConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).numConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.stringUpdate.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.stringUpdate.at(0).first, 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.stringUpdate.at(0).second, 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().first, 0);
        BOOST_TEST(TA.states[0]->next.at(0).at(1).update.numberUpdate.front().second.is_equal_to(Variable(1)));
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).guard.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).resetVars.front(), 0);
        BOOST_CHECK_EQUAL(TA.states[0]->next.at(0).at(1).target.lock(), TA.states[1]);

        // #### FROM STATE 1 ####
        BOOST_CHECK_EQUAL(TA.states[1]->next.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).size(), 3);

        // ###### Transition 1 #####
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().kind,
                          Symbolic::StringConstraint::kind_t::NE);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(0).stringConstraints.front().children[1].value), 1ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).numConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).update.numberUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().x, 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().odr, ::TimingConstraint::Order::le);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).guard.front().c, 30);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(0).target.lock(), TA.states[1]);

        // ###### Transition 2 #####
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).stringConstraints.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).stringConstraints.front().kind,
                          Symbolic::StringConstraint::kind_t::EQ);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).stringConstraints.front().children[0].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(1).stringConstraints.front().children[0].value), 0ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).stringConstraints.front().children[1].value.index(), 0);
        BOOST_CHECK_EQUAL(
                std::get<std::size_t>(TA.states[1]->next.at(0).at(1).stringConstraints.front().children[1].value), 1ul);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).numConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).update.numberUpdate.size(), 1);
        BOOST_TEST(TA.states[1]->next.at(0).at(1).update.numberUpdate.front().second.is_equal_to(
                Variable(0) + Variable(1)));
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).guard.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).guard.front().x, 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).guard.front().odr, ::TimingConstraint::Order::le);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).guard.front().c, 30);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(1).target.lock(), TA.states[1]);

        // ###### Transition 3 #####
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).stringConstraints.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).numConstraints.size(), 1);
        BOOST_TEST(TA.states[1]->next.at(0).at(2).numConstraints.at(0).is_equal_to(Variable(0) > 10000));
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).update.stringUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).update.numberUpdate.size(), 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).guard.size(), 1);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).guard.front().x, 0);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).guard.front().odr, ::TimingConstraint::Order::gt);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).guard.front().c, 30);
        BOOST_CHECK_EQUAL(TA.states[1]->next.at(0).at(2).target.lock(), TA.states[2]);


        // #### FROM STATE 2 ####
        BOOST_CHECK_EQUAL(TA.states[2]->next.size(), 0);
      }

BOOST_AUTO_TEST_SUITE_END() // DataParametricTimedAutomaton
BOOST_AUTO_TEST_SUITE_END() // ConvBoostTATests
BOOST_AUTO_TEST_SUITE_END()
