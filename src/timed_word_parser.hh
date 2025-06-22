#pragma once
#include <istream>
#include <string>
#include <vector>
#include "signature.hh"
#include <iostream>

template<typename Number, typename TimeStamp = double>
struct TimedWordEvent {
  std::size_t actionId;
  std::vector<std::string> strings;
  std::vector<Number> numbers;
  TimeStamp timestamp;
};

/*!
  @brief Parser of a timed word
 */
template<typename Number, typename TimeStamp = double>
class TimedWordParser {
public:
  TimedWordParser(std::istream &is, const Signature &sig) : is(is), sig(sig) {}
  /*!
    @brief Parse and return an event with data
    @retval true If the parse succeeded
    @retval false If the parse failed
   */
  bool parse(TimedWordEvent<Number, TimeStamp> &event) {
    while (true) {
      if (is.eof()) {
        return false;
      }
      std::string action;
      is >> action;
      if (action.empty()) {
        return false;
      }
      if (!sig.isDefined(action)) {
        std::string skipped;
        std::getline(is, skipped);
        std::cerr << "Undefined action: " << action << std::endl;
        continue;
      }
      const std::size_t stringSize = sig.getStringSize(action);
      const std::size_t numberSize = sig.getNumberSize(action);
      event.actionId = sig.getId(action);
      event.strings.resize(stringSize);
      for (std::size_t i = 0; i < stringSize; i++) {
        std::string str;
        is >> str;
        event.strings[i] = std::move(str);
      }
      event.numbers.resize(numberSize);
      for (std::size_t i = 0; i < numberSize; i++) {
        Number num;
        is >> num;
        event.numbers[i] = std::move(num);
      }
      is >> event.timestamp;
      return true;
    }
  }
private:
  std::istream &is;
  const Signature &sig;  
};
