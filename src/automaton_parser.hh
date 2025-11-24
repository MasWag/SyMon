#pragma once

#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_string_constraint.hh"

#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"

#include "io_operators.hh"

#include "parametric_timing_constraint_helper.hh"
#include "timing_constraint.hh"

namespace boost {
  using ::operator>>;
  using ::operator<<;
  using namespace Parma_Polyhedra_Library::IO_Operators;
  namespace detail {
    using ::operator>>;
    using ::operator<<;
  }
} // namespace boost

namespace std {
  using namespace Parma_Polyhedra_Library::IO_Operators;
}

//(setq flycheck-clang-language-standard "c++17")
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <fstream>
#include <iostream>

#include "automaton.hh"
#include "non_symbolic_update.hh"
#include "symbolic_update.hh"

namespace boost {
  enum vertex_match_t { vertex_match };
  enum edge_label_t { edge_label };
  enum edge_reset_t { edge_reset };
  enum edge_guard_t { edge_guard };
  enum graph_clock_variable_size_t { graph_clock_variable_size };
  enum graph_string_variable_size_t { graph_string_variable_size };
  enum graph_number_variable_size_t { graph_number_variable_size };
  enum graph_parameter_size_t { graph_parameter_size };

  BOOST_INSTALL_PROPERTY(graph, clock_variable_size);
  BOOST_INSTALL_PROPERTY(graph, string_variable_size);
  BOOST_INSTALL_PROPERTY(graph, number_variable_size);
  BOOST_INSTALL_PROPERTY(graph, parameter_size);
  BOOST_INSTALL_PROPERTY(vertex, match);
  BOOST_INSTALL_PROPERTY(edge, label);
  BOOST_INSTALL_PROPERTY(edge, reset);
  BOOST_INSTALL_PROPERTY(edge, guard);
} // namespace boost

template <typename Timestamp>
static inline std::ostream &operator<<(std::ostream &os, const TimingConstraintOrder &odr) {
  switch (odr) {
    case TimingConstraintOrder::lt:
      os << "<";
      break;
    case TimingConstraintOrder::le:
      os << "<=";
      break;
    case TimingConstraintOrder::ge:
      os << ">=";
      break;
    case TimingConstraintOrder::gt:
      os << ">";
      break;
    case TimingConstraint::Order::eq:
      os << "==";
      break;
  }
  return os;
}

static inline std::ostream &operator<<(std::ostream &os, const TimingConstraintOrder &odr) {
  switch (odr) {
    case TimingConstraintOrder::lt:
      os << "<";
      break;
    case TimingConstraintOrder::le:
      os << "<=";
      break;
    case TimingConstraintOrder::ge:
      os << ">=";
      break;
    case TimingConstraintOrder::gt:
      os << ">";
      break;
  }
  return os;
}

template <typename Timestamp>
static inline std::ostream &operator<<(std::ostream &os, const TimingConstraint<Timestamp> &p) {
  os << "x" << int(p.x) << " " << p.odr << " " << p.c;
  return os;
}

template <typename Timestamp>
static inline std::istream &operator>>(std::istream &is, TimingConstraint<Timestamp> &p) {
  if (is.get() != 'x') {
    is.setstate(std::ios_base::failbit);
    return is;
  }
  int x;
  is >> x;
  p.x = x;
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  char odr[2];
  is >> odr[0] >> odr[1];

  switch (odr[0]) {
    case '>':
      if (odr[1] == '=') {
        p.odr = TimingConstraintOrder::ge;
        if (is.get() != ' ') {
          is.setstate(std::ios_base::failbit);
          return is;
        }
      } else if (odr[1] == ' ') {
        p.odr = TimingConstraintOrder::gt;
      } else {
        is.setstate(std::ios_base::failbit);
        return is;
      }
      break;
    case '<':
      if (odr[1] == '=') {
        p.odr = TimingConstraintOrder::le;
        if (is.get() != ' ') {
          is.setstate(std::ios_base::failbit);
          return is;
        }
      } else if (odr[1] == ' ') {
        p.odr = TimingConstraintOrder::lt;
      } else {
        is.setstate(std::ios_base::failbit);
        return is;
      }
      break;
    case '=':
      if (odr[1] == '=') {
        p.odr = TimingConstraint::Order::eq;
        if (is.get() != ' ') {
          is.setstate(std::ios_base::failbit);
          return is;
        }
      } else {
        is.setstate(std::ios_base::failbit);
        return is;
      }
      break;
    default:
      is.setstate(std::ios_base::failbit);
      return is;
  }

  is >> p.c;
  return is;
}

