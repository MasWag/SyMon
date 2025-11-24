#include <boost/test/unit_test.hpp>

#include "../src/timing_constraint.hh"
#include "../src/automaton_parser.hh"

BOOST_AUTO_TEST_SUITE(TimingConstraintTest)
    BOOST_AUTO_TEST_CASE(Shift) {
        constexpr TimingConstraint<double> constraint{0, TimingConstraint<double>::Order::lt, 10};
        auto [x, odr, c] = constraint.shift(1);
        BOOST_CHECK_EQUAL(x, 1);
        BOOST_CHECK_EQUAL(odr, TimingConstraint<double>::Order::lt);
        BOOST_CHECK_EQUAL(c, 10);
    }

    BOOST_AUTO_TEST_CASE(Satisfy) {
        constexpr TimingConstraint<double> constraint{0, TimingConstraint<double>::Order::lt, 10};
        BOOST_CHECK(constraint.satisfy(5));
        BOOST_CHECK(!constraint.satisfy(10));
        BOOST_CHECK(!constraint.satisfy(15));
    }

    BOOST_AUTO_TEST_CASE(Decimal) {
        constexpr TimingConstraint constraint{1, TimingConstraint::Order::ge, 0.5};
        BOOST_CHECK(!constraint.satisfy(0.4));
        BOOST_CHECK(constraint.satisfy(0.5));
        BOOST_CHECK(constraint.satisfy(0.6));
    }

BOOST_AUTO_TEST_SUITE_END() // TimingConstraintTest
