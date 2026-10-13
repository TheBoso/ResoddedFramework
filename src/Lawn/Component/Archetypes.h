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

/// @brief Components built for an entity before it gets them (from a definition file or the vanilla tables).
/// Owns them until AttachTo hands them to the entity.
class ComponentList
{
  public:
	ComponentList() = default;
	ComponentList(const ComponentList &) = delete;
	ComponentList &operator=(const ComponentList &) = delete;
	~ComponentList()
	{
		for (Component *aComponent : mComponents)
		{
			delete aComponent;
		}
	}

	/// @brief Construct a component and add it: Add<WalkerComponent>(0.66f, 0.68f, -175)
	template <typename T, typename... TArgs>
	T *Add(TArgs &&...theArgs)
	{
		T *aComponent = new T(std::forward<TArgs>(theArgs)...);
		mComponents.push_back(aComponent);
		return aComponent;
	}
	/// @brief Add a component made elsewhere (the list owns it from now on)
	template <typename T>
	T *Add(T *theComponent)
	{
		mComponents.push_back(theComponent);
		return theComponent;
	}
	/// @brief The first component of that class (or derived from it), or nullptr
	template <typename T>
	T *Find() const
	{
		for (Component *aComponent : mComponents)
		{
			if (aComponent->IsA(GetComponentTypeId<T>()))
				return static_cast<T *>(aComponent);
		}
		return nullptr;
	}
	/// @brief Attach every component to the entity, in order; the list is empty afterwards
	void AttachTo(GameObject *theOwner);
	/// @brief Hand the components over (the caller owns them); the list is empty afterwards
	std::vector<Component *> Release()
	{
		std::vector<Component *> aComponents = std::move(mComponents);
		mComponents.clear();
		return aComponents;
	}
	const std::vector<Component *> &Get() const
	{
		return mComponents;
	}

  private:
	std::vector<Component *> mComponents;
};

/// @brief The default components of the vanilla entities ("archetypes").
/// Applied when an entity spawns, and to entities from saves made before the component system.
/// A definition file for the type (see ComponentDefinitions.h) replaces the built-in tables; without one the
/// Build* functions below are used. Content can still add/remove components after spawning (see EntityBuilder.h).
namespace Archetypes
{
/// @param theFromOldSave The plant comes from a save without components: carry its timers over
void ApplyPlantDefaults(Plant *thePlant, bool theFromOldSave = false);
void ApplyZombieDefaults(Zombie *theZombie);
void ApplyProjectileDefaults(Projectile *theProjectile);

/// @brief The built-in components of a vanilla type (what the definition files were exported from)
/// @param theLaunchRate The plant's production rate (its definition's launch rate)
void BuildVanillaPlant(SeedType theSeedType, int theLaunchRate, ComponentList &theOut);
void BuildVanillaZombie(ZombieType theType, ComponentList &theOut);
void BuildVanillaProjectile(ProjectileType theType, ComponentList &theOut);

/// @brief The team a projectile type belongs to when nobody assigned one (old saves only; spawners set it)
int DefaultProjectileTeam(ProjectileType theProjectileType);

const PlantShotDefinition &GetPlantShot(SeedType theSeedType);
} // namespace Archetypes
