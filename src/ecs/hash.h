//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_HASH_H_
#define ECS_ECS_HASH_H_

#include "types.h"

namespace ecs {

/**
 * @brief Generates a hash for the specified type.
 *
 * This function template generates a hash value for a given type using
 * the C++ type_index and typeid mechanisms. This is useful for type identification
 * and comparison within the ECS (Entity Component System) framework.
 *
 * @tparam T The type for which the hash is to be generated.
 * @return Hash The generated hash value representing the type.
 */
template<typename T>
Hash get_type_hash() {
    return std::type_index(typeid(T));
}

}    // namespace ecs

#endif    // ECS_ECS_HASH_H_
