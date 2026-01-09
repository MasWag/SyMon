/*!
 * @author Masaki Waga
 * @date 2019-01-29
*/

#include "../src/parametric_monitor.hh"
#include "../test/fixture/non_integer_timestamp_fixture.hh"
#include "ppl_rational.hh"
#include "symbolic_string_constraint.hh"
#include <ppl.hh>
#include <sstream>

namespace Parma_Polyhedra_Library {
  static inline
  std::size_t hash_value(const Symbolic::NumberValuation &p) {
    return static_cast<std::size_t>(p.hash_code());
  }
}

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using TWEvent = TimedWordEvent<PPLRational, PPLRational>;

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
    auto monitor = std::make_shared<ParametricMonitor>(automaton);
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

  BOOST_FIXTURE_TEST_CASE(none, ParametricMonitorFixture) {
    ParametricTA automaton;
    automaton.clockVariableSize = 0;
    automaton.parameterSize = 0;
    automaton.stringVariableSize = 1;
    automaton.numberVariableSize = 1;
    automaton.states.resize(2);
    automaton.states[0] = std::make_shared<PTAState>(false);
    automaton.states[1] = std::make_shared<PTAState>(true);
    automaton.initialStates = {automaton.states[0]};

    using namespace Parma_Polyhedra_Library;

    automaton.states[0]->next[0].resize(1);
    automaton.states[0]->next[0].at(0).target = automaton.states[1];
    automaton.states[0]->next[0].at(0).numConstraints.emplace_back(Variable(0) == Variable(1));
    automaton.states[0]->next[0].at(0).stringConstraints.emplace_back(Symbolic::StringConstraint{{Symbolic::StringAtom{VariableID(0)}, Symbolic::StringAtom{VariableID(1)}}, Symbolic::StringConstraint::kind_t::EQ});

    std::vector<TWEvent> dummyTimedWord(1);
    PPLRational timestamp = 2;
    dummyTimedWord.at(0) = {0, {"foo"}, {PPLRational{2, 5}}, timestamp};
    feed(automaton, std::move(dummyTimedWord));
    BOOST_CHECK_EQUAL(1, resultVec.size());

    using namespace Parma_Polyhedra_Library::IO_Operators;
    std::stringstream ss;
    ss << resultVec.at(0).numberValuation;
    BOOST_CHECK_EQUAL("5*A = 2", ss.str());

    BOOST_CHECK_EQUAL(1, resultVec.at(0).stringValuation.size());
    BOOST_CHECK_EQUAL(1, resultVec.at(0).stringValuation.at(0).index());
    BOOST_CHECK_EQUAL("foo", std::get<std::string>(resultVec.at(0).stringValuation.at(0)));
  }

  BOOST_FIXTURE_TEST_CASE(non_integer_timestamp_test, ParametricMonitorFixture) {
    auto automaton = Parametric::ParametricNonIntegerTimestampFixture().automaton;

      std::vector<TWEvent> dummyTimedWord{
        {0, {}, {0}, {0, 1}},
        {0, {}, {0}, {100, 100}},
        {0, {}, {0}, {210, 100}},
        {0, {}, {0}, {330, 100}},
        {0, {}, {0}, {445, 100}}
      };
      feed(automaton, std::move(dummyTimedWord));
      BOOST_CHECK_EQUAL(resultVec.size(), 2);
      BOOST_CHECK_EQUAL(resultVec.front().index, 2);
      auto t0 = PPLRational{21, 10};
      BOOST_CHECK_EQUAL(resultVec.front().timestamp, t0);
      BOOST_CHECK_EQUAL(resultVec[1].index, 4);
      auto t1 = PPLRational{445, 100};
      BOOST_CHECK_EQUAL(resultVec[1].timestamp, t1);
  }

BOOST_AUTO_TEST_SUITE_END()

