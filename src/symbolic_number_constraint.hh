#pragma once

#ifndef mem_fun_ref
#define mem_fun_ref mem_fn
#endif

#include <functional>
#include <ppl.hh>

namespace Symbolic {
  /*!
     @brief valuation of number variables
   */
  using NumberValuation = Parma_Polyhedra_Library::NNC_Polyhedron;
  using NumberExpression = Parma_Polyhedra_Library::Linear_Expression;
  using NumberConstraint = Parma_Polyhedra_Library::Constraint;
  using Parma_Polyhedra_Library::IO_Operators::operator<<;
}

