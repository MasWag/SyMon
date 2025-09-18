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
  Signature() = default;
  explicit Signature(std::istream &is) {
    std::string key;
    std::size_t stringSize, numberSize;
    std::size_t id = 0;
    while (is.good()) {
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
  [[nodiscard]] std::size_t getStringSize(const std::string &key) const {
    return stringSizeMap.at(key);
  }
  [[nodiscard]] std::size_t getNumberSize(const std::string &key) const {
    return numberSizeMap.at(key);
  }
  [[nodiscard]] std::size_t getId(const std::string &key) const {
    return idMap.at(key);
  }
  [[nodiscard]] bool isDefined(const std::string &key) const {
    return idMap.find(key) != idMap.end();
  }
  [[nodiscard]] std::size_t size() const {
    return this->idMap.size();
  }
  [[nodiscard]] std::vector<std::string> getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(idMap.size());
    for (const auto &[fst, snd]: idMap) {
      keys.push_back(fst);
    }
    return keys;
  }

  Signature(std::unordered_map<std::string, std::size_t> idMap,
            std::unordered_map<std::string, std::size_t> stringSizeMap,
            std::unordered_map<std::string, std::size_t> numberSizeMap)
      : idMap(std::move(idMap)), stringSizeMap(std::move(stringSizeMap)), numberSizeMap(std::move(numberSizeMap)) {
    if (idMap.size() != stringSizeMap.size() || idMap.size() != numberSizeMap.size()) {
      throw std::runtime_error("Signature maps must have the same size");
    }
  }

private:
  std::unordered_map<std::string, std::size_t> idMap;
  std::unordered_map<std::string, std::size_t> stringSizeMap;
  std::unordered_map<std::string, std::size_t> numberSizeMap;
};
