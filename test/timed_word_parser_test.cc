#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <sstream>
#include "../src/timed_word_parser.hh"

BOOST_AUTO_TEST_SUITE(TimedWordParserTest)

struct WithdrawWordFixture {
  std::stringstream sigStream;
  std::stringstream wordStream;
  void execute() {
    // Construct signature for withdraw
    Signature sig(sigStream);
    TimedWordParser<int> parser {wordStream, sig};

    const std::vector<TimedWordEvent<int>> expectedEvents =
      {{0, {"Alice"}, {6000}, 10},
       {0, {"Bob"}, {300}, 20},                         
       {0, {"Dan"}, {300}, 20},
       {0, {"Charlie"}, {2000}, 20},
       {0, {"Alice"}, {6000}, 30},
       {0, {"Charlie"}, {9000}, 60}
      };

    TimedWordEvent<int> event;

    for (const auto &expectedEvent: expectedEvents) {
      BOOST_TEST(parser.parse(event));
      BOOST_CHECK_EQUAL(event.actionId, expectedEvent.actionId);
      BOOST_CHECK_EQUAL(event.strings.size(), expectedEvent.strings.size());
      BOOST_CHECK_EQUAL(event.strings.front(), expectedEvent.strings.front());
      BOOST_CHECK_EQUAL(event.numbers.size(), expectedEvent.numbers.size());
      BOOST_CHECK_EQUAL(event.numbers.front(), expectedEvent.numbers.front());
      BOOST_CHECK_EQUAL(event.strings.size(), expectedEvent.strings.size());
      BOOST_CHECK_EQUAL(event.timestamp, expectedEvent.timestamp);
    }
    BOOST_TEST(!parser.parse(event));
  }
};

BOOST_FIXTURE_TEST_CASE(withdraw, WithdrawWordFixture)
{
  sigStream << "withdraw\t1\t1" << "\n";
  wordStream << "withdraw\tAlice\t6000\t10" << "\n"
             << "withdraw\tBob\t300\t20" << "\n"
             << "withdraw\tDan\t300\t20" << "\n"
             << "withdraw\tCharlie\t2000\t20" << "\n"
             << "withdraw\tAlice\t6000\t30" << "\n"
             << "withdraw\tCharlie\t9000\t60";
  execute();
}

BOOST_FIXTURE_TEST_CASE(withdrawWithoutLF, WithdrawWordFixture)
{
  sigStream << "withdraw\t1\t1";
  wordStream << "withdraw\tAlice\t6000\t10" << "\n"
             << "withdraw\tBob\t300\t20" << "\n"
             << "withdraw\tDan\t300\t20" << "\n"
             << "withdraw\tCharlie\t2000\t20" << "\n"
             << "withdraw\tAlice\t6000\t30" << "\n"
             << "withdraw\tCharlie\t9000\t60" << "\n";
  execute();
}

BOOST_AUTO_TEST_SUITE_END()
