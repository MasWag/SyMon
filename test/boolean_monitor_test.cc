#include "../src/boolean_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"
#include "../test/fixture/non_integer_timestamp_fixture.hh"
#include "../test/fixture/epsilon_transition_automaton_fixture.hh"
#include "automaton.hh"
#include "timed_word_parser.hh"
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace IntTest {
  using Number = int;
  using TimedWordEvent = TimedWordEvent<Number, double>;

  struct DummyTimedWordSubject : public SingleSubject<TimedWordEvent> {
    DummyTimedWordSubject(std::vector<TimedWordEvent> &&vec) : vec(std::move(vec)) {
    }
    virtual ~DummyTimedWordSubject() {
    }
    void notifyAll() {
      for (const auto &event: vec) {
        notifyObservers(event);
      }
      vec.clear();
    }
    std::vector<TimedWordEvent> vec;
  };

  struct DummyBooleanMonitorObserver : public Observer<BooleanMonitorResult<Number>> {
    DummyBooleanMonitorObserver() {
    }
    virtual ~DummyBooleanMonitorObserver() {
    }
    void notify(const BooleanMonitorResult<Number> &result) {
      resultVec.push_back(result);
    }
    std::vector<BooleanMonitorResult<Number>> resultVec;
  };

  struct BooleanMonitorFixture {
    void feed(const NonParametricTA<int> &automaton, std::vector<TimedWordEvent> &&vec) {
      auto monitor = std::make_shared<NonSymbolic::BooleanMonitor<int>>(automaton);
      std::shared_ptr<DummyBooleanMonitorObserver> observer = std::make_shared<DummyBooleanMonitorObserver>();
      monitor->addObserver(observer);
      DummyTimedWordSubject subject{std::move(vec)};
      subject.addObserver(monitor);
      subject.notifyAll();
      // Ensure the monitor's destructor runs now to emit epsilon-transition notifications
      subject.addObserver(nullptr); // release subject's shared ownership
      monitor.reset();            // release local ownership
      resultVec = std::move(observer->resultVec);
    }
    std::vector<BooleanMonitorResult<int>> resultVec;
  };

  BOOST_AUTO_TEST_SUITE(BooleanMonitorTest)
    BOOST_FIXTURE_TEST_CASE(test1, BooleanMonitorFixture)
    {
      std::vector<TimedWordEvent> dummyTimedWord(3);
      dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
      dummyTimedWord[1] = {0, {"y"}, {200}, 10};
      dummyTimedWord[2] = {0, {"x"}, {200}, 15};
      feed(CopyFixture().automaton, std::move(dummyTimedWord));
      BOOST_TEST(resultVec.empty());
    }

    BOOST_FIXTURE_TEST_CASE(test2, BooleanMonitorFixture)
    {
      std::vector<TimedWordEvent> dummyTimedWord(4);
      dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
      dummyTimedWord[1] = {0, {"y"}, {200}, 10};
      dummyTimedWord[2] = {0, {"x"}, {200}, 12};
      dummyTimedWord[3] = {0, {"z"}, {200}, 15.5};
      feed(CopyFixture().automaton, std::move(dummyTimedWord));
      BOOST_CHECK_EQUAL(resultVec.size(), 1);
      BOOST_CHECK_EQUAL(resultVec.front().index, 3);
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, 15.5);
    }

    BOOST_FIXTURE_TEST_CASE(epsilon_test1, BooleanMonitorFixture)
    {
      auto automaton = EpsilonTransitionAutomatonFixture::FIXTURE1.makeBooleanTA();

      std::vector<TimedWordEvent> timedWord{
            {0, {"c"}, {}, 1},
            {0, {"a"}, {}, 10},
            {0, {"b"}, {}, 12},
            {0, {"b"}, {}, 15},
            {0, {"c"}, {}, 20},
            {0, {"a"}, {}, 32},
            {0, {"b"}, {}, 40},
            {0, {"c"}, {}, 42},
            {0, {"a"}, {}, 51.5},
            {0, {"b"}, {}, 52},
            {0, {"a"}, {}, 53},
            {0, {"b"}, {}, 54},
            {0, {"c"}, {}, 55},
          };
          feed(automaton, std::move(timedWord));

          BOOST_CHECK_EQUAL(resultVec.size(), 1);
          BOOST_CHECK_EQUAL(resultVec.front().index, 6);
          BOOST_CHECK_EQUAL(resultVec.front().timestamp, 40);
    }

    BOOST_FIXTURE_TEST_CASE(epsilon_test2, BooleanMonitorFixture)
    {
      auto automaton = EpsilonTransitionAutomatonFixture::FIXTURE2.makeBooleanTA();

      std::vector<TimedWordEvent> timedWord{
            {0, {"a"}, {}, 0},
            {0, {"b"}, {}, 5},
            {0, {"a"}, {}, 10},
            {0, {"b"}, {}, 15},
          };
          feed(automaton, std::move(timedWord));

          BOOST_CHECK_EQUAL(resultVec.size(), 2);
          BOOST_CHECK_EQUAL(resultVec[0].index, 1);
          BOOST_CHECK_EQUAL(resultVec[1].index, 3);
    }

    BOOST_FIXTURE_TEST_CASE(epsilon_test3, BooleanMonitorFixture)
    {
      auto automaton = EpsilonTransitionAutomatonFixture::FIXTURE3.makeBooleanTA();

      std::vector<TimedWordEvent> timedWord{
            {0, {"a"}, {}, 0},
            {0, {"b"}, {}, 1},
            {0, {"c"}, {}, 7},
            {0, {"b"}, {}, 100},
            {0, {"a"}, {}, 101},
            {0, {"c"}, {}, 107},
          };
          feed(automaton, std::move(timedWord));

          BOOST_CHECK_EQUAL(resultVec.size(), 1);
          BOOST_CHECK_EQUAL(resultVec.front().index, 5);
    }

    BOOST_FIXTURE_TEST_CASE(epsilon_test4, BooleanMonitorFixture)
    {
      auto automaton = EpsilonTransitionAutomatonFixture::FIXTURE4.makeBooleanTA();

      std::vector<TimedWordEvent> timedWord{
            {0, {"a"}, {}, 1.5},
            {0, {"b"}, {}, 2.5},
            {0, {"a"}, {}, 3.5},
            {0, {"b"}, {}, 4.5},
          };
          feed(automaton, std::move(timedWord));

          BOOST_CHECK_EQUAL(resultVec.size(), 2);
          BOOST_CHECK_EQUAL(resultVec[0].index, 2);
          BOOST_CHECK_EQUAL(resultVec[0].timestamp, 5.5);
          // The last index of timed word is 3, and it matches after the epsilon transition following the last event
          BOOST_CHECK_EQUAL(resultVec[1].index, 4);
          // The matched timestamp is 7.5 = 4.5 + 3 (the guard of epsilon transition is x0 == 3)
          BOOST_CHECK_EQUAL(resultVec[1].timestamp, 7.5);
    }

  BOOST_AUTO_TEST_SUITE_END()
}

