#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include "Component.h"

/// @brief Names of the game's enum values ("ZOMBIE_FOOTBALL", "HELMTYPE_PAIL", "IMAGE_ZOMBIEFOOTBALLHEAD_ID"...),
/// so definition files can use names instead of numbers. The tables are generated from the headers by
/// tools/generate_enum_names.py into EnumNames.generated.cpp: rerun it after adding enum values.
struct EnumNameEntry
{
	const char *mName;
	int64_t mValue;
};

namespace EnumNames
{
/// @param theEnumName The C++ name of the enum ("ZombieType")
void Register(const void *theEnumKey, const char *theEnumName, const EnumNameEntry *theEntries, size_t theCount);

/// @brief Name one more value at runtime (custom types from definition files). A name that had another value is
/// moved to this one.
void Add(const void *theEnumKey, const std::string &theName, int64_t theValue);

/// @brief The name of a value, or nullptr when it has none (or the enum has no registered names)
const char *NameOf(const void *theEnumKey, int64_t theValue);
/// @brief The value of a name; false when the enum has no such name
bool ValueOf(const void *theEnumKey, const char *theName, int64_t &theValue);

template <typename T>
const char *NameOf(T theValue)
{
	return NameOf(EnumKey<T>(), (int64_t)theValue);
}
template <typename T>
bool ValueOf(const char *theName, T &theValue)
{
	int64_t aValue;
	if (!ValueOf(EnumKey<T>(), theName, aValue))
		return false;
	theValue = (T)aValue;
	return true;
}
} // namespace EnumNames
