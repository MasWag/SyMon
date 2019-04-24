#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <sstream>
#include "../src/signature.hh"

BOOST_AUTO_TEST_SUITE(SignatureTest)

BOOST_AUTO_TEST_CASE(test1)
{
  std::stringstream ss;
  ss << "type1\t1\t2" << "\n"
     << "type2\t2\t1" << "\n";
  Signature sig(ss);

  BOOST_CHECK_EQUAL(sig.getStringSize("type1"), 1u);
  BOOST_CHECK_EQUAL(sig.getNumberSize("type1"), 2u);

  BOOST_CHECK_EQUAL(sig.getStringSize("type2"), 2u);
  BOOST_CHECK_EQUAL(sig.getNumberSize("type2"), 1u);

  BOOST_CHECK_THROW(sig.getNumberSize("type3"), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
