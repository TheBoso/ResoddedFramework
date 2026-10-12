#pragma once

#include "../../ConstEnums.h"
#include "Component.h"

// The built-in components. Include this header to get all of them.

// ─── Teams ────────────────────────────────────────────────────────────────────

/// @brief Built-in teams. Any other int is a valid custom team (e.g. a third faction in a mod).
enum TeamId : int32_t
{
	TEAM_NONE = -1, // neutral: hostile to nobody
	TEAM_PLANTS = 0,
	TEAM_ZOMBIES = 1
};

/// @brief Which side an entity fights for. Projectiles carry their shooter's team and only hit hostile entities.
class TeamComponent : public Component
{
	COMPONENT_TYPE(TeamComponent)

	explicit TeamComponent(int theTeam = TEAM_NONE) : mTeam(theTeam), mNativeTeam(theTeam)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mTeam;
	/// The team the entity spawned on. Hypnosis changes mTeam, so mTeam != mNativeTeam means "converted".
	int32_t mNativeTeam;
};

/// @brief Which way the entity faces: +1 = right (towards the zombies' side), -1 = left (towards the house)
class FacingComponent : public Component
{
	COMPONENT_TYPE(FacingComponent)

	explicit FacingComponent(int theDirection = 1) : mDirection(theDirection), mNativeDirection(theDirection)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mDirection;
	/// The direction the entity spawned with. Plants mirror their attack area when mDirection differs.
	int32_t mNativeDirection;
};

// ─── Hypnosis ────────────────────────────────────────────────────────────────

/// @brief The entity can be hypnotized (switched to another team)
class HypnotizableComponent : public Component
{
	COMPONENT_TYPE(HypnotizableComponent)

	explicit HypnotizableComponent(int theHypnotizedTeam = TEAM_PLANTS, bool theFlipFacing = true)
	    : mHypnotizedTeam(theHypnotizedTeam), mFlipFacing(theFlipFacing)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// The team the entity joins when hypnotized
	int32_t mHypnotizedTeam;
	/// Turn around when hypnotized (zombies walk back, plants shoot the other way)
	bool mFlipFacing;
};

/// @brief Whoever bites this entity gets hypnotized (Hypno-shroom)
class HypnotizeOnEatenComponent : public Component
{
	COMPONENT_TYPE(HypnotizeOnEatenComponent)

	explicit HypnotizeOnEatenComponent(bool theRequiresAwake = true) : mRequiresAwake(theRequiresAwake)
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Mushrooms asleep during the day do nothing
	bool mRequiresAwake;
};

// ─── Combat ──────────────────────────────────────────────────────────────────

/// @brief Short plant: straight-flying projectiles pass over it (Puff-shroom, Spikeweed, Lily Pad...).
/// Lobbed projectiles still hit it.
class LowProfileComponent : public Component
{
	COMPONENT_TYPE(LowProfileComponent)
};

#include "ArmorComponents.h"
#include "HealthComponent.h"
#include "ProducerComponent.h"
#include "StatusEffects.h"
#include "ShooterComponent.h"
#include "PlantHeadComponent.h"
