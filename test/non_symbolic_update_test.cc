#include "../src/non_symbolic_update.hh"
#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_string_constraint.hh"
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <optional>

BOOST_AUTO_TEST_SUITE(NonSymbolicUpdateTests)
  BOOST_AUTO_TEST_SUITE(EvalTests)
    using Expr = NonSymbolic::NumberExpression<int>;

    BOOST_AUTO_TEST_CASE(variable) {
      NonSymbolic::NumberValuation<int> numEnv = {std::make_optional(7)};
      Expr expr(0);
      std::optional<int> result;
      expr.eval(numEnv, result);
      BOOST_CHECK_EQUAL(result.value(), 7);
    }

    BOOST_AUTO_TEST_CASE(constant) {
      NonSymbolic::NumberValuation<int> numEnv = {};
      Expr expr = Expr::constant(5);
      std::optional<int> result;
      expr.eval(numEnv, result);
      BOOST_CHECK_EQUAL(result.value(), 5);
    }

    BOOST_AUTO_TEST_CASE(plus) {
      NonSymbolic::NumberValuation<int> numEnv = {std::make_optional(7)};
      Expr expr = {Expr::kind_t::PLUS, std::make_shared<Expr>(Expr(0)), std::make_shared<Expr>(Expr::constant(5))};
      std::optional<int> result;
      expr.eval(numEnv, result);
      BOOST_CHECK_EQUAL(result.value(), 12);
    }

    BOOST_AUTO_TEST_CASE(execute) {
      std::vector<std::pair<VariableID, VariableID>> stringUpdate = {{0, 1}};
      NonSymbolic::StringValuation stringEnv = {std::make_optional("Alice"), std::make_optional("Bob")};
      std::vector<std::pair<VariableID, Expr>> numUpdate = {
          {0, Expr(1)},
          {1, Expr::constant(5)},
          {2, {Expr::kind_t::PLUS, std::make_shared<Expr>(Expr(0)), std::make_shared<Expr>(Expr::constant(10))}}};
      // x0=x1, x1=5, x2=x0+10
      NonSymbolic::NumberValuation<int> numEnv = {std::make_optional(1), std::make_optional(2), std::make_optional(4)};

      NonSymbolic::Update<int> update = {stringUpdate, numUpdate};
      update.execute(stringEnv, numEnv);
      BOOST_CHECK_EQUAL(stringEnv[0].value(), "Bob");
      BOOST_CHECK_EQUAL(numEnv[0].value(), 2);
      BOOST_CHECK_EQUAL(numEnv[1].value(), 5);
      //The evaluation of x0 is updated to 2 before evaluating x2
      BOOST_CHECK_EQUAL(numEnv[2].value(), 12);
    }
  BOOST_AUTO_TEST_SUITE_END() // EvalTests
BOOST_AUTO_TEST_SUITE_END() // NonSymbolicUpdateTests
