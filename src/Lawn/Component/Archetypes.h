#pragma once

#include "Components.h"

class Plant;
class Zombie;
class Projectile;

/// @brief "Fires nothing" marker for PlantShotDefinition::mProjectileType
constexpr ProjectileType kNoProjectile = (ProjectileType)-1;

/// @brief What a plant shoots. Shared by Plant::Fire (vanilla plants) and ShooterComponent::CreateLike
/// (anything that should shoot like that plant, e.g. a Zombotany zombie).
struct PlantShotDefinition
{
	SeedType mSeedType;
	ProjectileType mProjectileType; // kNoProjectile: the plant does not fire projectiles
	ShotArc mArc;
	int mShotsPerVolley;
	/// Pixels ahead the plant can see; 0 = the whole row
	int mRange;
};

/// @brief The default components of the vanilla entities ("archetypes").
/// Applied when an entity spawns, and to entities from saves made before the component system.
/// Content that needs different behaviour adds/removes components after spawning (see EntityBuilder.h).
namespace Archetypes
{
/// @param theFromOldSave The plant comes from a save without components: carry its timers over
void ApplyPlantDefaults(Plant *thePlant, bool theFromOldSave = false);
void ApplyZombieDefaults(Zombie *theZombie);
void ApplyProjectileDefaults(Projectile *theProjectile);

/// @brief The team a projectile type belongs to when nobody assigned one (old saves only; spawners set it)
int DefaultProjectileTeam(ProjectileType theProjectileType);

const PlantShotDefinition &GetPlantShot(SeedType theSeedType);
} // namespace Archetypes
