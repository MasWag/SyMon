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
      Expr expr = {NonSymbolic::NumberExpressionKind::PLUS, std::make_shared<Expr>(Expr(0)), std::make_shared<Expr>(Expr::constant(5))};
      std::optional<int> result;
      expr.eval(numEnv, result);
      BOOST_CHECK_EQUAL(result.value(), 12);
    }

    BOOST_AUTO_TEST_CASE(execute) {
      std::vector<std::pair<VariableID, NonSymbolic::StringAtom>> stringUpdate = {{0, NonSymbolic::StringAtom{VariableID{1}}}};
      NonSymbolic::StringValuation stringEnv = {std::make_optional("Alice"), std::make_optional("Bob")};
      std::vector<std::pair<VariableID, Expr>> numUpdate = {
          {0, Expr(1)},
          {1, Expr::constant(5)},
          {2, {NonSymbolic::NumberExpressionKind::PLUS, std::make_shared<Expr>(Expr(0)), std::make_shared<Expr>(Expr::constant(10))}}};
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

    BOOST_AUTO_TEST_CASE(string_execute) {
      std::vector<std::pair<VariableID, NonSymbolic::StringAtom>> stringUpdate = {
        {0, NonSymbolic::StringAtom{VariableID{1}}},
        {1, NonSymbolic::StringAtom{std::string{"Charlie"}}},

        {0, NonSymbolic::StringAtom{VariableID{1}}}
      };
      NonSymbolic::StringValuation stringEnv = {std::make_optional("Alice"), std::make_optional("Bob")};
      std::vector<std::pair<VariableID, Expr>> numUpdate;
      NonSymbolic::NumberValuation<int> numEnv;

      NonSymbolic::Update<int> update = {stringUpdate, numUpdate};
      update.execute(stringEnv, numEnv);
      BOOST_CHECK_EQUAL(stringEnv[0].value(), "Charlie");
      BOOST_CHECK_EQUAL(stringEnv[1].value(), "Charlie");
    }

    BOOST_AUTO_TEST_CASE(stringExecute2) {
      NonSymbolic::NumberValuation<int> numEnv;
      std::vector<NonSymbolic::NumberConstraint<int>> numberConstraints;
      std::vector<std::pair<VariableID, NonSymbolic::NumberExpression<int>>> numUpdate;

      NonSymbolic::StringValuation stringEnv = {std::nullopt, std::nullopt};
      std::vector<NonSymbolic::StringConstraint> stringConstraints1 = {
        NonSymbolic::SCMaker(0) != "Alice",
        NonSymbolic::SCMaker(0) != "Bob",
        NonSymbolic::SCMaker(0) != "Charlie",
        NonSymbolic::SCMaker(1) != "Alice",
        NonSymbolic::SCMaker(1) != "Bob"
      };
      BOOST_TEST(NonSymbolic::eval(stringConstraints1, stringEnv, numberConstraints, numEnv));
    
      std::vector<std::pair<VariableID, NonSymbolic::StringAtom>> stringUpdate = {
        {1, NonSymbolic::StringAtom{VariableID{0}}},
        {0, NonSymbolic::StringAtom{std::string{"Charlie"}}},
      };
      NonSymbolic::Update<int> update = {stringUpdate, numUpdate};
      update.execute(stringEnv, numEnv);
      // After the update, stringEnv[0] is "Charlie", and stringEnv[1] is disabled for "Alice", "Bob", "Charlie"
      BOOST_CHECK_EQUAL(*stringEnv[0], "Charlie");
      //BOOST_CHECK_EQUAL(std::get<std::vector<std::string>>(stringEnv[1]).size(), 3);

      //std::vector<NonSymbolic::StringConstraint> stringConstraints2 = {NonSymbolic::SCMaker(1) == "Charlie"};
      //BOOST_TEST(!NonSymbolic::eval(stringConstraints2, stringEnv, numberConstraints, numEnv));
    
      std::vector<NonSymbolic::StringConstraint> stringConstraints3 = {
        NonSymbolic::SCMaker(0) != "Alice",
        NonSymbolic::SCMaker(0) != "Bob",
        NonSymbolic::SCMaker(0) == "Charlie",
        NonSymbolic::SCMaker(0) != "David"
      };
      BOOST_TEST(NonSymbolic::eval(stringConstraints3, stringEnv, numberConstraints, numEnv));
      
      std::vector<NonSymbolic::StringConstraint> stringConstraints4 = {NonSymbolic::SCMaker(1) == "David"};
      // stringEnv[1] is assumed to be "David"
      BOOST_TEST(NonSymbolic::eval(stringConstraints4, stringEnv, numberConstraints, numEnv));

      //std::vector<NonSymbolic::StringConstraint> stringConstraints5 = {NonSymbolic::SCMaker(1) == "Eve"};
      // stringEnv[1] is assumed to be "David" above, so it cannot be "Eve"
      //BOOST_TEST(!NonSymbolic::eval(stringConstraints5, stringEnv, numberConstraints, numEnv));
    
    }
  BOOST_AUTO_TEST_SUITE_END() // EvalTests
BOOST_AUTO_TEST_SUITE_END() // NonSymbolicUpdateTests
