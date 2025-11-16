#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "common_types.hh"

namespace NonSymbolic {
  /*!
    @brief valuation of number variables
    @note We do nothing symbolic.
  */
  template <typename Number> using NumberValuation = std::vector<std::optional<Number>>;

  /*!
    @note When we need an optimization, we can make it a vector of children not a tree.
   */
  template <typename Number>
  struct NumberExpression {
    enum class kind_t { ATOM, PLUS, MINUS, CONSTANT } kind;

    NumberExpression(VariableID id = 0) : kind(kind_t::ATOM), child(id) {
    }

    /*
    NumberExpression(Number value = 0) : kind(kind_t::CONSTANT), child(value) {
    }
    */
    static NumberExpression<Number> constant(Number value) {
      NumberExpression<Number> expr;
      expr.kind = kind_t::CONSTANT;
      expr.child = value;
      return expr;
    }

    NumberExpression(kind_t kind, std::shared_ptr<NumberExpression> first, std::shared_ptr<NumberExpression> second)
        : kind(kind) {
      assert(kind != kind_t::ATOM);
      child = std::array<std::shared_ptr<NumberExpression>, 2>();
      std::get<1>(child)[0] = std::move(first);
      std::get<1>(child)[1] = std::move(second);
    }

    std::variant<VariableID, std::array<std::shared_ptr<NumberExpression>, 2>, Number> child;

    void eval(const NumberValuation<Number> &env, std::optional<Number> &result) const {
      switch (kind) {
        case kind_t::ATOM:
          assert(child.index() == 0);
          result = env[std::get<VariableID>(child)];
          return;
        case kind_t::CONSTANT:
          assert(child.index() == 2);
          result = std::get<Number>(child);
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

  template <typename Number> struct NumberConstraint {
    enum class kind_t { GT, GE, EQ, NE, LE, LT } kind;
    NumberExpression<Number> left;
    NumberExpression<Number> right;

    bool eval(const NumberValuation<Number> &env) const {
      std::optional<Number> leftResult;
      left.eval(env, leftResult);
      std::optional<Number> rightResult;
      right.eval(env, rightResult);
      switch (kind) {
        case kind_t::GT:
          return *leftResult > rightResult;
        case kind_t::GE:
          return *leftResult >= rightResult;
        case kind_t::EQ:
          return *leftResult == rightResult;
        case kind_t::NE:
          return *leftResult != rightResult;
        case kind_t::LE:
          return *leftResult <= rightResult;
        case kind_t::LT:
          return *leftResult < rightResult;
      }
      return false;
    }
  };

  // テストで NumberConstraint を簡単に作るためのヘルパークラスっぽい？
  //! @todo Write other operators e.g.,
  template <typename Number> class NCMakerVar {
  public:
    NCMakerVar(VariableID id) : id(id) {
    }

    NonSymbolic::NumberConstraint<Number> operator==(Number num) {
      NumberExpression<Number> left{id};
      NumberExpression<Number> right = NumberExpression<Number>::constant(num);
      return {NonSymbolic::NumberConstraint<Number>::kind_t::EQ, left, right};
    }

    NonSymbolic::NumberConstraint<Number> operator==(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberConstraint<Number>::kind_t::EQ, first, second};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(Number num) {
      NumberExpression<Number> left{id};
      NumberExpression<Number> right = NumberExpression<Number>::constant(num);
      return {NonSymbolic::NumberConstraint<Number>::kind_t::NE, left, right};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberConstraint<Number>::kind_t::NE, first, second};
    }

    NonSymbolic::NumberConstraint<Number> operator>(Number num) {
      NumberExpression<Number> expr{id};
      return {NonSymbolic::NumberConstraint<Number>::kind_t::GT, expr, num};
    }

    NonSymbolic::NumberConstraint<Number> operator>(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberConstraint<Number>::kind_t::GT, first, second};
    }

  private:
    const VariableID id;
  };
} // namespace NonSymbolic
