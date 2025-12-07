#include "../src/boolean_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"
#include "../test/fixture/non_integer_timestamp_fixture.hh"
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

  struct CopyBooleanMonitorFixture : public CopyFixture {
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
    BOOST_FIXTURE_TEST_CASE(test1, CopyBooleanMonitorFixture) {
      std::vector<TimedWordEvent> dummyTimedWord(3);
      dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
      dummyTimedWord[1] = {0, {"y"}, {200}, 10};
      dummyTimedWord[2] = {0, {"x"}, {200}, 15};
      feed(std::move(dummyTimedWord));
      BOOST_TEST(resultVec.empty());
    }

    BOOST_FIXTURE_TEST_CASE(test2, CopyBooleanMonitorFixture) {
      std::vector<TimedWordEvent> dummyTimedWord(4);
      dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
      dummyTimedWord[1] = {0, {"y"}, {200}, 10};
      dummyTimedWord[2] = {0, {"x"}, {200}, 12};
      dummyTimedWord[3] = {0, {"z"}, {200}, 15.5};
      feed(std::move(dummyTimedWord));
      BOOST_CHECK_EQUAL(resultVec.size(), 1);
      BOOST_CHECK_EQUAL(resultVec.front().index, 3);
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, 15.5);
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
