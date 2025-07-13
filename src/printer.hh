#include <iomanip>

#include "boolean_monitor.hh"

struct BooleanPrinter : public Observer<BooleanMonitorResult> {
  BooleanPrinter() = default;

  virtual ~BooleanPrinter() = default;

  void notify(const BooleanMonitorResult &result) override {
    printf("@%f. (time-point %lu)\n", result.timestamp, result.index);
  }
};

#include "data_parametric_monitor.hh"

struct DataParametricPrinter : public Observer<DataParametricMonitorResult> {
  DataParametricPrinter() = default;

  virtual ~DataParametricPrinter() = default;

  void notify(const DataParametricMonitorResult &result) override {
    using Parma_Polyhedra_Library::IO_Operators::operator<<;
    std::cout << "@" << std::fixed << result.timestamp << ".\t(time-point " << result.index << ")\t";
    for (std::size_t i = 0; i < result.stringValuation.size(); i++) {
      if (result.stringValuation[i].index() == 0) {
        std::cout << "x" << i << " != {";
        for (const auto &r: std::get<0>(result.stringValuation[i])) {
          std::cout << r << ", ";
        }
        std::cout << "}\t";
      } else {
        std::cout << "x" << i << " == " << std::get<1>(result.stringValuation[i]) << "\t";
      }
    }

    std::cout << result.numberValuation
              << "\n";
  }
};

#include "parametric_monitor.hh"

struct ParametricPrinter : public Observer<ParametricMonitorResult> {
  ParametricPrinter() = default;

  virtual ~ParametricPrinter() = default;

  void notify(const ParametricMonitorResult &result) override {
    using Parma_Polyhedra_Library::IO_Operators::operator<<;
    std::cout << "@" << result.timestamp << ".\t(time-point " << result.index << ")\t";
    for (std::size_t i = 0; i < result.stringValuation.size(); i++) {
      if (result.stringValuation[i].index() == 0) {
        std::cout << "x" << i << " != {";
        for (const auto &r: std::get<0>(result.stringValuation[i])) {
          std::cout << r << ", ";
        }
        std::cout << "}\t";
      } else {
        std::cout << "x" << i << " == " << std::get<1>(result.stringValuation[i]) << "\t";
      }
    }

    std::cout << "Num: " << result.numberValuation
              << "\tClock: " << result.parametricTimingValuation << "\n";
  }
};

