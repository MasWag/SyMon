#pragma once

#include <memory>
#include <array>
#include <vector>
#include <variant>
#include <algorithm>
#include <optional>
#include <cassert>

#include "common_types.hh"

namespace NonSymbolic {
  /*!
    @brief valuation of number variables
    @note We do nothing symbolic.
  */
  template<typename Number>
  using NumberValuation = std::vector<std::optional<Number>>;

  /*!
    @note When we need an optimization, we can make it a vector of children not a tree.
   */
  struct NumberExpression {
    enum class kind_t {
      ATOM, PLUS, MINUS
    } kind;

    NumberExpression(VariableID id = 0) : kind(kind_t::ATOM), child(id) {}

    NumberExpression(kind_t kind,
                     std::shared_ptr<NumberExpression> first,
                     std::shared_ptr<NumberExpression> second) : kind(kind) {
      assert(kind != kind_t::ATOM);
      child = std::array<std::shared_ptr<NumberExpression>, 2>();
      std::get<1>(child)[0] = std::move(first);
      std::get<1>(child)[1] = std::move(second);
    }

    std::variant<VariableID, std::array<std::shared_ptr<NumberExpression>, 2>> child;

    template<typename Number>
    void eval(const NumberValuation<Number> &env, std::optional<Number> &result) const {
      switch (kind) {
        case kind_t::ATOM:
          assert(child.index() == 0);
          result = env[std::get<VariableID>(child)];
          return;
        case kind_t::PLUS: {
          assert(child.index() == 1);
          std::array<std::optional<Number>, 2> childrenResults;
          for (int i = 0; i < 2; i++) {
            std::get<1>(child)[i]->eval(env, childrenResults[i]);
          }
          result = *childrenResults[0] + *childrenResults[1];
          return;
        }
        case kind_t::MINUS: {
          assert(child.index() == 1);
          std::array<std::optional<Number>, 2> childrenResults;
          for (int i = 0; i < 2; i++) {
            std::get<1>(child)[i]->eval(env, childrenResults[i]);
          }
          result = *childrenResults[0] - *childrenResults[1];
          return;
        }
      }
    }
  };

  template<typename Number>
  struct NumberConstraint {
    NumberExpression expr;
    enum class kind_t {
      GT, GE, EQ, NE, LE, LT
    } kind;
    Number num;

    bool eval(const NumberValuation<Number> &env) const {
      std::optional<Number> result;
      expr.eval(env, result);
      switch (kind) {
        case kind_t::GT:
          return *result > num;
        case kind_t::GE:
          return *result >= num;
        case kind_t::EQ:
          return *result == num;
        case kind_t::NE:
          return *result != num;
        case kind_t::LE:
          return *result <= num;
        case kind_t::LT:
          return *result < num;
      }
      return false;
    }
  };

  //! @todo Write other operators e.g., 
  template<typename Number>
  class NCMakerVar {
  public:
    NCMakerVar(VariableID id) : id(id) {
    }

    NonSymbolic::NumberConstraint<Number> operator==(Number num) {
      NumberExpression expr{NumberExpression::kind_t::ATOM, id};
      return {expr, NonSymbolic::NumberConstraint<Number>::kind_t::EQ, num};
    }

    NonSymbolic::NumberConstraint<Number> operator==(NCMakerVar maker) {
      auto first = std::make_shared<NumberExpression>(id);
      auto second = std::make_shared<NumberExpression>(maker.id);
      return {{NumberExpression::kind_t::MINUS, std::move(first), std::move(second)},
              NonSymbolic::NumberConstraint<Number>::kind_t::EQ, 0};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(Number num) {
      NumberExpression expr{NumberExpression::kind_t::ATOM, id};
      return {expr, NonSymbolic::NumberConstraint<Number>::kind_t::NE, num};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(NCMakerVar maker) {
      auto first = std::make_shared<NumberExpression>(id);
      auto second = std::make_shared<NumberExpression>(maker.id);
      return {{NumberExpression::kind_t::MINUS, std::move(first), std::move(second)},
              NonSymbolic::NumberConstraint<Number>::kind_t::NE, 0};
    }

  private:
    const VariableID id;
  };
}
