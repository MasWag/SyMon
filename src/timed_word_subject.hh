#pragma once

#include "subject.hh"
#include "timed_word_parser.hh"
#include <memory>

template <typename Number, typename TimeStamp = double>
class TimedWordSubject : public SingleSubject<TimedWordEvent<Number, TimeStamp>> {
public:
  TimedWordSubject(std::unique_ptr<TimedWordParser<Number, TimeStamp>> parser) : parser(std::move(parser)) {
  }
  void parseAndSubjectAll() const {
    TimedWordEvent<Number, TimeStamp> event;
    while (parser->parse(event)) {
      this->notifyObservers(event);
    }
  }

private:
  std::unique_ptr<TimedWordParser<Number, TimeStamp>> parser;
};
