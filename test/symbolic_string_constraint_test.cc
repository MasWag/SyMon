#include <boost/test/unit_test.hpp>
#include "../src/io_operators.hh"
#include <boost/lexical_cast.hpp>

BOOST_AUTO_TEST_SUITE(SymbolicStringConstraintTests)
  BOOST_AUTO_TEST_SUITE(LexicalCastTests)
    using boost::lexical_cast;

    BOOST_AUTO_TEST_CASE(variable) {
      std::string str = "x12";
      using type = Symbolic::StringAtom;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.value.index(), 0);
      BOOST_CHECK_EQUAL(std::get<0>(result.value), 12ul);
    }

    BOOST_AUTO_TEST_CASE(constant) {
      std::string str = "'string'";
      using type = Symbolic::StringAtom;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.value.index(), 1);
      BOOST_CHECK_EQUAL(std::get<std::string>(result.value), "string");
    }

    BOOST_AUTO_TEST_CASE(varEqConstant) {
      std::string str = "x0 == 'y'";
      using type = Symbolic::StringConstraint;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.kind, Symbolic::StringConstraint::kind_t::EQ);
      // std::array<StringAtom, 2> children;
      // enum class kind_t {EQ, NE} kind;
    }

    BOOST_AUTO_TEST_CASE(varEqConstants) {
      std::string str = "{x0 == 'y'}";
      using type = std::vector<Symbolic::StringConstraint>;
      type result = lexical_cast<type>(str);
      BOOST_CHECK_EQUAL(result.size(), 1ul);
    }

  BOOST_AUTO_TEST_SUITE_END() // LexicalCastTests

  BOOST_AUTO_TEST_SUITE(MergeTest)
    using namespace Symbolic;

    BOOST_AUTO_TEST_CASE(eq_eq) {
      StringValuation left = {"x"};
      StringValuation right = {"y"};
      const auto merged = merge(left, right);
      BOOST_TEST(!bool(merged));
    }
    BOOST_AUTO_TEST_CASE(ne_eq_not_included) {
      StringValuation left = {std::vector<std::string>{"x"}};
      StringValuation right = {"y"};
      const auto merged = merge(left, right);
      BOOST_TEST(!bool(merged));
    }
    BOOST_AUTO_TEST_CASE(ne_eq_included) {
      StringValuation left = {std::vector<std::string>{"x", "y"}};
      StringValuation right = {"y"};
      const auto merged = merge(left, right);
      BOOST_TEST(bool(merged));
      BOOST_CHECK_EQUAL(1, merged->size());
      BOOST_CHECK_EQUAL(0, merged->front().index());
      BOOST_CHECK_EQUAL((std::vector<std::string>{"x"}), std::get<std::vector<std::string>>(merged->front()));
    }
    BOOST_AUTO_TEST_CASE(eq_ne_not_included) {
      StringValuation left = {"x"};
      StringValuation right = {std::vector<std::string>{"y"}};
      const auto merged = merge(left, right);
      BOOST_TEST(!bool(merged));
    }
    BOOST_AUTO_TEST_CASE(eq_ne_included) {
      StringValuation left = {"x"};
      StringValuation right = {std::vector<std::string>{"x", "y"}};
      const auto merged = merge(left, right);
      BOOST_TEST(bool(merged));
      BOOST_CHECK_EQUAL(1, merged->size());
      BOOST_CHECK_EQUAL(0, merged->front().index());
      BOOST_CHECK_EQUAL((std::vector<std::string>{"y"}), std::get<std::vector<std::string>>(merged->front()));
    }
    BOOST_AUTO_TEST_CASE(ne_ne) {
      StringValuation left = {std::vector<std::string>{"x", "z"}};
      StringValuation right = {std::vector<std::string>{"y", "z"}};
      const auto merged = merge(left, right);
      BOOST_TEST(bool(merged));
      BOOST_CHECK_EQUAL(1, merged->size());
      BOOST_CHECK_EQUAL(0, merged->front().index());
      BOOST_CHECK_EQUAL((std::vector<std::string>{"z"}), std::get<std::vector<std::string>>(merged->front()));
    }
  BOOST_AUTO_TEST_SUITE_END() // MergeTest
BOOST_AUTO_TEST_SUITE_END() // SymbolicStringConstraintTests
