#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "common_types.hh"

//! @brief The return values of comparison of two values. Similar to strcmp.
enum class Order {
  LT, EQ, GT
};

inline bool toBool(Order odr) {
  return odr == Order::EQ;
}

//! @brief A constraint in a guard of transitions
struct TimingConstraint {
  enum class Order {
    lt, le, ge, gt
  };

  ClockVariables x;
  Order odr;
  int c;

  bool satisfy(double d) const {
    switch (odr) {
      case Order::lt:
        return d < c;
      case Order::le:
        return d <= c;
      case Order::gt:
        return d > c;
      case Order::ge:
        return d >= c;
    }
    return false;
  }

  using Interpretation = std::vector<double>;

  ::Order operator()(Interpretation val) const {
    if (satisfy(val.at(x))) {
      return ::Order::EQ;
    } else if (odr == Order::lt || odr == Order::le) {
      return ::Order::GT;
    } else {
      return ::Order::LT;
    }
  }
};

// An interface to write an inequality constrait easily
class ConstraintMaker {
  ClockVariables x;
public:
  ConstraintMaker(ClockVariables x) : x(x) {}

  TimingConstraint operator<(int c) {
    return TimingConstraint{x, TimingConstraint::Order::lt, c};
  }

  TimingConstraint operator<=(int c) {
    return TimingConstraint{x, TimingConstraint::Order::le, c};
  }

  TimingConstraint operator>(int c) {
    return TimingConstraint{x, TimingConstraint::Order::gt, c};
  }

  TimingConstraint operator>=(int c) {
    return TimingConstraint{x, TimingConstraint::Order::ge, c};
  }
};

/*!
  @brief remove any inequality x > c or x >= c
 */
static inline void widen(std::vector<TimingConstraint> &guard) {
  guard.erase(std::remove_if(guard.begin(), guard.end(), [](TimingConstraint g) {
    return g.odr == TimingConstraint::Order::ge || g.odr == TimingConstraint::Order::gt;
  }), guard.end());
}

using TimingValuation = std::vector<double>;

static inline
bool eval(const TimingValuation &clockValuation,
          const std::vector<TimingConstraint> &guard) {
  return std::all_of(guard.begin(), guard.end(),
                     [&clockValuation](const TimingConstraint &g) {
                       return g.satisfy(clockValuation.at(g.x));
                     });
}
