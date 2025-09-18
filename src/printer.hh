#include <iomanip>

#include "boolean_monitor.hh"

template <class Number> struct BooleanPrinter : public Observer<BooleanMonitorResult<Number>> {
  BooleanPrinter() = default;

  virtual ~BooleanPrinter() = default;

  void notify(const BooleanMonitorResult<Number> &result) override {
    printf("@%f.\t(time-point %lu)\t", result.timestamp, result.index);
    for (std::size_t i = 0; i < result.stringValuation.size(); i++) {
      if (result.stringValuation[i]) {
        printf("x%zu == %s\t", i, result.stringValuation[i]->c_str());
      }
    }

    for (std::size_t i = 0; i < result.numberValuation.size(); i++) {
      std::cout << "x" << i << " == " << *(result.numberValuation[i]) << "\t";
    }
    printf("\n");
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

    std::cout << result.numberValuation << "\n";
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

    std::cout << "Num: " << result.numberValuation << "\tClock: " << result.parametricTimingValuation << "\n";
  }
};
