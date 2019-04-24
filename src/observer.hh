#pragma once

/*!
  @brief Abstract Class for observer pattrn
  @sa Subject
 */
template<typename T>
class Observer {
public:
  virtual void notify(const T&) = 0;
};