static inline std::ostream &operator<<(std::ostream &os, const std::string &resetVars) {
  bool first = true;
  os << "{";
  for (const auto &x: resetVars) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << int(x);
  }
  os << "}";
  return os;
}

static inline std::istream &operator>>(std::istream &is, std::vector<ClockVariables> &resetVars) {
  resetVars.clear();
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != '{') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  while (true) {
    int x;
    is >> x;
    resetVars.emplace_back(ClockVariables(x));
    if (!is) {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    char c;
    is >> c;
    if (c == '}') {
      break;
    } else if (c == ',') {
      is >> c;
      if (c != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
  }

  return is;
}

namespace boost {
  template <class T> static inline std::ostream &operator<<(std::ostream &os, const boost::optional<T> &x) {
    if (x) {
      os << x.get();
    } else {
      os << "";
    }
    return os;
  }

  template <class T> static inline std::istream &operator>>(std::istream &is, boost::optional<T> &x) {
    T result;
    if (is >> result) {
      x = result;
    }
    return is;
  }

  using ::operator>>;
  using std::operator>>;
} // namespace boost

struct ResetVars {
  std::vector<ClockVariables> resetVars;
};

template <typename T> struct VectorWrapper {
  std::vector<T> values;
};

template <typename T> static inline std::istream &operator>>(std::istream &is, VectorWrapper<T> &wrapper) {
  return parse_vector(is, wrapper.values);
}

template <typename T> static inline std::ostream &operator<<(std::ostream &os, const VectorWrapper<T> &wrapper) {
  return os << wrapper.values;
}

static inline std::istream &operator>>(std::istream &is, ResetVars &resetVars) {
  is >> resetVars.resetVars;
  return is;
}

static inline std::ostream &operator<<(std::ostream &os, const ResetVars &resetVars) {
  os << resetVars.resetVars;
  return os;
}

template <typename TimingConstraint> struct Guard {
  std::vector<TimingConstraint> guard;
};

template <typename TimingConstraint>
static inline std::istream &operator>>(std::istream &is, Guard<TimingConstraint> &guard) {
  is >> guard.guard;
  return is;
}

template <typename TimingConstraint>
static inline std::ostream &operator<<(std::ostream &os, const Guard<TimingConstraint> &guard) {
  os << guard.guard;
  return os;
}

struct BoostTAState {
  bool isInit;
  bool isMatch;
};

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
struct BoostTATransition {
  Action c;
  VectorWrapper<StringConstraint> stringConstraints;
  VectorWrapper<NumberConstraint> numConstraints;
  VectorWrapper<typename decltype(std::declval<Update>().stringUpdate)::value_type> stringUpdate;
  VectorWrapper<typename decltype(std::declval<Update>().numberUpdate)::value_type> numberUpdate;
  //! @note this structure is necessary because of some problem in boost graph
  ResetVars resetVars;
  Guard<TimingConstraint> guard;
};

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
using BoostTimedAutomaton = boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS, BoostTAState,
    BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>,
    boost::property<boost::graph_clock_variable_size_t, std::size_t,
                    boost::property<boost::graph_string_variable_size_t, std::size_t,
                                    boost::property<boost::graph_number_variable_size_t, std::size_t>>>>;

template <typename Number, typename Timestamp>
using NonParametricBoostTA = BoostTimedAutomaton<NonSymbolic::StringConstraint, NonSymbolic::NumberConstraint<Number>,
                                                 TimingConstraint<Timestamp>, NonSymbolic::Update<Number>>;

template <typename Timestamp>
using DataParametricBoostTA =
    BoostTimedAutomaton<Symbolic::StringConstraint, Symbolic::NumberConstraint, TimingConstraint<Timestamp>, Symbolic::Update>;
using BoostPTA = boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS, BoostTAState,
    BoostTATransition<Symbolic::StringConstraint, Symbolic::NumberConstraint, ParametricTimingConstraintHelper,
                      Symbolic::Update>,
    boost::property<
        boost::graph_clock_variable_size_t, std::size_t,
        boost::property<boost::graph_string_variable_size_t, std::size_t,
                        boost::property<boost::graph_parameter_size_t, std::size_t,
                                        boost::property<boost::graph_number_variable_size_t, std::size_t>>>>>;

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
static inline void
parseBoostTA(std::istream &file,
             BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &BoostTA) {

  boost::dynamic_properties dp(boost::ignore_other_properties);

  boost::ref_property_map<BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> *,
                          std::size_t>
      gcvar_size(get_property(BoostTA, boost::graph_clock_variable_size));
  dp.property("clock_variable_size", gcvar_size);
  boost::ref_property_map<BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> *,
                          std::size_t>
      gsvar_size(get_property(BoostTA, boost::graph_string_variable_size));
  dp.property("string_variable_size", gsvar_size);
  boost::ref_property_map<BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> *,
                          std::size_t>
      gnvar_size(get_property(BoostTA, boost::graph_number_variable_size));
  dp.property("number_variable_size", gnvar_size);

  dp.property("match", boost::get(&BoostTAState::isMatch, BoostTA));
  dp.property("init", boost::get(&BoostTAState::isInit, BoostTA));
  dp.property("label",
              boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::c, BoostTA));
  dp.property(
      "reset",
      boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::resetVars, BoostTA));
  dp.property(
      "guard",
      boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::guard, BoostTA));
  dp.property("n_update",
              boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::numberUpdate,
                         BoostTA));
  dp.property("s_update",
              boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::stringUpdate,
                         BoostTA));
  dp.property(
      "n_constraints",
      boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::numConstraints,
                 BoostTA));
  dp.property(
      "s_constraints",
      boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::stringConstraints,
                 BoostTA));

  boost::read_graphviz(file, BoostTA, dp, "id");
}

