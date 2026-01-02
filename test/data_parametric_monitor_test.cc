#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../src/data_parametric_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"
#include "../test/fixture/epsilon_transition_automaton_fixture.hh"

using TWEvent = TimedWordEvent<PPLRational>;

struct DummyDataTimedWordSubject : public SingleSubject<TWEvent> {
  DummyDataTimedWordSubject(std::vector<TWEvent> &&vec) :vec(std::move(vec)) {}
  virtual ~DummyDataTimedWordSubject(){}
  void notifyAll() {
    for (const auto &event: vec) {
      notifyObservers(event);
    }
    vec.clear();
  }
  std::vector<TWEvent> vec;
};

struct DummyDataParametricMonitorObserver : public Observer<DataParametricMonitorResult> {
  DummyDataParametricMonitorObserver() {}
  virtual ~DummyDataParametricMonitorObserver() {}
  void notify(const DataParametricMonitorResult& result) {
    resultVec.push_back(result);
  }
  std::vector<DataParametricMonitorResult> resultVec;
};

struct DataParametricMonitorFixture {
  void feed(DataParametricTA automaton, std::vector<TWEvent> &&vec) {
    auto monitor = std::make_shared<DataParametricMonitor>(automaton);
    std::shared_ptr<DummyDataParametricMonitorObserver> observer = std::make_shared<DummyDataParametricMonitorObserver>();
    monitor->addObserver(observer);
    DummyDataTimedWordSubject subject{std::move(vec)};
    subject.addObserver(monitor); //&monitor, DataParametricMonitor, should be Observer<TWEvent>
    subject.notifyAll();
    // Ensure the monitor's destructor runs now to emit epsilon-transition notifications
    subject.addObserver(nullptr); // release subject's shared ownership
    monitor.reset();            // release local ownership
    resultVec = std::move(observer->resultVec);
  }
  std::vector<DataParametricMonitorResult> resultVec;
};

BOOST_AUTO_TEST_SUITE(DataParametricMonitorTest)

BOOST_FIXTURE_TEST_CASE(test1, DataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {50}, 0.1};
  dummyTimedWord[1] = {0, {"x"}, {51, 2}, 1.5};
  dummyTimedWord[2] = {0, {"y"}, {200}, 10};
  dummyTimedWord[3] = {0, {"x"}, {200}, 15};
  feed(DataParametricCopy().automaton, std::move(dummyTimedWord));
  BOOST_TEST(resultVec.empty());
}

BOOST_FIXTURE_TEST_CASE(test2, DataParametricMonitorFixture)
{
  std::vector<TWEvent> dummyTimedWord(4);
  dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
  dummyTimedWord[1] = {0, {"y"}, {100, 3}, 10};
  dummyTimedWord[2] = {0, {"x"}, {100, 3}, 12};
  dummyTimedWord[3] = {0, {"z"}, {100, 3}, 15.5};
  feed(DataParametricCopy().automaton, std::move(dummyTimedWord));
  BOOST_CHECK_EQUAL(resultVec.size(), 1);
  BOOST_CHECK_EQUAL(resultVec.front().index, 3);
  BOOST_CHECK_EQUAL(resultVec.front().timestamp, 15.5);
}

BOOST_FIXTURE_TEST_CASE(epsilon_test1, DataParametricMonitorFixture)
{
  auto automaton = EpsilonTransitionAutomatonFixture().makeDataParametricTA();

  std::vector<TWEvent> timedWord{
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

BOOST_FIXTURE_TEST_CASE(epsilon_test2, DataParametricMonitorFixture)
{
  auto automaton = EpsilonTransitionAutomatonFixture2().makeDataParametricTA();

  std::vector<TWEvent> timedWord{
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

BOOST_FIXTURE_TEST_CASE(epsilon_test3, DataParametricMonitorFixture)
{
  auto automaton = EpsilonTransitionAutomatonFixture3().makeDataParametricTA();

  std::vector<TWEvent> timedWord{
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

BOOST_FIXTURE_TEST_CASE(epsilon_test4, DataParametricMonitorFixture)
{
  auto automaton = EpsilonTransitionToAcceptStateAutomatonFixture().makeDataParametricTA();

  std::vector<TWEvent> timedWord{
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
