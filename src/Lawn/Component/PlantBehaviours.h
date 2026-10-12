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
// The behaviours of the vanilla plants (implemented in VanillaPlantBehaviours.cpp). They can be removed, combined or
// given to another plant, but they play the original plant's animation tracks (anim_bite, anim_lookleft...), so on
// a plant whose animation lacks those tracks they act without animating.

#define VANILLA_PLANT_BEHAVIOUR(ClassName)                                                                             	class ClassName : public PlantBehaviourComponent                                                                   	{                                                                                                                  		COMPONENT_SUBTYPE(ClassName, PlantBehaviourComponent)                                                          		void UpdateBehaviour(Plant *thePlant) override;                                                                	};

// Behaviours that are pure animation/state bookkeeping, with nothing worth tuning
VANILLA_PLANT_BEHAVIOUR(FlowerPotBehaviour)
VANILLA_PLANT_BEHAVIOUR(LilyPadBehaviour)
VANILLA_PLANT_BEHAVIOUR(ImitaterBehaviour)
VANILLA_PLANT_BEHAVIOUR(CobCannonBehaviour)
VANILLA_PLANT_BEHAVIOUR(CactusBehaviour)
VANILLA_PLANT_BEHAVIOUR(SunShroomGrowthBehaviour)
VANILLA_PLANT_BEHAVIOUR(PotatoMineArmingBehaviour)

#undef VANILLA_PLANT_BEHAVIOUR

/// @brief Squash: looks at a zombie coming close, jumps on it and crushes everything it lands on
class SquashBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(SquashBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks spent looking at the target, then winding up the jump
	int32_t mLookTicks = 80;
	int32_t mWindupTicks = 30;
	/// Damage to everything it lands on (1800 kills anything but the boss)
	int32_t mDamage = 1800;

  private:
	void DoSquashDamage(Plant *thePlant);
};

/// @brief Chomper: swallows the zombie in front of it whole, then chews for a long time.
/// Zombies with an UnswallowableComponent (Gargantuars, Dr. Zomboss) just get bitten.
class ChomperBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(ChomperBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks from opening the mouth to the bite landing
	int32_t mBiteTicks = 70;
	/// Ticks spent chewing a swallowed zombie
	int32_t mChewTicks = 4000;
	/// Damage of a bite on a zombie too big to swallow
	int32_t mBiteDamage = 40;
};

/// @brief Spikeweed / Spikerock: hurts every zombie standing on it
class SpikeweedBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(SpikeweedBehaviour, PlantBehaviourComponent)

	SpikeweedBehaviour() = default;
	SpikeweedBehaviour(int theDamage, int theFirstHitTick, int theSecondHitTick)
	    : mDamage(theDamage), mFirstHitTick(theFirstHitTick), mSecondHitTick(theSecondHitTick)
	{
	}

	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mDamage = 20;
	/// Points of the 100-tick attack animation (counting down) where the spikes hit; -1 = no second hit
	int32_t mFirstHitTick = 75;
	int32_t mSecondHitTick = -1;
};

/// @brief Magnet-shroom: pulls metal (buckets, helmets, doors, ladders...) off nearby zombies
class MagnetShroomBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(MagnetShroomBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Reach in pixels, larger for zombies busy eating
	int32_t mRange = 270;
	int32_t mRangeWhileEating = 320;
	/// Rows above and below it reaches
	int32_t mRowRange = 2;
	/// Ticks before it can pull again
	int32_t mRechargeTicks = 1500;
};

/// @brief Gold Magnet: collects coins and diamonds on the lawn every so often
class GoldMagnetBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(GoldMagnetBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks between pulls (random between the two)
	int32_t mRechargeMinTicks = 200;
	int32_t mRechargeMaxTicks = 300;
	/// 1 in this many ticks it checks for coins to pull
	int32_t mPullChance = 50;
};

/// @brief Grave Buster: eats the grave it is planted on, then drops loot and disappears
class GraveBusterBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(GraveBusterBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mEatTicks = 400;
};

/// @brief Tangle Kelp: grabs a zombie in the water and drags it under
class TangleKelpBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(TangleKelpBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks from grabbing to the zombie being gone (it is pulled under halfway through)
	int32_t mGrabTicks = 100;
};

/// @brief Scaredy-shroom: hides (and stops shooting) while a hostile zombie is close
class ScaredyShroomBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(ScaredyShroomBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	int32_t mFearRadius = 120;
	/// Rows above and below it watches
	int32_t mFearRowRange = 1;
};

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
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks between being triggered and actually blocking
	int32_t mTriggerTicks = 5;
};

/// @brief Coffee Bean: wakes up the mushroom it is planted on, then crumbles
class CoffeeBeanBehaviour : public PlantBehaviourComponent
{
	COMPONENT_SUBTYPE(CoffeeBeanBehaviour, PlantBehaviourComponent)
	void UpdateBehaviour(Plant *thePlant) override;
	void DoSpecial(Plant *thePlant) override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// Ticks until the mushroom is awake
	int32_t mWakeTicks = 100;
};
