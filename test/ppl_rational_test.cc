#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <sstream>

#ifndef mem_fun_ref
#define mem_fun_ref mem_fn
#endif

#include "../src/ppl_rational.hh"

BOOST_AUTO_TEST_SUITE(PPLRationalIStreamTests)

BOOST_AUTO_TEST_CASE(parse_integer) {
  std::istringstream is("42");
  PPLRational r;
  is >> r;
  BOOST_TEST(!is.bad());
  BOOST_TEST(r.getNumerator() == 42);
  BOOST_TEST(r.getDenominator() == 1);
}

BOOST_AUTO_TEST_CASE(parse_decimal_simple) {
  std::istringstream is("3.5");
  PPLRational r;
  is >> r;
  BOOST_TEST(!is.bad());
  BOOST_TEST(r.getNumerator() == 7);
  BOOST_TEST(r.getDenominator() == 2);
}

BOOST_AUTO_TEST_CASE(parse_leading_dot) {
  std::istringstream is(".2");
  PPLRational r;
  is >> r;
  BOOST_TEST(!is.bad());
  BOOST_TEST(r.getNumerator() == 1);
  BOOST_TEST(r.getDenominator() == 5);
}

BOOST_AUTO_TEST_CASE(parse_negative_decimal) {
  std::istringstream is("-1.05");
  PPLRational r;
  is >> r;
  BOOST_TEST(!is.bad());
  BOOST_TEST(r.getNumerator() == -21);
  BOOST_TEST(r.getDenominator() == 20);
}

BOOST_AUTO_TEST_CASE(parse_with_whitespace_and_plus) {
  std::istringstream is("   +10.0\n");
  PPLRational r;
  is >> r;
  BOOST_TEST(!is.fail());
  BOOST_TEST(r.getNumerator() == 10);
  BOOST_TEST(r.getDenominator() == 1);
}

BOOST_AUTO_TEST_CASE(invalid_multiple_dots_sets_failbit) {
  std::istringstream is("1.2.3");
  PPLRational r(0, 1);
  
  is >> r;
  BOOST_TEST(is.fail());
  // Stream should not have consumed beyond the invalid sequence handling
  // We only assert that the original value wasn't changed by a successful parse
  // and that failbit is set.
}

BOOST_AUTO_TEST_CASE(parse_sequence) {
  std::istringstream is("0.25 -2 .5 3");
  PPLRational a, b, c, d;
  is >> a >> b >> c >> d;
  BOOST_TEST(!is.bad());
  BOOST_TEST(is.eof());
  BOOST_TEST(a.getNumerator() == 1);
  BOOST_TEST(a.getDenominator() == 4);
  BOOST_TEST(b.getNumerator() == -2);
  BOOST_TEST(b.getDenominator() == 1);
  BOOST_TEST(c.getNumerator() == 1);
  BOOST_TEST(c.getDenominator() == 2);
  BOOST_TEST(d.getNumerator() == 3);
  BOOST_TEST(d.getDenominator() == 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PPLRationalOperatorMinusTests)

BOOST_AUTO_TEST_CASE(binary_subtract_simple) {
  PPLRational a(3, 4);  // 0.75
  PPLRational b(1, 2);  // 0.5
  PPLRational c = a - b; // 0.25
  BOOST_TEST(c.getNumerator() == 1);
  BOOST_TEST(c.getDenominator() == 4);
}

BOOST_AUTO_TEST_CASE(binary_subtract_negative_result) {
  PPLRational a(1, 3);
  PPLRational b(2, 3);
  PPLRational c = a - b; // -1/3
  BOOST_TEST(c.getNumerator() == -1);
  BOOST_TEST(c.getDenominator() == 3);
}

BOOST_AUTO_TEST_CASE(binary_subtract_to_zero) {
  PPLRational a(5, 7);
  PPLRational b(5, 7);
  PPLRational c = a - b; // 0
  BOOST_TEST(c.getNumerator() == 0);
  BOOST_TEST(c.getDenominator() == 1);
}

BOOST_AUTO_TEST_CASE(unary_negation) {
  PPLRational a(2, 5);
  PPLRational b = -a; // -2/5
  BOOST_TEST(b.getNumerator() == -2);
  BOOST_TEST(b.getDenominator() == 5);
  PPLRational c = -b; // back to 2/5
  BOOST_TEST(c.getNumerator() == 2);
  BOOST_TEST(c.getDenominator() == 5);
}

BOOST_AUTO_TEST_CASE(mixed_integer_and_fraction) {
  PPLRational one(1, 1);
  PPLRational twoThirds(2, 3);
  PPLRational r = one - twoThirds; // 1/3
  BOOST_TEST(r.getNumerator() == 1);
  BOOST_TEST(r.getDenominator() == 3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PPLRationalOStreamTests)
namespace bdata = boost::unit_test::data;

BOOST_DATA_TEST_CASE(random_decimal, bdata::xrange(100) ^ bdata::random( bdata::distribution = std::uniform_real_distribution<double>(-2, 2)), idx, value) {
  std::stringstream ss;
  ss << value;
  std::istringstream is(ss.str());
  std::string asString = ss.str();
  PPLRational r;
  is >> r;
  ss.str("");
  ss << r;
  BOOST_TEST(ss.str() == asString);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PPLRationalComparisonTests)

  BOOST_AUTO_TEST_CASE(compare_equal_with_rationals) {
    PPLRational twoFourths(2, 4);
    PPLRational threeSixths(3, 6);
    BOOST_CHECK_EQUAL(twoFourths, threeSixths);
  }

  BOOST_AUTO_TEST_CASE(compare_equal_with_integer) {
    PPLRational fourHalves(4, 2);
    BOOST_CHECK_EQUAL(fourHalves, 2);
  }
BOOST_AUTO_TEST_SUITE_END()
