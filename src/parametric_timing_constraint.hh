/*
 * @author Masaki Waga
 * @date 2019/01/26
*/

#ifndef DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH
#define DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH

#define mem_fun_ref mem_fn

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

#endif //DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH
