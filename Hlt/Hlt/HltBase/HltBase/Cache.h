#ifndef HltEvent_CACHE_H_
#define HltEvent_CACHE_H_
// =============================================================================
/**
 * @file   Cache.h
 * @author alexander.mazurov@gmail.com
 * @date   Jun 15, 2010
 * @brief  Cache implementation for stage objects
 */
// =============================================================================
#include <boost/variant.hpp>
#include <boost/foreach.hpp>
// =============================================================================
#include <string>
#include <map>
#include <vector>
#include <ostream>
// =============================================================================
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/ToStream.h"
// =============================================================================
namespace Hlt
{
// =============================================================================
using Gaudi::StringKey;
using GaudiUtils::VectorMap;
// =============================================================================
/**
 * @author alexander.mazurov@gmail.com
 * @date   Jun 15, 2010
 * @brief  Cache implementation for stage objects
 * @attention Currently implementation is bases on VectorMap class and uses
 *            StringKey as key. After some tests those classes may be will be
 *            replaced by more time and memory effective competitor.
 * @see Hlt::Stage
 *
 * Cache for stage objects is a string -> value mapping.
 */
// =============================================================================
class Cache
{
// =============================================================================
 public:
// =============================================================================
  typedef VectorMap<StringKey, bool> BooleanMap;
  typedef VectorMap<StringKey, int> IntegerMap;
  typedef VectorMap<StringKey, double> DoubleMap;
  typedef VectorMap<StringKey, std::string> StringMap;
// =============================================================================
  /**
   * @brief Check if cache contains value with the specified key
   * @param[in] key
   */
  bool has(const StringKey& key) const;
  /**
   * @brief Erase all values with the specified key
   * @param[in] key
   */
  void erase(const StringKey& key);
  /**
   * @brief Return a list of existent keys
   */
  const std::vector<const StringKey*> keys() const;

  /**
   * @brief Check if cache contains value with the specified key and type
   */
  template<typename T> bool has_(const StringKey& key) const;
  /**
   * @brief Add value with the specified type and key
   * @return true if value was added and false if we try insert an existing key
   */
  template<typename T> bool insert(const StringKey& key, const T& value);
  /**
   * @brief Update value with the specified type and key. If the value with the same
   * key already exists it will be replaced.
   */
  template<typename T> void update(const StringKey& key, const T& value);
  /**
   * Get the value associated with the specified type and key.
   * @returns The value associated with the specified type and key. If the such
   * value does not exists def is returned.
   */
  template<typename T> T info(const StringKey& key, const T& def) const;

