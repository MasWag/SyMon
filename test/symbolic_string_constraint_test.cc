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
BOOST_AUTO_TEST_SUITE_END() // SymbolicStringConstraintTests
