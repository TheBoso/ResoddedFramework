#pragma once

#include <string>
#include <vector>
#include "../../ConstEnums.h"
#include "../Plant.h"
#include "../Projectile.h"
#include "../Zombie.h"

/// @brief Zombie, plant and projectile types added by definition files, without code changes (see docs/component-system/
/// DEFINITIONS.md):
///
///     { "zombie": "ZOMBIE_BLACK_FOOTBALL", "basedOn": "ZOMBIE_FOOTBALL",
///       "info": { "name": "Black Football Zombie", ... }, "components": [ ... ] }
///
/// A custom type is a variant of a vanilla type. In play, the zombie or plant has its base type in mZombieType /
/// mSeedType, so every rule that is still written per type treats it like the base. Its own type is in
/// mDefinitionType: that one picks its components (its definition file), animation, name, cost and Almanac entry.
///
/// Custom types get the IDs from FIRST_CUSTOM_ZOMBIE_TYPE / FIRST_CUSTOM_SEED_TYPE up (ConstEnums.h), so seed
/// packets, waves, saves and the Almanac can name them like any other type. A file can ask for a fixed "id", which
/// keeps saves valid when other custom types are added or removed; otherwise the next free ID is used.
namespace CustomTypes
{
struct ZombieInfo
{
	std::string mTypeName; // "ZOMBIE_BLACK_FOOTBALL", also its string-table key
	ZombieType mType = ZombieType::ZOMBIE_INVALID;
	ZombieType mBase = ZombieType::ZOMBIE_INVALID;
	/// What GetZombieDefinition returns for it (mZombieName points at mTypeName)
	ZombieDefinition mDefinition = {};
	/// Adventure levels 1-50 it can appear on (the base's unless the file lists its own)
	bool mAllowedOnLevel[50] = {};
	std::string mFile;
	bool mActive = false;
};

struct PlantInfo
{
	std::string mTypeName; // "SEED_FIRE_PEASHOOTER", also its string-table key
	SeedType mType = SeedType::SEED_NONE;
	SeedType mBase = SeedType::SEED_NONE;
	/// What GetPlantDefinition returns for it (mPlantName points at mTypeName)
	PlantDefinition mDefinition = {};
	std::string mFile;
	bool mActive = false;
};

struct ProjectileInfo
{
	std::string mTypeName; // "PROJECTILE_FIRE_STAR"
	ProjectileType mType = ProjectileType::PROJECTILE_PEA;
	ProjectileType mBase = ProjectileType::PROJECTILE_PEA;
	/// What Projectile::GetProjectileDef returns for it (damage, image row)
	ProjectileDefinition mDefinition = {};
	/// Its own image: a game image ("IMAGE_PROJECTILEPEA_ID") or an image id from resources.xml; "" for its base's
	std::string mImage;
	/// Scale of its image (0: its base's)
	float mScale = 0.0f;
	std::string mFile;
	bool mActive = false;
};

bool IsCustom(ZombieType theType);
bool IsCustom(SeedType theType);
bool IsCustom(ProjectileType theType);
/// @brief A registered custom type (or any vanilla one)
bool Exists(ZombieType theType);
bool Exists(SeedType theType);
bool Exists(ProjectileType theType);

/// @brief The vanilla type a type plays as: its base for a custom type, itself otherwise
ZombieType PlaysAs(ZombieType theType);
SeedType PlaysAs(SeedType theType);
ProjectileType PlaysAs(ProjectileType theType);

/// @brief Every zombie type: the vanilla ones, then the custom ones (in ID order)
const std::vector<ZombieType> &AllZombieTypes();
/// @brief Every plant: the vanilla ones (SEED_PEASHOOTER up to NUM_SEED_TYPES), then the custom ones
const std::vector<SeedType> &AllPlantTypes();
const std::vector<ZombieType> &CustomZombieTypes();
const std::vector<SeedType> &CustomPlantTypes();
const std::vector<ProjectileType> &CustomProjectileTypes();

/// @brief A custom type's info, or nullptr for a vanilla or unregistered type
ZombieInfo *GetZombie(ZombieType theType);
PlantInfo *GetPlant(SeedType theType);
ProjectileInfo *GetProjectile(ProjectileType theType);
/// @brief A custom projectile's own image, or nullptr for its base's (looked up the first time it is drawn)
Sexy::Image *GetProjectileImage(ProjectileType theType);

// ── Registration (ComponentDefinitions::Load) ──

/// @brief Start a reload: types not registered again are no longer listed (their IDs stay reserved)
void BeginReload();
/// @brief Register (or re-register) a custom type by name. theRequestedId: a fixed ID, or -1 for the next free one.
/// nullptr (and theError) when it can't be registered.
ZombieInfo *RegisterZombie(const std::string &theTypeName, ZombieType theBase, int theRequestedId,
                           std::string &theError);
PlantInfo *RegisterPlant(const std::string &theTypeName, SeedType theBase, int theRequestedId, std::string &theError);
ProjectileInfo *RegisterProjectile(const std::string &theTypeName, ProjectileType theBase, int theRequestedId,
                                   std::string &theError);
/// @brief Rebuild the type lists after registering
void EndReload();

/// @brief Set a string of the game's string table ("[KEY]" in the game's text)
void SetString(const std::string &theKey, const std::string &theValue);
/// @brief A string of the game's string table, or "" when it has none
std::string GetString(const std::string &theKey);
} // namespace CustomTypes
