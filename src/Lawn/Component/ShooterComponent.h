#pragma once

#include "../../ConstEnums.h"
#include "Component.h"

class Reanimation;

enum class ShotArc : int32_t
{
	Straight = 0, // flies along the row in the facing direction
	Lobbed = 1    // arcs onto the target, like the catapult plants and the Catapult Zombie
};

/// @brief Fires projectiles at the nearest hostile plant or zombie in its row, in the direction its owner faces.
/// Works on zombies and plants alike; the projectiles carry the owner's team.
///
///     zombie->AddComponent<ShooterComponent>(ProjectileType::PROJECTILE_MELON, ShotArc::Lobbed, 300);
///
/// Vanilla shooter plants still fire through Plant::UpdateShooter; this is for zombies and new content.
class ShooterComponent : public Component
{
	COMPONENT_TYPE(ShooterComponent)

	ShooterComponent() = default;
	ShooterComponent(ProjectileType theProjectileType, ShotArc theArc, int theInterval = 150, int theShotsPerVolley = 1)
	    : mProjectileType(theProjectileType), mArc(theArc), mInterval(theInterval), mShotsPerVolley(theShotsPerVolley),
	      mCountdown(theInterval)
	{
	}

	/// @brief A shooter that fires like the given plant (projectile, arc, fire rate, volley size).
	/// Returns nullptr for plants that do not fire projectiles.
	static ShooterComponent *CreateLike(SeedType theSeedType);

	void OnTick() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	ProjectileType mProjectileType = ProjectileType::PROJECTILE_PEA;
	ShotArc mArc = ShotArc::Straight;
	/// Ticks between volleys (100 ticks = 1 second)
	int32_t mInterval = 150;
	int32_t mShotsPerVolley = 1;
	/// Ticks between the shots of one volley
	int32_t mShotSpacing = 17;
	/// Ticks of "anim_shooting" played before the first shot of a volley
	int32_t mWindup = 35;
	/// How far ahead (pixels) it looks for targets; 0 = the whole row
	int32_t mRange = 0;
	/// DamageRangeFlags bits: what the projectiles can hit (ground, flying, ...)
	int32_t mDamageRangeFlags = 1 << DamageRangeFlags::DAMAGES_GROUND;
	/// Only shoot when something hostile is in range
	bool mRequiresTarget = true;
	/// Muzzle offset: X is applied in the facing direction, Y downwards
	float mMuzzleX = 0.0f;
	float mMuzzleY = 0.0f;

	// Runtime state
	int32_t mCountdown = 150;
	int32_t mShotsLeft = 0;
	int32_t mShotTimer = 0;

  protected:
	/// @brief Can the owner shoot right now? Override to add conditions.
	virtual bool CanShoot();
	/// @brief Spawn one projectile. Override for custom shots.
	virtual void Fire();
	/// @brief The animation to play "anim_shooting" on: a zombie's plant head, or a plant's body
	virtual Reanimation *GetShootingReanim();

	struct Target
	{
		bool mFound = false;
		float mX = 0.0f;
		float mY = 0.0f;
	};
	Target FindTarget(float theOriginX);
	void GetMuzzle(float &theX, float &theY);
	int GetRow();
	int GetDirection();
	int GetTeam();
	void PlayAnim(bool theShooting);
};
