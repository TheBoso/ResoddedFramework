#pragma once

#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "Component.h"

/// @brief Base of plant behaviours (what used to be the seed-type switches in Plant::UpdateAbilities and
/// Plant::DoSpecial).
///
/// Unlike OnTick, UpdateBehaviour runs inside Plant::UpdateAbilities, at the spot the seed dispatch used to be:
/// only while the plant is in play, awake, not squished and not grabbed by a Bungee, and in step with the plant's
/// own counters (mStateCountdown, mDoSpecialCountdown). A plant can have several behaviours; they run in the order
/// they were added.
///
///     // A Wall-nut that blows up like a Cherry Bomb when its fuse runs out
///     plant->AddComponent<FuseComponent>(300);
///     plant->AddComponent(ExplodeComponent::CherryBomb());
class PlantBehaviourComponent : public Component
{
	COMPONENT_TYPE(PlantBehaviourComponent)

	/// @brief Per-tick behaviour (see the class comment for when it runs)
	virtual void UpdateBehaviour(Plant *thePlant)
	{
		(void)thePlant;
	}
	/// @brief Plant::DoSpecial: the plant's activation (exploding, blowing, ...). Every behaviour of the plant gets it.
	virtual void DoSpecial(Plant *thePlant)
	{
		(void)thePlant;
	}
};

// ─── Instant-use effects (data-driven) ───────────────────────────────────────

/// @brief Waits until the plant is awake, then lights a fuse; Plant::DoSpecial runs when it burns down
/// (Ice-shroom, Doom-shroom). Cherry Bomb and Jalapeño have their fuse set when planted instead.
class FuseComponent : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(FuseComponent, PlantBehaviourComponent)

	explicit FuseComponent(int theFuseTicks = 100) : mFuseTicks(theFuseTicks)
	{
	}

	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mFuseTicks;

  protected:
	/// @brief The fuse was just lit (play a warm-up animation, a sound...)
	virtual void OnLit(Plant *thePlant)
	{
		(void)thePlant;
	}
};

/// @brief Doom-shroom's fuse: swells and shakes before going off
class DoomFuseComponent : public FuseComponent
{
	COMPONENT_SUBTYPE(DoomFuseComponent, FuseComponent)
	using FuseComponent::FuseComponent;

  protected:
	void OnLit(Plant *thePlant) override;
};

/// @brief Blows up zombies around the plant, then the plant is used up (Cherry Bomb, Doom-shroom, Potato Mine)
class ExplodeComponent : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(ExplodeComponent, PlantBehaviourComponent)

	ExplodeComponent() = default;
	static ExplodeComponent *CherryBomb();
	static ExplodeComponent *DoomShroom();
	static ExplodeComponent *PotatoMine();

	void DoSpecial(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Blast radius in pixels, and how many rows above/below it reaches
	int32_t mRadius = 115;
	int32_t mRowRange = 1;
	/// Burned zombies turn to ash instead of falling over
	bool mBurns = true;
	/// Blast center offset from the plant's center
	int32_t mCenterOffsetX = 0;
	FoleyType mFoley1 = FoleyType::FOLEY_CHERRYBOMB;
	FoleyType mFoley2 = FoleyType::FOLEY_JUICY;
	/// ResourceId of a sound sample to play, -1 for none
	int32_t mSoundId = -1;
	ParticleEffect mParticle = ParticleEffect::PARTICLE_POWIE;
	/// Draw the particle in the plant's row instead of on top of everything
	bool mParticleInRow = false;
	/// Leave a crater for this many ticks (0 = none)
	int32_t mCraterTicks = 0;
	/// Also destroys the plants around it (Doom-shroom)
	bool mKillsNearbyPlants = false;
	/// AchievementID to give for the blast (-1 = none): when more than mAchievementMinZombies zombies are within
	/// mAchievementRadius / mAchievementRowRange of the blast, or every time if mAchievementMinZombies is -1
	int32_t mAchievement = -1;
	int32_t mAchievementMinZombies = -1;
	int32_t mAchievementRadius = 0;
	int32_t mAchievementRowRange = 0;
	bool mAchievementInIZombie = true;
};

/// @brief Burns every zombie in the plant's row, then the plant is used up (Jalapeño)
class BurnRowComponent : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(BurnRowComponent, PlantBehaviourComponent)

	explicit BurnRowComponent(int theRowSpread = 0) : mRowSpread(theRowSpread)
	{
	}

	void DoSpecial(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Also burn this many rows above and below (0 = only its own row)
	int32_t mRowSpread;
};

/// @brief Freezes every zombie on the lawn, then the plant is used up (Ice-shroom)
class FreezeAllComponent : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(FreezeAllComponent, PlantBehaviourComponent)

	void DoSpecial(Plant *thePlant) override;
};

// ─── Vanilla state machines ──────────────────────────────────────────────────
// Behaviours whose code still lives in Plant.cpp (Plant::UpdateChomper etc.): each is its own component, so it can
// be removed, combined or given to another plant, but it drives the original plant's animations, so on a different
// plant it may not look right. Moving the code into the components is the next step.

#define VANILLA_PLANT_BEHAVIOUR(ClassName)                                                                             \
	class ClassName : public PlantBehaviourComponent                                                                   \
	{                                                                                                                  \
		COMPONENT_SUBTYPE(ClassName, PlantBehaviourComponent)                                                          \
		void UpdateBehaviour(Plant *thePlant) override;                                                                \
	};

VANILLA_PLANT_BEHAVIOUR(SquashBehaviour)
VANILLA_PLANT_BEHAVIOUR(ChomperBehaviour)
VANILLA_PLANT_BEHAVIOUR(FlowerPotBehaviour)
VANILLA_PLANT_BEHAVIOUR(LilyPadBehaviour)
VANILLA_PLANT_BEHAVIOUR(ImitaterBehaviour)
VANILLA_PLANT_BEHAVIOUR(CobCannonBehaviour)
VANILLA_PLANT_BEHAVIOUR(CactusBehaviour)
VANILLA_PLANT_BEHAVIOUR(MagnetShroomBehaviour)
VANILLA_PLANT_BEHAVIOUR(GoldMagnetBehaviour)
VANILLA_PLANT_BEHAVIOUR(SunShroomGrowthBehaviour)
VANILLA_PLANT_BEHAVIOUR(GraveBusterBehaviour)
VANILLA_PLANT_BEHAVIOUR(PotatoMineArmingBehaviour)
VANILLA_PLANT_BEHAVIOUR(SpikeweedBehaviour)
VANILLA_PLANT_BEHAVIOUR(TangleKelpBehaviour)
VANILLA_PLANT_BEHAVIOUR(ScaredyShroomBehaviour)

#undef VANILLA_PLANT_BEHAVIOUR

/// @brief Blover: blows away flying zombies once its intro animation ends
class BloverBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(BloverBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void DoSpecial(Plant *thePlant) override;
};

/// @brief Umbrella Leaf: pops up to bounce lobbed projectiles and Bungees
class UmbrellaBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(UmbrellaBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void DoSpecial(Plant *thePlant) override;
};

/// @brief Coffee Bean: wakes up the mushroom it is planted on, then crumbles
class CoffeeBeanBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(CoffeeBeanBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void DoSpecial(Plant *thePlant) override;
};
