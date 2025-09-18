#pragma once

#include "observer.hh"
#include <algorithm>
#include <memory>
#include <vector>

/*!
  @brief Abstract Class for observer pattrn
  @sa Observer
 */
template <typename T> class AbstractSubject {
public:
  virtual void addObserver(std::shared_ptr<Observer<T>>) = 0;
  virtual void deleteObserver(std::shared_ptr<Observer<T>>) = 0;

protected:
  virtual void notifyObservers(const T &) const = 0;
};

/*!
  @brief Abstract Class of subject, where the number of object is at most one
  @sa Observer
 */
template <typename T> class SingleSubject : public AbstractSubject<T> {
public:
  void addObserver(std::shared_ptr<Observer<T>> observer) {
    this->observer = observer;
  }
  void deleteObserver(std::shared_ptr<Observer<T>> observer) {
    if (observer && this->observer == observer) {
      observer.reset();
    }
  }

protected:
  void notifyObservers(const T &data) const {
    if (observer) {
      observer->notify(data);
    }
  }
  std::shared_ptr<Observer<T>> observer;
};

/*!
  @brief Abstract Class of subject, where the number of object may be more than one.
  @sa Observer
 */
template <typename T> class Subject {
public:
  void addObserver(std::shared_ptr<Observer<T>> ptr) {
    this->ptrs.push_back(std::move(ptr));
  }
  void deleteObserver(std::shared_ptr<Observer<T>> ptr) {
    auto it = std::find(ptrs.begin(), ptrs.end(), ptr);
    if (it != ptrs.end()) {
      ptrs.remove(it);
    }
  }

protected:
  void notifyObservers(const T &data) const {
    for (const auto observer: ptrs) {
      if (observer) {
        observer->notify(data);
      }
    }
  };
  std::vector<std::shared_ptr<Observer<T>>> ptrs;
};
