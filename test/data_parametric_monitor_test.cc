#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../src/data_parametric_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"

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
BOOST_AUTO_TEST_SUITE_END()