   /**
    * @brief Fill the ASCII output stream
    */
  virtual std::ostream& fillStream(std::ostream& s) const;
// =============================================================================
 private:
// =============================================================================
  BooleanMap m_boolean_store;
  IntegerMap m_integer_store;
  DoubleMap  m_double_store;
  StringMap m_string_store;
// =============================================================================
  template<typename T> void _insert(const StringKey& key, const T& value);
  template<typename T> void _update(const StringKey& key, const T& value);
}; // Cache
// =============================================================================
template<typename T> inline bool Cache::insert(const StringKey& key,
                                                          const T& value) {
  if (has(key)) return false;
  _insert(key,value);
  return true;
}

template<typename T> inline void Cache::update(const StringKey& key,
                                                          const T& value) {
  erase(key);
  _update(key,value);
}
// =============================================================================
// Boolean
// =============================================================================
template<> inline bool Cache::has_<bool>(const StringKey& key) const {
  return m_boolean_store.count(key) > 0;
}

template<> inline void Cache::_insert(const StringKey& key,
                                                         const bool& value) {
  m_boolean_store.insert(key,value);
}

template<> inline void Cache::_update(const StringKey& key,
                                                         const bool& value) {
  m_boolean_store.update(key,value);
}

template<> inline bool Cache::info<bool>(const StringKey& key,
    const bool& def) const {
  if (!has_<bool>(key)) return def;
  return m_boolean_store[key];
}
// =============================================================================
// Integer
// =============================================================================
template<> inline bool Cache::has_<int>(const StringKey& key) const {
  return m_integer_store.count(key) > 0;
}

template<> inline void Cache::_insert(const StringKey& key,
                                                          const int& value) {
  m_integer_store.insert(key, value);
}

template<> inline void Cache::_update(const StringKey& key,
                                                          const int& value) {
  m_integer_store.update(key, value);
}

template<> inline int Cache::info<int>(const StringKey& key,
                                                            const int& def) const {
  if (!has_<int>(key)) return def;
  return m_integer_store[key];
}
// =============================================================================
// Double
// =============================================================================
template<> inline bool Cache::has_<double>(const StringKey& key) const {
  return m_double_store.count(key) > 0;
}
template<> inline void Cache::_insert(const StringKey& key,
                                                       const double& value) {
  m_double_store.insert(key, value);
}

template<> inline void Cache::_update(const StringKey& key,
                                                       const double& value) {
  m_double_store.update(key, value);
}

template<> inline double Cache::info<double>(const StringKey& key,
                                                  const double& def)  const {
  if (!has_<double>(key)) return def;
  return m_double_store[key];
}
// =============================================================================
// String
// =============================================================================
template<> inline bool Cache::has_<std::string>(const StringKey& key) const {
  return m_string_store.count(key) > 0;
}

template<> inline void Cache::_insert(const StringKey& key,
                                                   const std::string& value) {
  m_string_store.insert(key, value);
}

template<> inline void Cache::_update(const StringKey& key,
                                                   const std::string& value) {
  m_string_store.update(key, value);
}

template<>
 inline std::string Cache::info<std::string>(const StringKey& key,
                                              const std::string& def) const {
  if (!has_<std::string>(key)) return def;
  return m_string_store[key];
}
// =============================================================================
inline bool Cache::has(const StringKey& key) const {
  return has_<bool>(key) || has_<int>(key) || has_<double>(key) ||
      has_<std::string>(key);
}

inline void Cache::erase(const StringKey& key) {
  m_boolean_store.erase(key);
  m_integer_store.erase(key);
  m_double_store.erase(key);
  m_string_store.erase(key);
}

inline const std::vector<const StringKey*> Cache::keys() const{
  std::vector<const StringKey*> result;
  BOOST_FOREACH (BooleanMap::reference value, m_boolean_store){
    result.push_back(&value.first);
  }
  BOOST_FOREACH (IntegerMap::reference value, m_integer_store){
      result.push_back(&value.first);
    }
  BOOST_FOREACH (DoubleMap::reference value, m_double_store){
      result.push_back(&value.first);
    }

  BOOST_FOREACH (StringMap::reference value, m_string_store){
      result.push_back(&value.first);
    }
  return result;
}
// =============================================================================
inline std::ostream& operator << (std::ostream& str, const Cache& obj) {
  return obj.fillStream(str);
}
// =============================================================================
inline std::ostream& Cache::fillStream(std::ostream& s) const
{
  std::string delim = "";
  s << "Cache {";
  BOOST_FOREACH (BooleanMap::reference value, m_boolean_store){
    s << delim;
    Gaudi::Utils::toStream(value.first, s);
    s << ": ";
    Gaudi::Utils::toStream(value.second,s);
    delim = ", ";
  }
  BOOST_FOREACH (IntegerMap::reference value, m_integer_store){
    s << delim;
    Gaudi::Utils::toStream(value.first, s);
    s<< ": ";
    Gaudi::Utils::toStream(value.second, s);
    delim = ", ";
  }
  BOOST_FOREACH (DoubleMap::reference value, m_double_store){
      s << delim;
      Gaudi::Utils::toStream(value.first, s);
      s << ": ";
      Gaudi::Utils::toStream(value.second, s);
      delim = ", ";
  }

  BOOST_FOREACH (StringMap::reference value, m_string_store){
      s << delim;
      Gaudi::Utils::toStream(value.first, s);
      s << ": ";
      Gaudi::Utils::toStream(value.second, s);
      delim = ", ";
  }
  s << "}";
  s << " CacheKeys [";
  delim = "";
  BOOST_FOREACH(const StringKey* key, keys()) {
    s << delim;
    Gaudi::Utils::toStream(*key, s);
    delim = ", ";
  }
  s << "]";
  return s;
}


} //  Hlt

#endif /* HltEvent_CACHE_H_ */
