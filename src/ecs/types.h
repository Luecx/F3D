//
// Created by Luecx on 15.06.2021.
//

#ifndef ECS_ECS_TYPES_H_
#define ECS_ECS_TYPES_H_

#include <cstdint>
#include <typeindex>

namespace ecs {

/**
 * @brief Forward declaration of the Entity structure.
 */
struct Entity;

/**
 * @brief Forward declaration of the ECS structure.
 */
struct ECS;

/**
 * @brief Forward declaration of the System structure.
 */
struct System;

/**
 * @brief Base class for component containers.
 */
struct ComponentContainerBase;

/**
 * @brief Template structure for component containers.
 *
 * @tparam T The type of the component.
 */
template<typename T>
struct ComponentContainer;

/**
 * @brief Template structure for iterating over entities with specific component types.
 *
 * @tparam RTypes The component types required for the iteration.
 */
template<typename... RTypes>
struct EntityIterator;

/**
 * @brief Template structure for a subset of entities with specific component types.
 *
 * @tparam RTypes The component types that define the subset.
 */
template<typename... RTypes>
struct EntitySubSet;

/**
 * @brief Type alias for entity IDs.
 */
typedef uint32_t ID;

/**
 * @brief Type alias for component hashes using std::type_index.
 */
typedef std::type_index Hash;

/**
 * @brief Constant representing an invalid entity ID.
 */
#define INVALID_ID ID(-1)

}    // namespace ecs

#endif    // ECS_ECS_TYPES_H_