static inline void parseBoostTA(std::istream &file, BoostPTA &BoostTA) {
  using namespace Symbolic;
  boost::dynamic_properties dp(boost::ignore_other_properties);

  boost::ref_property_map<BoostPTA *, std::size_t> gcvar_size(get_property(BoostTA, boost::graph_clock_variable_size));
  dp.property("clock_variable_size", gcvar_size);
  boost::ref_property_map<BoostPTA *, std::size_t> gsvar_size(get_property(BoostTA, boost::graph_string_variable_size));
  dp.property("string_variable_size", gsvar_size);
  boost::ref_property_map<BoostPTA *, std::size_t> gnvar_size(get_property(BoostTA, boost::graph_number_variable_size));
  dp.property("number_variable_size", gnvar_size);
  boost::ref_property_map<BoostPTA *, std::size_t> gparam_size(get_property(BoostTA, boost::graph_parameter_size));
  dp.property("parameter_size", gparam_size);

  dp.property("match", boost::get(&BoostTAState::isMatch, BoostTA));
  dp.property("init", boost::get(&BoostTAState::isInit, BoostTA));
  dp.property(
      "label",
      boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper, Update>::c,
                 BoostTA));
  dp.property(
      "reset",
      boost::get(
          &BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper, Update>::resetVars,
          BoostTA));
  dp.property(
      "guard",
      boost::get(
          &BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper, Update>::guard,
          BoostTA));
  dp.property("n_update", boost::get(&BoostTATransition<StringConstraint, NumberConstraint,
                                                        ParametricTimingConstraintHelper, Update>::numberUpdate,
                                     BoostTA));
  dp.property("s_update", boost::get(&BoostTATransition<StringConstraint, NumberConstraint,
                                                        ParametricTimingConstraintHelper, Update>::stringUpdate,
                                     BoostTA));
  dp.property("n_constraints", boost::get(&BoostTATransition<StringConstraint, NumberConstraint,
                                                             ParametricTimingConstraintHelper, Update>::numConstraints,
                                          BoostTA));
  dp.property("s_constraints",
              boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                            Update>::stringConstraints,
                         BoostTA));

  boost::read_graphviz(file, BoostTA, dp, "id");
}

