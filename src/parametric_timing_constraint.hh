/*
 * @author Masaki Waga
 * @date 2019/01/26
*/

#ifndef DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH
#define DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH

#ifndef mem_fun_ref
#define mem_fun_ref mem_fn
#endif

#include <functional>
#include <ppl.hh>

using ParametricTimingValuation = Parma_Polyhedra_Library::NNC_Polyhedron;
using ParametricTimingConstraint = Parma_Polyhedra_Library::NNC_Polyhedron;

static bool eval(ParametricTimingValuation &cval, const ParametricTimingConstraint &guard) {
    cval.intersection_assign(guard);
    return !cval.is_empty();
}

/**
 * @brief Shift the clock variable id of the guard by a given width.
 *
 * @param guard the guard to shift
 * @param width the width to shift the clock variable id
 * @return a new ParametricTimingConstraint with the clock variable shifted
 */
static ParametricTimingConstraint shift(const ParametricTimingConstraint &guard, const std::size_t width) {
    auto result = Parma_Polyhedra_Library::NNC_Polyhedron(width, Parma_Polyhedra_Library::UNIVERSE);
    result.concatenate_assign(guard);
    return result;
}

/*!
 * @brief Compute the intersection of two ParametricTimingConstraints.
 *
 * @param left the first ParametricTimingConstraint
 * @param right the second ParametricTimingConstraint
 * @return a new ParametricTimingConstraint that is the intersection of the two
 */
static ParametricTimingConstraint operator&&(const ParametricTimingConstraint &left,
                                             const ParametricTimingConstraint &right) {
    auto result = left;
    if (result.space_dimension() < right.space_dimension()) {
        result.add_space_dimensions_and_embed(right.space_dimension() - result.space_dimension());
    } else if (result.space_dimension() > right.space_dimension()) {
        auto shiftedRight = right;
        shiftedRight.add_space_dimensions_and_embed(result.space_dimension() - right.space_dimension());
        result.concatenate_assign(shiftedRight);
        return result;
    }
    result.intersection_assign(right);
    return result;
}

/*!
 * @brief Modify the guard to the given size.
 *
 * @param guard the vector of TimingConstraint to adjust.
 * @param size the size to adjust the guard to
 * @return a new vector of TimingConstraint with the clock variables adjusted
 */
static ParametricTimingConstraint adjustDimension(const ParametricTimingConstraint &guard, const std::size_t size) {
    auto result = guard;
    if (result.space_dimension() < size) {
        result.add_space_dimensions_and_embed(size - result.space_dimension());
    } else if (result.space_dimension() > size) {
        result.remove_higher_space_dimensions(result.space_dimension() - size);
    }
    assert(result.space_dimension() == size);
    return result;
}

#endif //DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH
