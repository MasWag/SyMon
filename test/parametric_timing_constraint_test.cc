#include <boost/test/unit_test.hpp>
#include <sstream>

#include "../src/parametric_timing_constraint.hh"

BOOST_AUTO_TEST_SUITE(ParametricTimingConstraintTest)

    BOOST_AUTO_TEST_CASE(Shift) {
        using namespace Parma_Polyhedra_Library;
        using namespace Parma_Polyhedra_Library::IO_Operators;
        std::stringstream ss;
        Constraint_System expr;
        expr.insert(Constraint{Linear_Expression{Variable(0)} < 3});
        expr.insert(Constraint{Linear_Expression{Variable(1)} + Variable(2) > 5});
        ParametricTimingConstraint constraint(expr);
        auto shifted = shift(constraint, 2);
        Constraint_System expected;
        expected.insert(Constraint{Linear_Expression{Variable(2)} < 3});
        expected.insert(Constraint{Linear_Expression{Variable(3)} + Variable(4) > 5});
        ss << shifted;
        std::string shiftedStr = ss.str();
        ss.str(""); // Clear the stringstream
        ss << expected;
        std::string expectedStr = ss.str();

    BOOST_CHECK_EQUAL(expectedStr, shiftedStr);
    }

BOOST_AUTO_TEST_SUITE_END() // ParametricTimingConstraintTest
