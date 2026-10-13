#pragma once

#include "ShooterComponent.h"
#include "../Plant.h"
#include "../../Sexy.TodLib/TodFoley.h"

class Zombie;

/// @brief Where a plant's shot leaves from
enum class MuzzleAnchor : int32_t
{
	Plant = 0,          // the plant's top-left corner
	PeaHead = 1,        // the pea head's current position (it bobs while idling)
	PeaHeadMirrored = 2 // the pea head, mirrored (Leftpeater)
};

/// @brief How the projectile flies
enum class PlantShotMotion : int32_t
{
	Straight = 0,
	Lobbed = 1,    // arcs onto the target (catapults)
	Puff = 2,      // short range puff that fades out
	Backwards = 3, // straight, towards the house
	Homing = 4,    // follows its target (Cattail)
	ByRow = 5,     // angles into the row it was fired at (Threepeater)
	Cob = 6,       // goes up, then falls on the targeted spot (Cob Cannon)
	Star = 7       // five stars in a spread (Starfruit)
};

/// @brief Which animation plays the shooting animation
enum class ShotAnimTarget : int32_t
{
	Auto = 0,  // the head if it has the track, else the body; no track at all: fire right away
	Head2 = 1, // the second head (Split Pea's back head)
	Body = 2   // always the body
};

/// @brief One of a plant's weapons (most have one; Split Pea, Cactus and Kernel-pult have two)
struct PlantWeaponDef
{
	/// kNoProjectile: hits its row directly (Fume-shroom, Gloom-shroom) for mAreaDamage
	ProjectileType mProjectile = ProjectileType::PROJECTILE_PEA;
	PlantShotMotion mMotion = PlantShotMotion::Straight;
	int32_t mAreaDamage = 0;
	FoleyType mFoley = FoleyType::FOLEY_THROW;
	FoleyType mExtraFoley = FoleyType::NUM_FOLEY;

	MuzzleAnchor mMuzzleAnchor = MuzzleAnchor::Plant;
	int32_t mMuzzleX = 10;
	int32_t mMuzzleY = 5;
	ParticleEffect mMuzzleParticle = ParticleEffect::PARTICLE_NONE;
	int32_t mMuzzleParticleX = 0;
	int32_t mMuzzleParticleY = 0;

	ShotAnimTarget mAnimTarget = ShotAnimTarget::Auto;
	char mAnim[32] = "anim_shooting";
	float mAnimRate = 35.0f;
	/// Ticks between starting the animation and the shot (Plant::mShootingCounter)
	int32_t mShootDelay = 29;

	/// @brief Named fields for definition files (saves copy the whole struct)
	void Serialize(ComponentArchive &theArchive);
};

/// @brief Plant-specific shooting code for the few plants that need their own rules
enum class PlantShotPattern : int32_t
{
	Standard = 0,
	Threepeater = 1, // three heads, one per row
	SplitPea = 2,    // a front and a back head
	Starfruit = 3,   // shoots when a star would hit something in any of its five directions
	Cactus = 4,      // high spike for balloons, low spike otherwise
	Catapult = 5,    // aims at its target, sometimes throws its second weapon (Kernel-pult butter)
	CobCannon = 6    // fired by the player, re-arms afterwards
};

/// @brief A vanilla-style plant shooter: the plant's own update drives it, so timing and animations work exactly
/// like the original plants. Everything that used to be a seed-type branch in Plant::Fire, FindTargetAndFire,
/// UpdateShooting and UpdateShooter is data here.
///
/// It is a ShooterComponent, so GetComponent<ShooterComponent>() finds plant and zombie shooters alike. The generic
/// ShooterComponent::OnTick logic is not used for plants.
class PlantShooterComponent : public ShooterComponent
{
	COMPONENT_SUBTYPE(PlantShooterComponent, ShooterComponent)

	static constexpr int kMaxTicks = 4;
	static constexpr int kMaxExtraLaunches = 2;

	struct ExtraLaunch
	{
		/// Plant::mLaunchCounter value at which to launch again (Repeater: 25)
		int32_t mLaunchCounter = 0;
		int32_t mWeapon = 0;
	};

	/// @brief The shooter of a vanilla plant, or nullptr for plants that don't shoot
	static PlantShooterComponent *CreateVanilla(SeedType theSeedType);

	void OnTick() override
	{
	}
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief Plant::UpdateAbilities hooks
	void UpdateLaunch(Plant *thePlant);
	void UpdateShooting(Plant *thePlant);

	/// @brief Look for a target; if there is one, start shooting at it
	bool FindTargetAndFire(Plant *thePlant, int theRow, PlantWeapon theWeapon);
	/// @brief Fire a weapon now
	void Fire(Plant *thePlant, Zombie *theTargetZombie, int theRow, PlantWeapon theWeapon);

	const PlantWeaponDef &GetWeapon(PlantWeapon theWeapon) const;

	PlantShotPattern mPattern = PlantShotPattern::Standard;
	PlantWeaponDef mWeapons[2];
	/// Launches on its own every Plant::mLaunchRate ticks (false: Cob Cannon, fired by the player)
	bool mAutoLaunch = true;
	/// Which weapon the regular launch uses (Split Pea: the back head)
	int32_t mLaunchWeapon = 0;
	ExtraLaunch mExtraLaunches[kMaxExtraLaunches];
	int32_t mExtraLaunchCount = 0;
	/// Shooting-counter values at which the shot leaves (Gatling Pea: 4 shots); none = when the animation ends
	int32_t mFireTicks[kMaxTicks] = {};
	int32_t mFireTickCount = 0;
	/// Only fire if there still is a target when the shot leaves (Cattail)
	bool mFireNeedsTarget = false;
	/// Cloud particles during the animation (Fume-shroom, Gloom-shroom)
	ParticleEffect mCloudParticle = ParticleEffect::PARTICLE_NONE;
	int32_t mCloudX = 0;
	int32_t mCloudY = 0;
	int32_t mCloudTicks[kMaxTicks] = {};
	int32_t mCloudTickCount = 0;
	/// Catapult: 1 in N throws use the second weapon (Kernel-pult butter, 0 = never), shown/hidden track prefixes
	int32_t mAltWeaponChance = 0;
	char mAltWeaponPrefix[32] = "";
	char mMainWeaponPrefix[32] = "";

  private:
	/// @return False if the plant has no animation for it (the shot then fires right away)
	bool PlayShootAnim(Plant *thePlant, const PlantWeaponDef &theWeapon);
	void FireAtAnimEnd(Plant *thePlant);
	void RestoreIdle(Plant *thePlant);
	void LaunchThreepeater(Plant *thePlant);
	void FireStars(Plant *thePlant);
};
