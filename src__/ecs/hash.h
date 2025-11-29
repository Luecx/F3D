/**
* @file hash.h
 * @brief Utilities for generating stable runtime type hashes.
 */

#pragma once

#include "types.h"

namespace ecs {

/**
 * @brief Generates a hash for the specified type using std::type_index.
 *
 * @tparam T Type for which the hash is generated.
 * @return Hash value representing type T.
 */
template<typename T>
inline Hash get_type_hash() {
  return std::type_index(typeid(T));
}

} // namespace ecs
