/**
* @file types.h
 * @brief Fundamental ECS type definitions and forward declarations.
 */

#pragma once

#include <cstdint>
#include <typeindex>

namespace ecs {

struct Entity;

struct ECSBase;
struct ECS;

struct ComponentEntityList;

struct System;

template<typename... RTypes>
struct EntityIterator;

template<typename... RTypes>
struct EntitySubSet;

/** Raw ID type for entities, components, systems, listeners. */
using ID = std::size_t;

/** Runtime type identifier for components, events, etc. */
using Hash = std::type_index;

/** Sentinel invalid ID. */
#define ECS_INVALID_ID   ID(-1)
/** Sentinel invalid hash. */
#define ECS_INVALID_HASH Hash(typeid(void))

// Keep compatibility with your existing macros:
#define INVALID_ID   ECS_INVALID_ID
#define INVALID_HASH ECS_INVALID_HASH

} // namespace ecs
