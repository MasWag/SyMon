/*!
 * @author Masaki Waga
 * @date 2019-01-29
*/

#include "../src/parametric_monitor.hh"

namespace Parma_Polyhedra_Library {
  static inline
  std::size_t hash_value(const Symbolic::NumberValuation &p) {
    return static_cast<std::size_t>(p.hash_code());
  }
}

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using TWEvent = TimedWordEvent<Parma_Polyhedra_Library::Coefficient, Parma_Polyhedra_Library::Coefficient>;

struct DummyParametricTimedWordSubject : public SingleSubject<TWEvent> {
  DummyParametricTimedWordSubject(std::vector<TWEvent> &&vec) : vec(std::move(vec)) {}

  virtual ~DummyParametricTimedWordSubject() {}

  void notifyAll() {
    for (const auto &event: vec) {
      notifyObservers(event);
    }
    vec.clear();
  }

  std::vector<TWEvent> vec;
};

struct DummyParametricMonitorObserver : public Observer<ParametricMonitorResult> {
  DummyParametricMonitorObserver() {}

  virtual ~DummyParametricMonitorObserver() {}

  void notify(const ParametricMonitorResult &result) {
    resultVec.push_back(result);
  }

  std::vector<ParametricMonitorResult> resultVec;
};

struct ParametricMonitorFixture {
  void feed(ParametricTA automaton, std::vector<TWEvent> &&vec) {
    auto monitor = std::make_shared<ParametricMonitor<false>>(automaton);
    auto observer = std::make_shared<DummyParametricMonitorObserver>();
    monitor->addObserver(observer);
    DummyParametricTimedWordSubject subject{std::move(vec)};
    subject.addObserver(monitor);
    subject.notifyAll();
    resultVec = std::move(observer->resultVec);
  }

  std::vector<ParametricMonitorResult> resultVec;
};

BOOST_AUTO_TEST_SUITE(ParametricMonitorTest)

  namespace bdata = boost::unit_test::data;

  BOOST_DATA_TEST_CASE_F(ParametricMonitorFixture, bugFix20190129Gt, bdata::make({9, 10, 11}) ^ bdata::make(1, 1, 0),
                         timestamp, result) {
    ParametricTA automaton;
    automaton.clockVariableSize = 1;
    automaton.parameterSize = 0;
    automaton.stringVariableSize = 0;
    automaton.numberVariableSize = 0;
    automaton.states.resize(3);
    automaton.states[0] = std::make_shared<PTAState>(false);
    automaton.states[1] = std::make_shared<PTAState>(false);
    automaton.states[2] = std::make_shared<PTAState>(true);
    automaton.initialStates = {automaton.states[0]};

    using namespace Parma_Polyhedra_Library;

    automaton.states[0]->next[127].resize(1);
    automaton.states[0]->next[127].at(0).guard = Parma_Polyhedra_Library::NNC_Polyhedron(1);
    automaton.states[0]->next[127].at(0).target = automaton.states[1];

    automaton.states[1]->next[0].resize(1);
    automaton.states[1]->next[0].at(0).guard = Parma_Polyhedra_Library::NNC_Polyhedron(1);
    automaton.states[1]->next[0].at(0).guard.add_constraint(Variable(0) > 10);
    automaton.states[1]->next[0].at(0).target = automaton.states[2];

    std::vector<TWEvent> dummyTimedWord(1);
    dummyTimedWord.at(0) = {0, {}, {}, timestamp};
    feed(automaton, std::move(dummyTimedWord));
    BOOST_CHECK_EQUAL(resultVec.empty(), result);
  }

  BOOST_DATA_TEST_CASE_F(ParametricMonitorFixture, bugFix20190129Lt, bdata::make({9, 10, 11}) ^ bdata::make(0, 1, 1),
                         timestamp, result) {
    ParametricTA automaton;
    automaton.clockVariableSize = 1;
    automaton.parameterSize = 0;
    automaton.stringVariableSize = 0;
    automaton.numberVariableSize = 0;
    automaton.states.resize(3);
    automaton.states[0] = std::make_shared<PTAState>(false);
    automaton.states[1] = std::make_shared<PTAState>(false);
    automaton.states[2] = std::make_shared<PTAState>(true);
    automaton.initialStates = {automaton.states[0]};

    using namespace Parma_Polyhedra_Library;

    automaton.states[0]->next[127].resize(1);
    automaton.states[0]->next[127].at(0).guard = Parma_Polyhedra_Library::NNC_Polyhedron(1);
    automaton.states[0]->next[127].at(0).target = automaton.states[1];

    automaton.states[1]->next[0].resize(1);
    automaton.states[1]->next[0].at(0).guard = Parma_Polyhedra_Library::NNC_Polyhedron(1);
    automaton.states[1]->next[0].at(0).guard.add_constraint(Variable(0) < 10);
    automaton.states[1]->next[0].at(0).target = automaton.states[2];

    std::vector<TWEvent> dummyTimedWord(1);
    dummyTimedWord.at(0) = {0, {}, {}, timestamp};
    feed(automaton, std::move(dummyTimedWord));
    BOOST_CHECK_EQUAL(resultVec.empty(), result);
  }

BOOST_AUTO_TEST_SUITE_END()
