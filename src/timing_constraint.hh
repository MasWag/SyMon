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

    [[nodiscard]] bool satisfy(double d) const {
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

    /*!
     * @brief Shift the id of the clock variable by a given width.
     *
     * @param width the width to shift the clock variable id
     * @return a new TimingConstraint with the clock variable shifted
     */
    [[nodiscard]] TimingConstraint shift(ClockVariables width) const {
        return TimingConstraint{x + width, odr, c};
    }

    [[nodiscard]] bool operator==(const TimingConstraint &other) const {
        return x == other.x && odr == other.odr && c == other.c;
    }
};

// An interface to write an inequality constrait easily
class ConstraintMaker {
    ClockVariables x;

public:
    explicit ConstraintMaker(ClockVariables x) : x(x) {
    }

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
static void widen(std::vector<TimingConstraint> &guard) {
    guard.erase(std::remove_if(guard.begin(), guard.end(), [](TimingConstraint g) {
        return g.odr == TimingConstraint::Order::ge || g.odr == TimingConstraint::Order::gt;
    }), guard.end());
}

using TimingValuation = std::vector<double>;

static bool eval(const TimingValuation &clockValuation,
                 const std::vector<TimingConstraint> &guard) {
    return std::all_of(guard.begin(), guard.end(),
                       [&clockValuation](const TimingConstraint &g) {
                           return g.satisfy(clockValuation.at(g.x));
                       });
}

/*!
 * @brief Shift the clock variables in the guard by a given width.
 *
 * @param guard the vector of TimingConstraint to shift
 * @param width the width to shift the clock variable id
 * @return a new vector of TimingConstraint with the clock variables shifted
 */
static std::vector<TimingConstraint> shift(const std::vector<TimingConstraint> &guard, const ClockVariables width) {
    std::vector<TimingConstraint> shiftedGuard;
    shiftedGuard.reserve(guard.size());
    for (const auto &g: guard) {
        shiftedGuard.push_back(g.shift(width));
    }

    return shiftedGuard;
}

/*!
 * @brief Combine two vectors of TimingConstraint using logical AND.
 *
 * @param left the first vector of TimingConstraint
 * @param right the second vector of TimingConstraint
 * @return a new vector containing all TimingConstraints from both vectors
 */
static std::vector<TimingConstraint> operator&&(const std::vector<TimingConstraint> &left,
                                                const std::vector<TimingConstraint> &right) {
    std::vector<TimingConstraint> result = left;
    result.reserve(left.size() + right.size());
    std::copy_if(right.begin(), right.end(), std::back_inserter(result), [&left](const auto &guard) {
        return std::find(left.begin(), left.end(), guard) == left.end();
    });
    return result;
}

/*!
 * @brief Modify the guard to the given size.
 *
 * @param guard the vector of TimingConstraint to adjust.
 * @param size the size to adjust the guard to
 * @return a new vector of TimingConstraint with the clock variables adjusted
 */
static std::vector<TimingConstraint> adjustDimension(const std::vector<TimingConstraint> &guard, const ClockVariables size) {
    return guard;
}
