#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "../src/boolean_monitor.hh"
#include "../test/fixture/copy_automaton_fixture.hh"

struct DummyTimedWordSubject : public SingleSubject<TimedWordEvent<int>> {
  DummyTimedWordSubject(std::vector<TimedWordEvent<int>> &&vec) :vec(std::move(vec)) {}
  virtual ~DummyTimedWordSubject(){}
  void notifyAll() {
    for (const auto &event: vec) {
      notifyObservers(event);
    }
    vec.clear();
  }
  std::vector<TimedWordEvent<int>> vec;
};

struct DummyBooleanMonitorObserver : public Observer<BooleanMonitorResult<int>> {
  DummyBooleanMonitorObserver() {}
  virtual ~DummyBooleanMonitorObserver() {}
  void notify(const BooleanMonitorResult<int>& result) {
    resultVec.push_back(result);
  }
  std::vector<BooleanMonitorResult<int>> resultVec;
};

struct CopyBooleanMonitorFixture : public CopyFixture {
  void feed(std::vector<TimedWordEvent<int>> &&vec) {
    auto monitor = std::make_shared<NonSymbolic::BooleanMonitor<int>>(automaton);
    std::shared_ptr<DummyBooleanMonitorObserver> observer = std::make_shared<DummyBooleanMonitorObserver>();
    monitor->addObserver(observer);
    DummyTimedWordSubject subject{std::move(vec)};
    subject.addObserver(monitor);
    subject.notifyAll();
    resultVec = std::move(observer->resultVec);
  }
  std::vector<BooleanMonitorResult<int>> resultVec;
};

BOOST_AUTO_TEST_SUITE(BooleanMonitorTest)

BOOST_FIXTURE_TEST_CASE(test1, CopyBooleanMonitorFixture)
{
  std::vector<TimedWordEvent<int>> dummyTimedWord(3);
  dummyTimedWord[0] = {0, {"x"}, {100}, 0.1};
  dummyTimedWord[1] = {0, {"y"}, {200}, 10};
  dummyTimedWord[2] = {0, {"x"}, {200}, 15};
  feed(std::move(dummyTimedWord));
  BOOST_TEST(resultVec.empty());
}

BOOST_FIXTURE_TEST_CASE(test2, CopyBooleanMonitorFixture)
{
  std::vector<TimedWordEvent<int>> dummyTimedWord(4);
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
