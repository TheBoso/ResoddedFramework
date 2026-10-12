#pragma once

#include "../../ConstEnums.h"
#include "Component.h"
#include "../../SexyAppFramework/Rect.h"

using Sexy::Rect;

// Traits other entities react to. Each used to be a list of seed types checked in Zombie.cpp/Projectile.cpp;
// now any plant (or anything else) that has the component gets the behaviour.

/// @brief Spikes on the ground (Spikeweed, Spikerock): pops vehicle tyres, only Gargantuars and vehicles can
/// target it. Spikerock also survives being driven over or smashed, losing health instead.
class SpikesComponent : public Component
{
	COMPONENT_TYPE(SpikesComponent)

	explicit SpikesComponent(bool theSurvivesCrushing = false) : mSurvivesCrushing(theSurvivesCrushing)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	bool mSurvivesCrushing;
};

/// @brief Too tall to jump over (Tall-nut): stops Pole Vaulters, Dolphin Riders and Pogo Zombies
class TallComponent : public Component
{
	COMPONENT_TYPE(TallComponent)
};

/// @brief A defensive wall (Wall-nut, Tall-nut, Pumpkin): soft crunch when chewed, Ladder Zombies put ladders on
/// it, and in I, Zombie levels zombies chew through it twice as fast
class WallComponent : public Component
{
	COMPONENT_TYPE(WallComponent)

	explicit WallComponent(bool theShowsChewMarks = true) : mShowsChewMarks(theShowsChewMarks)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Bits fly off while it is chewed
	bool mShowsChewMarks;
};

/// @brief Zombies that bite it get a yucky face and switch lanes (Garlic)
class RepelOnEatenComponent : public Component
{
	COMPONENT_TYPE(RepelOnEatenComponent)
};

/// @brief Zombies leave it alone while it is awake (instant-use plants waiting to go off)
class InedibleComponent : public Component
{
	COMPONENT_TYPE(InedibleComponent)

	InedibleComponent(bool theBlocksChewing = true, bool theBlocksVehicles = true, bool theIgnoredByZombies = false)
	    : mBlocksChewing(theBlocksChewing), mBlocksVehicles(theBlocksVehicles), mIgnoredByZombies(theIgnoredByZombies)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Zombies can't chew it while it is awake
	bool mBlocksChewing;
	/// Zambonis and Catapults can't drive over it while it is awake
	bool mBlocksVehicles;
	/// Zombies can't target it at all, awake or not (Tangle Kelp)
	bool mIgnoredByZombies;
};

/// @brief Goes off (DoSpecial) when a zombie starts eating it (Blover)
class ActivatesWhenEatenComponent : public Component
{
	COMPONENT_TYPE(ActivatesWhenEatenComponent)

	explicit ActivatesWhenEatenComponent(bool theRequiresAwake = false) : mRequiresAwake(theRequiresAwake)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	bool mRequiresAwake;
};

/// @brief A buried mine (Potato Mine): can't be eaten once armed, and Diggers tunnel into it while it is unarmed
class MineComponent : public Component
{
	COMPONENT_TYPE(MineComponent)
};

/// @brief Turns projectiles that pass through it into other projectiles (Torchwood: pea -> fireball,
/// snow pea -> pea)
class ProjectileConverterComponent : public Component
{
	COMPONENT_TYPE(ProjectileConverterComponent)

	static constexpr int kMaxConversions = 4;

	ProjectileConverterComponent() = default;
	/// @brief Add a conversion; returns *this so they can be chained
	ProjectileConverterComponent &Convert(ProjectileType theFrom, ProjectileType theTo);
	/// @brief What theType becomes, or theType itself if this converter doesn't touch it
	ProjectileType GetConversion(ProjectileType theType) const;

	/// @brief Where projectiles get converted
	static Rect GetConversionRect(Plant *thePlant);

	void OnTick() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mCount = 0;
	ProjectileType mFrom[kMaxConversions] = {};
	ProjectileType mTo[kMaxConversions] = {};
};
