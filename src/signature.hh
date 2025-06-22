#pragma once

#include <fstream>
#include <istream>
#include <string>
#include <unordered_map>

/*!
  @brief Signature of input data
 */
class Signature {
public:
  Signature(std::istream& is) {
    std::string key;
    std::size_t stringSize, numberSize;
    std::size_t id = 0;
    while(is.good()) {
      is >> key >> stringSize;
      if (!is.good()) {
        break;
      }
      is >> numberSize;
      if (is.fail()) {
        break;
      }
      stringSizeMap[key] = stringSize;
      numberSizeMap[key] = numberSize;
      idMap[key] = id++;
    }
  }
  std::size_t getStringSize(const std::string &key) const {
    return stringSizeMap.at(key);
  }
  std::size_t getNumberSize(const std::string &key) const {
    return numberSizeMap.at(key);
  }
  std::size_t getId(const std::string &key) const {
    return idMap.at(key);
  }
  bool isDefined(const std::string &key) const {
    return idMap.find(key) != idMap.end();
  }
private:
  std::unordered_map<std::string, std::size_t> idMap;
  std::unordered_map<std::string, std::size_t> stringSizeMap;
  std::unordered_map<std::string, std::size_t> numberSizeMap;
};
