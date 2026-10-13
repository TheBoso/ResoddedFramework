#pragma once

#include "Component.h"

// Traits of zombies that other code reacts to. Each replaces a list of zombie types that was checked all over
// Zombie.cpp/Plant.cpp; query them with Zombie::IsGiant(), IsVehicle(), IsInDanceTroupe() and IsSwimmer().

/// @brief Too big to be swallowed whole: a Chomper only bites it (Gargantuar, Giga-gargantuar, Dr. Zomboss)
class UnswallowableComponent : public Component
{
	COMPONENT_TYPE(UnswallowableComponent)
};

/// @brief A giant (Gargantuars): crushes spikes instead of avoiding them, never loses body parts, deep groans,
/// shakes the board as it falls, plays its death animation even when disintegrated. Its look (shadow,
/// charred remains, cracked body) comes from its ZombieBodyComponent and GargantuarBehaviour.
class GiantComponent : public Component
{
	COMPONENT_TYPE(GiantComponent)
};

/// @brief A vehicle (Zamboni, Catapult): drives over plants instead of eating them, gets destroyed by spikes,
/// ignores fire, never loses body parts and breaks down when badly damaged
class VehicleComponent : public Component
{
	COMPONENT_TYPE(VehicleComponent)

	VehicleComponent() = default;
	VehicleComponent(bool theCrushesSpikes, int theBreakdownHealth)
	    : mCrushesSpikes(theCrushesSpikes), mBreakdownHealth(theBreakdownHealth)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Drives right over Spikeweed instead of avoiding it (Zamboni; the Catapult stops in front)
	bool mCrushesSpikes = false;
	/// Below this much health it falls apart on its own
	int32_t mBreakdownHealth = 200;
};

/// @brief Part of a dance troupe (Dancer and Backup Dancers): the troupe moves, slows down and goes quiet together
class DanceTroupeComponent : public Component
{
	COMPONENT_TYPE(DanceTroupeComponent)
};

/// @brief Gets into the pool on its own terms (Snorkel, Dolphin Rider) instead of the regular pool entry
class SwimmerComponent : public Component
{
	COMPONENT_TYPE(SwimmerComponent)
};

/// @brief How a zombie moves across the lawn
enum class MoveStyle : int32_t
{
	/// Its feet follow the walk animation (the "_ground" track); slowed down when chilled
	Walk,
	/// Moves at its speed whatever the animation does, which plays at its own pace; slowed down when chilled
	/// (Catapult)
	Glide,
	/// Moves at its speed whatever the animation does, even when chilled (Zamboni)
	Drive,
	/// Never walks on its own (Bungee, Dr. Zomboss)
	Stand,
};

/// @brief How a zombie walks: its move style, its speed range, and how far past the lawn it must walk to get into the house.
/// Zombies without one walk at the regular 0.23-0.37 and get in at x = -100. Phases with their own speed
/// (Pole Vaulter running, Dolphin Rider in the pool, Digger tunnelling...) still override it.
class WalkerComponent : public Component
{
	COMPONENT_TYPE(WalkerComponent)

	WalkerComponent() = default;
	WalkerComponent(float theMinSpeed, float theMaxSpeed, int theHouseEdgeX)
	    : mMinSpeed(theMinSpeed), mMaxSpeed(theMaxSpeed), mHouseEdgeX(theHouseEdgeX)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief The regular walk: speeds as the plain zombies, with their two walk animations
	bool HasRegularSpeed() const
	{
		return mMinSpeed == kRegularMinSpeed && mMaxSpeed == kRegularMaxSpeed;
	}

	static constexpr float kRegularMinSpeed = 0.23f;
	static constexpr float kRegularMaxSpeed = 0.37f;
	static constexpr int kRegularHouseEdgeX = -100;

	/// Pixels per tick (picked at random between the two, again whenever the zombie changes pace)
	float mMinSpeed = kRegularMinSpeed;
	float mMaxSpeed = kRegularMaxSpeed;
	/// The zombie gets into the house once it walks past this x (big zombies have to walk further)
	int32_t mHouseEdgeX = kRegularHouseEdgeX;
	MoveStyle mMoveStyle = MoveStyle::Walk;
};
