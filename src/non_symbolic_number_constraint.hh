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

  enum class NumberExpressionKind { ATOM, PLUS, MINUS, CONSTANT };
  
  /*!
    @note When we need an optimization, we can make it a vector of children not a tree.
   */
  template <typename Number>
  struct NumberExpression {
    NumberExpressionKind kind;

    NumberExpression(VariableID id = 0) : kind(NumberExpressionKind::ATOM), child(id) {
    }

    /*
    NumberExpression(Number value = 0) : kind(NumberExpressionKind::CONSTANT), child(value) {
    }
    */
    static NumberExpression<Number> constant(Number value) {
      NumberExpression<Number> expr;
      expr.kind = NumberExpressionKind::CONSTANT;
      expr.child = value;
      return expr;
    }

    NumberExpression(NumberExpressionKind kind, std::shared_ptr<NumberExpression> first, std::shared_ptr<NumberExpression> second)
        : kind(kind) {
      assert(kind != NumberExpressionKind::ATOM);
      child = std::array<std::shared_ptr<NumberExpression>, 2>();
      std::get<1>(child)[0] = std::move(first);
      std::get<1>(child)[1] = std::move(second);
    }

    std::variant<VariableID, std::array<std::shared_ptr<NumberExpression>, 2>, Number> child;

    void eval(const NumberValuation<Number> &env, std::optional<Number> &result) const {
      switch (kind) {
        case NumberExpressionKind::ATOM:
          assert(child.index() == 0);
          result = env[std::get<VariableID>(child)];
          return;
        case NumberExpressionKind::CONSTANT:
          assert(child.index() == 2);
          result = std::get<Number>(child);
          return;
        case NumberExpressionKind::PLUS: {
          assert(child.index() == 1);
          std::array<std::optional<Number>, 2> childrenResults;
          for (int i = 0; i < 2; i++) {
            std::get<1>(child)[i]->eval(env, childrenResults[i]);
          }
          result = *childrenResults[0] + *childrenResults[1];
          return;
        }
        case NumberExpressionKind::MINUS: {
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

  enum class NumberComparatorKind { GT, GE, EQ, NE, LE, LT };
  template <typename Number> struct NumberConstraint {
    NumberComparatorKind kind;
    NumberExpression<Number> left;
    NumberExpression<Number> right;

    bool eval(const NumberValuation<Number> &env) const {
      std::optional<Number> leftResult;
      left.eval(env, leftResult);
      std::optional<Number> rightResult;
      right.eval(env, rightResult);
      switch (kind) {
        case NumberComparatorKind::GT:
          return *leftResult > *rightResult;
        case NumberComparatorKind::GE:
          return *leftResult >= *rightResult;
        case NumberComparatorKind::EQ:
          return *leftResult == *rightResult;
        case NumberComparatorKind::NE:
          return *leftResult != *rightResult;
        case NumberComparatorKind::LE:
          return *leftResult <= *rightResult;
        case NumberComparatorKind::LT:
          return *leftResult < *rightResult;
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
      return {NonSymbolic::NumberComparatorKind::EQ, left, right};
    }

    NonSymbolic::NumberConstraint<Number> operator==(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberComparatorKind::EQ, first, second};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(Number num) {
      NumberExpression<Number> left{id};
      NumberExpression<Number> right = NumberExpression<Number>::constant(num);
      return {NonSymbolic::NumberComparatorKind::NE, left, right};
    }

    NonSymbolic::NumberConstraint<Number> operator!=(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberComparatorKind::NE, first, second};
    }

    NonSymbolic::NumberConstraint<Number> operator>(Number num) {
      NumberExpression<Number> expr{id};
      return {NonSymbolic::NumberComparatorKind::GT, expr, num};
    }

    NonSymbolic::NumberConstraint<Number> operator>(NCMakerVar maker) {
      auto first = NumberExpression<Number>(id);
      auto second = NumberExpression<Number>(maker.id);
      return {NonSymbolic::NumberComparatorKind::GT, first, second};
    }

  private:
    const VariableID id;
  };
} // namespace NonSymbolic
