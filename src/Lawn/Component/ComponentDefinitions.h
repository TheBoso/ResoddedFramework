#pragma once

#include <string>
#include <vector>
#include "EntityRef.h"

class ComponentList;

/// @brief Definition files: which components a plant, zombie or projectile type spawns with, and their values.
///
/// They live in definitions/ in the game folder (assets/definitions in the repo), one JSON file per type:
///
///     definitions/zombies/football.json
///     {
///       "zombie": "ZOMBIE_FOOTBALL",
///       "components": [
///         { "type": "WalkerComponent", "minSpeed": 0.66, "maxSpeed": 0.68, "houseEdgeX": -175 },
///         { "type": "HelmComponent", "helmType": "HELMTYPE_FOOTBALL", "health": 1400, ... },
///         ...
///       ]
///     }
///
/// "type" is a component's registered name (REGISTER_COMPONENT); the other keys are the names its Serialize gives
/// its fields. A field left out keeps the component's default. Enum values are written by name (see EnumNames.h).
/// // and /* */ comments are allowed.
///
/// A type with a definition spawns with exactly the components it lists; a type without one uses the built-in
/// tables (Archetypes::BuildVanilla*). The vanilla files were exported from those tables with
/// `LawnProject.exe -exportdefinitions=<folder>`, and `-checkdefinitions=<folder>` verifies they still build the
/// same components.
namespace ComponentDefinitions
{
/// @brief The folder definitions are read from, relative to the game folder
constexpr const char *kDefaultFolder = "definitions";

/// @brief (Re)load every definition in the folder: zombies/, plants/ and projectiles/
void Load(const std::string &theFolder = kDefaultFolder);
/// @brief Load the default folder unless something was loaded already
void EnsureLoaded();

/// @brief Build the components a type's definition lists. False when the type has no definition.
/// @param theType The ZombieType, SeedType or ProjectileType
bool Build(EntityKind theKind, int theType, ComponentList &theOut);

/// @brief How many definitions are loaded, and the problems found while loading or building them
size_t GetCount();
const std::vector<std::string> &GetMessages();

/// @brief Command-line tools: -exportdefinitions=<folder> writes the built-in tables as definition files,
/// -checkdefinitions=<folder> verifies that a folder's definitions build the same components as the built-in tables.
/// Runs before the game starts; returns true (and the exit code) when a tool ran.
bool RunCommandLineTool(int argc, char **argv, int &theExitCode);
} // namespace ComponentDefinitions
