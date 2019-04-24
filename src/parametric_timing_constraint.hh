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

static inline
bool eval(ParametricTimingValuation &cval, const ParametricTimingConstraint &guard) {
  cval.intersection_assign(guard);
  return !cval.is_empty();
}

#endif //DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HH