template <typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
static inline void
convBoostTA(const BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &BoostTA,
            TimedAutomaton<StringConstraint, NumberConstraint, std::vector<TimingConstraint>, Update> &TA) {
  TA.clockVariableSize = boost::get_property(BoostTA, boost::graph_clock_variable_size);
  TA.stringVariableSize = boost::get_property(BoostTA, boost::graph_string_variable_size);
  TA.numberVariableSize = boost::get_property(BoostTA, boost::graph_number_variable_size);
  TA.states.clear();
  TA.initialStates.clear();
  auto vertex_range = boost::vertices(BoostTA);
  std::unordered_map<
      typename BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update>::vertex_descriptor,
      std::shared_ptr<AutomatonState<StringConstraint, NumberConstraint, std::vector<TimingConstraint>, Update>>>
      stateConvMap;
  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    typename BoostTimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update>::vertex_descriptor v =
        *first;
    stateConvMap[v] =
        std::make_shared<AutomatonState<StringConstraint, NumberConstraint, std::vector<TimingConstraint>, Update>>(
            boost::get(&BoostTAState::isMatch, BoostTA, v));
    TA.states.emplace_back(stateConvMap[v]);
    if (boost::get(&BoostTAState::isInit, BoostTA, v)) {
      TA.initialStates.emplace_back(stateConvMap[v]);
    }
  }

  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    auto edge_range = boost::out_edges(*first, BoostTA);
    for (auto firstEdge = edge_range.first, lastEdge = edge_range.second; firstEdge != lastEdge; ++firstEdge) {
      AutomatonTransition<StringConstraint, NumberConstraint, std::vector<TimingConstraint>, Update> transition;
      transition.target = stateConvMap[boost::target(*firstEdge, BoostTA)]; //.get();
      transition.guard =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::guard, BoostTA,
                     *firstEdge)
              .guard;
      transition.resetVars =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::resetVars,
                     BoostTA, *firstEdge)
              .resetVars;
      UpdateTraits<Update>::stringUpdate(transition.update) =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::stringUpdate,
                     BoostTA, *firstEdge)
              .values;
      transition.stringConstraints = boost::get(
          &BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::stringConstraints, BoostTA,
          *firstEdge)
                                        .values;
      UpdateTraits<Update>::numberUpdate(transition.update) =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::numberUpdate,
                     BoostTA, *firstEdge)
              .values;
      transition.numConstraints =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::numConstraints,
                     BoostTA, *firstEdge)
              .values;
      stateConvMap[*first]
          ->next[boost::get(&BoostTATransition<StringConstraint, NumberConstraint, TimingConstraint, Update>::c,
                            BoostTA, *firstEdge)]
          .emplace_back(std::move(transition));
    }
  }
}

static inline void convBoostTA(const BoostPTA &BoostTA, ParametricTA &TA) {
  using namespace Symbolic;
  TA.clockVariableSize = boost::get_property(BoostTA, boost::graph_clock_variable_size);
  TA.stringVariableSize = boost::get_property(BoostTA, boost::graph_string_variable_size);
  TA.numberVariableSize = boost::get_property(BoostTA, boost::graph_number_variable_size);
  TA.parameterSize = boost::get_property(BoostTA, boost::graph_parameter_size);

  TA.states.clear();
  TA.initialStates.clear();
  auto vertex_range = boost::vertices(BoostTA);
  std::unordered_map<
      typename BoostTimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                   Update>::vertex_descriptor,
      std::shared_ptr<AutomatonState<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update>>>
      stateConvMap;
  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    typename BoostTimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                 Update>::vertex_descriptor v = *first;
    stateConvMap[v] = std::make_shared<PTAState>(boost::get(&BoostTAState::isMatch, BoostTA, v));
    TA.states.emplace_back(stateConvMap[v]);
    if (boost::get(&BoostTAState::isInit, BoostTA, v)) {
      TA.initialStates.emplace_back(stateConvMap[v]);
    }
  }

  for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
    auto edge_range = boost::out_edges(*first, BoostTA);
    for (auto firstEdge = edge_range.first, lastEdge = edge_range.second; firstEdge != lastEdge; ++firstEdge) {
      AutomatonTransition<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> transition;
      transition.target = stateConvMap[boost::target(*firstEdge, BoostTA)]; //.get();
      transition.guard = Parma_Polyhedra_Library::NNC_Polyhedron(TA.parameterSize + TA.clockVariableSize);
      const auto boostGuard =
          boost::get(
              &BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper, Update>::guard,
              BoostTA, *firstEdge)
              .guard;
      for (const auto &helper: boostGuard) {
        Parma_Polyhedra_Library::Constraint constraint;
        helper.extract(TA.parameterSize, constraint);
        transition.guard.add_constraint(constraint);
      }
      transition.resetVars = boost::get(&BoostTATransition<StringConstraint, NumberConstraint,
                                                           ParametricTimingConstraintHelper, Update>::resetVars,
                                        BoostTA, *firstEdge)
                                 .resetVars;
      UpdateTraits<Update>::stringUpdate(transition.update) =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                        Update>::stringUpdate,
                     BoostTA, *firstEdge)
              .values;
      transition.stringConstraints =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                        Update>::stringConstraints,
                     BoostTA, *firstEdge)
              .values;
      UpdateTraits<Update>::numberUpdate(transition.update) =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                        Update>::numberUpdate,
                     BoostTA, *firstEdge)
              .values;
      transition.numConstraints =
          boost::get(&BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper,
                                        Update>::numConstraints,
                     BoostTA, *firstEdge)
              .values;
      stateConvMap[*first]
          ->next[boost::get(
              &BoostTATransition<StringConstraint, NumberConstraint, ParametricTimingConstraintHelper, Update>::c,
              BoostTA, *firstEdge)]
          .emplace_back(std::move(transition));
    }
  }
}
