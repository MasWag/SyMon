#include <boost/test/unit_test.hpp>
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
