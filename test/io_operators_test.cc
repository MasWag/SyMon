#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../src/io_operators.hh"

BOOST_AUTO_TEST_SUITE(IOOperatorsTest)
  BOOST_AUTO_TEST_SUITE(SymbolicNumberExpression)

    BOOST_AUTO_TEST_CASE(readEof) {
      const std::string str = "x1";
      std::stringstream stream;
      stream << str;
      Symbolic::NumberExpression expr;
      stream >> expr;
      BOOST_TEST(stream.eof());
    }

    BOOST_AUTO_TEST_CASE(readNotEof) {
      const std::string str = "x1}";
      std::stringstream stream;
      stream << str;
      Symbolic::NumberExpression expr;
      stream >> expr;
      BOOST_TEST(!stream.eof());
      BOOST_TEST(!stream.fail());
      BOOST_CHECK_EQUAL(static_cast<char>(stream.peek()), '}');
    }

  BOOST_AUTO_TEST_SUITE_END() // SymbolicNumberExpression

BOOST_AUTO_TEST_SUITE_END() // IOOperatorsTest