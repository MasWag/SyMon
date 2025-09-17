//
// Created by Masaki Waga on 2019-01-28.
//

#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../src/common_types.hh"
#include "../src/automaton.hh"
#include "../src/automaton_parser.hh"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT ".."
#endif

BOOST_AUTO_TEST_SUITE(ParametricTimedAutomaton)
  BOOST_AUTO_TEST_SUITE(ParseBoostTATests)

    BOOST_AUTO_TEST_CASE(Copy) {
      BoostPTA BoostTA;
      std::ifstream file(PROJECT_ROOT "/example/copy/copy_parametric.dot");
      parseBoostTA(file, BoostTA);

      BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_clock_variable_size), 1);
      BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_string_variable_size), 0);
      BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_number_variable_size), 1);
      BOOST_CHECK_EQUAL(boost::get_property(BoostTA, boost::graph_parameter_size), 1);


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

  BOOST_AUTO_TEST_SUITE_END() //ParseBoostTATests
  BOOST_AUTO_TEST_SUITE(ConvBoostTATests)

    BOOST_AUTO_TEST_CASE(copy) {
      using namespace Parma_Polyhedra_Library;
      BoostPTA BoostTA;
      std::ifstream file(PROJECT_ROOT "/example/copy/copy_parametric.dot");
      parseBoostTA(file, BoostTA);
      ParametricTA TA;
      convBoostTA(BoostTA, TA);

      BOOST_CHECK_EQUAL(TA.clockVariableSize, 1);
      BOOST_CHECK_EQUAL(TA.stringVariableSize, 0);
      BOOST_CHECK_EQUAL(TA.numberVariableSize, 1);
      BOOST_CHECK_EQUAL(TA.parameterSize, 1);

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
      BOOST_TEST(TA.states[0]->next.at(0).at(1).guard.is_universe());
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
      {
        NNC_Polyhedron expectedGuard(2);
        expectedGuard.
                add_constraint(Variable(1)
                               < 3);
        BOOST_TEST((TA.states[1]->next.at(0).at(0).guard == expectedGuard));
      }

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
      {
        NNC_Polyhedron expectedGuard(2);
        expectedGuard.
                add_constraint(Variable(1)
                               < Variable(0));
        BOOST_TEST((TA.states[2]->next.at(0).at(3).guard == expectedGuard));
      }

      BOOST_CHECK_EQUAL(TA.states[2]->next.at(0).at(4).target.lock(), TA.states[3]);


      // #### FROM STATE 3 ####
      BOOST_CHECK_EQUAL(TA.states[3]->next.size(), 0);
    }

  BOOST_AUTO_TEST_SUITE_END() // ConvBoostTATests
BOOST_AUTO_TEST_SUITE_END() // ParametricTimedAutomaton