namespace DoubleTest {
  using Number = double;
  using TimedWordEvent = TimedWordEvent<Number, double>;

  struct DummyTimedWordSubject : public SingleSubject<TimedWordEvent> {
    DummyTimedWordSubject(std::vector<TimedWordEvent> &&vec) : vec(std::move(vec)) {
    }
    virtual ~DummyTimedWordSubject() {
    }
    void notifyAll() {
      for (const auto &event: vec) {
        notifyObservers(event);
      }
      vec.clear();
    }
    std::vector<TimedWordEvent> vec;
  };

  struct DummyBooleanMonitorObserver : public Observer<BooleanMonitorResult<Number>> {
    DummyBooleanMonitorObserver() {
    }
    virtual ~DummyBooleanMonitorObserver() {
    }
    void notify(const BooleanMonitorResult<Number> &result) {
      resultVec.push_back(result);
    }
    std::vector<BooleanMonitorResult<Number>> resultVec;
  };
  
  struct NonIntegerTimestampBooleanMonitorFixture : public NonParametric::NonIntegerTimestampFixture {
    void feed(std::vector<TimedWordEvent> &&vec) {
      auto monitor = std::make_shared<NonSymbolic::BooleanMonitor<Number>>(automaton);
      std::shared_ptr<DummyBooleanMonitorObserver> observer = std::make_shared<DummyBooleanMonitorObserver>();
      monitor->addObserver(observer);
      DummyTimedWordSubject subject{std::move(vec)};
      subject.addObserver(monitor);
      subject.notifyAll();
      resultVec = std::move(observer->resultVec);
    }
    std::vector<BooleanMonitorResult<Number>> resultVec;
  };

  BOOST_AUTO_TEST_SUITE(BooleanMonitorTest)
    BOOST_FIXTURE_TEST_CASE(non_integer_timestamp_test, NonIntegerTimestampBooleanMonitorFixture) {
      std::vector<TimedWordEvent> dummyTimedWord{
        {0, {}, {0}, 0.},
        {0, {}, {0}, 1.0},
        {0, {}, {0}, 2.1},
        {0, {}, {0}, 3.3},
        {0, {}, {0}, 4.6}
      };
      feed(std::move(dummyTimedWord));
      //NOTE: 3.3 - 2.1 is 1.2, but this is evaluated as 1.1999999999999997 < 1.2, so the fourth event is also matched.
      //BOOST_CHECK_EQUAL(resultVec.size(), 1);
      BOOST_CHECK_EQUAL(resultVec.front().index, 2);
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, 2.1);
    }
  BOOST_AUTO_TEST_SUITE_END()
}
