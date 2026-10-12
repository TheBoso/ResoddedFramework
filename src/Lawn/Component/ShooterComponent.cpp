#include "ShooterComponent.h"
#include "Archetypes.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "Teams.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodFoley.h"

REGISTER_COMPONENT(ShooterComponent)

ShooterComponent *ShooterComponent::CreateLike(SeedType theSeedType)
{
	const PlantShotDefinition &aShot = Archetypes::GetPlantShot(theSeedType);
	if (aShot.mProjectileType == kNoProjectile || aShot.mProjectileType == ProjectileType::PROJECTILE_COBBIG)
		return nullptr;

	int aLaunchRate = GetPlantDefinition(theSeedType).mLaunchRate;
	auto *aShooter = new ShooterComponent(aShot.mProjectileType, aShot.mArc, aLaunchRate > 0 ? aLaunchRate : 150,
	                                      aShot.mShotsPerVolley);
	aShooter->mRange = aShot.mRange;
	aShooter->mShotSpacing = aShot.mShotsPerVolley > 2 ? 17 : 26;
	aShooter->mWindup = aShot.mArc == ShotArc::Lobbed ? 36 : 35;
	return aShooter;
}

int ShooterComponent::GetRow()
{
	GameObject *anOwner = GetOwnerObject();
	return anOwner ? anOwner->mRow : 0;
}

int ShooterComponent::GetDirection()
{
	if (Zombie *aZombie = GetZombie())
		return Teams::FacingOf(aZombie);
	if (Plant *aPlant = GetPlant())
		return Teams::FacingOf(aPlant);
	return 1;
}

int ShooterComponent::GetTeam()
{
	if (Zombie *aZombie = GetZombie())
		return Teams::Of(aZombie);
	if (Plant *aPlant = GetPlant())
		return Teams::Of(aPlant);
	return TEAM_NONE;
}

bool ShooterComponent::CanShoot()
{
	if (Zombie *aZombie = GetZombie())
	{
		return !aZombie->IsDeadOrDying() && aZombie->mHasHead && aZombie->IsOnBoard() && !aZombie->IsImmobilizied() &&
		       aZombie->mZombiePhase != ZombiePhase::PHASE_RISING_FROM_GRAVE;
	}
	if (Plant *aPlant = GetPlant())
	{
		return aPlant->IsOnBoard() && !aPlant->mIsAsleep && !aPlant->NotOnGround();
	}
	return false;
}

ShooterComponent::Target ShooterComponent::FindTarget(float theOriginX)
{
	Board *aBoard = GetBoard();
	int aTeam = GetTeam();
	int aRow = GetRow();
	int aDirection = GetDirection();

	Target aBest;
	float aBestDistance = 0.0f;
	auto aConsider = [&](float theX, float theY)
	{
		float aDistance = (theX - theOriginX) * aDirection;
		if (aDistance < 0.0f || (mRange > 0 && aDistance > mRange))
			return;
		if (!aBest.mFound || aDistance < aBestDistance)
		{
			aBest = { true, theX, theY };
			aBestDistance = aDistance;
		}
	};

	Plant *aPlant = nullptr;
	while (aBoard->IteratePlants(aPlant))
	{
		if (aPlant->mRow == aRow && !aPlant->NotOnGround() && Teams::CanAttack(aTeam, aPlant))
		{
			aConsider(aPlant->mX + 40.0f, (float)aPlant->mY);
		}
	}

	Zombie *aZombie = nullptr;
	while (aBoard->IterateZombies(aZombie))
	{
		if ((aZombie->mRow == aRow || aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) &&
		    Teams::CanAttack(aTeam, aZombie, (unsigned int)mDamageRangeFlags))
		{
			Rect aRect = aZombie->GetZombieRect();
			aConsider(aRect.mX + aRect.mWidth / 2.0f, (float)aRect.mY);
		}
	}

	return aBest;
}

void ShooterComponent::GetMuzzle(float &theX, float &theY)
{
	int aDirection = GetDirection();
	theX = 0.0f;
	theY = 0.0f;

	if (Zombie *aZombie = GetZombie())
	{
		// Same spot as the vanilla Peashooter Zombie: its head track, shifted when facing right
		Reanimation *aBodyReanim = GetApp()->ReanimationTryToGet(aZombie->mBodyReanimID);
		int aTrackIndex = aBodyReanim ? aBodyReanim->FindTrackIndex("anim_head1") : -1;
		if (aTrackIndex >= 0)
		{
			ReanimatorTransform aTransform;
			aBodyReanim->GetCurrentTransform(aTrackIndex, &aTransform);
			theX = aZombie->mPosX + aTransform.mTransX - 9.0f;
			theY = aZombie->mPosY + aTransform.mTransY + 6.0f - aZombie->mAltitude;
		}
		else
		{
			theX = aZombie->mPosX + 20.0f;
			theY = aZombie->mPosY + 20.0f - aZombie->mAltitude;
		}
		if (aDirection > 0)
		{
			theX += 90.0f * aZombie->mScaleZombie;
		}
	}
	else if (Plant *aPlant = GetPlant())
	{
		theX = aPlant->mX + 20.0f + aDirection * 20.0f;
		theY = aPlant->mY + 10.0f;
	}

	theX += mMuzzleX * aDirection;
	theY += mMuzzleY;
}

void ShooterComponent::Fire()
{
	Board *aBoard = GetBoard();
	GameObject *anOwner = GetOwnerObject();
	int aDirection = GetDirection();
	int aTeam = GetTeam();

	float aOriginX, aOriginY;
	GetMuzzle(aOriginX, aOriginY);

	GetApp()->PlayFoley(FoleyType::FOLEY_THROW);
	int aRenderOrder = GetPlant() ? anOwner->mRenderOrder - 1 : anOwner->mRenderOrder;
	Projectile *aProjectile =
	    aBoard->AddProjectile((int)aOriginX, (int)aOriginY, aRenderOrder, GetRow(), mProjectileType);
	Teams::Set(aProjectile, aTeam);
	aProjectile->mDamageRangeFlags = mDamageRangeFlags;

	if (mArc == ShotArc::Lobbed)
	{
		Target aTarget = FindTarget(aOriginX);
		float aTargetX = aTarget.mFound ? aTarget.mX : aOriginX + aDirection * 300.0f;
		float aTargetY = aTarget.mFound ? aTarget.mY : aOriginY;

		// The arc the catapult plants and the Catapult Zombie use
		float aRangeX = std::max((aTargetX - aOriginX) * aDirection - 25.0f, 40.0f);
		aProjectile->mMotionType = ProjectileMotion::MOTION_LOBBED;
		aProjectile->mVelX = aDirection * aRangeX / 120.0f;
		aProjectile->mVelY = 0.0f;
		aProjectile->mVelZ = (aTargetY - aOriginY) / 120.0f - 7.0f;
		aProjectile->mAccZ = 0.115f;
	}
	else
	{
		aProjectile->mMotionType =
		    aDirection < 0 ? ProjectileMotion::MOTION_BACKWARDS : ProjectileMotion::MOTION_STRAIGHT;
	}
}

Reanimation *ShooterComponent::GetShootingReanim()
{
	if (PlantHeadComponent *aHead = GetSibling<PlantHeadComponent>())
		return GetApp()->ReanimationTryToGet(aHead->mHeadReanimID);
	if (Plant *aPlant = GetPlant())
		return GetApp()->ReanimationTryToGet(aPlant->mBodyReanimID);
	return nullptr;
}

void ShooterComponent::PlayAnim(bool theShooting)
{
	Reanimation *aReanim = GetShootingReanim();
	if (aReanim == nullptr)
		return;

	if (theShooting)
	{
		if (aReanim->TrackExists("anim_shooting"))
		{
			aReanim->PlayReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);
		}
	}
	else
	{
		const char *anIdle = aReanim->TrackExists("anim_head_idle") ? "anim_head_idle" : "anim_idle";
		if (aReanim->TrackExists(anIdle))
		{
			aReanim->PlayReanim(anIdle, ReanimLoopType::REANIM_LOOP, 20, 15.0f);
		}
	}
}

void ShooterComponent::OnTick()
{
	if (!CanShoot())
		return;

	// Frozen/buttered shooters pause, chilled ones shoot at half speed (like zombies walk and eat)
	GameObject *anOwner = GetOwnerObject();
	if (anOwner->IsFrozen() || anOwner->IsButtered() ||
	    (anOwner->IsChilled() && GetBoard()->mMainCounter % 2 == 1))
		return;

	if (mShotsLeft > 0)
	{
		if (--mShotTimer <= 0)
		{
			Fire();
			mShotTimer = mShotSpacing;
			if (--mShotsLeft == 0)
			{
				PlayAnim(false);
			}
		}
		return;
	}

	mCountdown--;
	if (mCountdown == mWindup)
	{
		float aOriginX, aOriginY;
		GetMuzzle(aOriginX, aOriginY);
		if (mRequiresTarget && !FindTarget(aOriginX).mFound)
		{
			// Hold here until something hostile shows up
			mCountdown++;
			return;
		}
		PlayAnim(true);
	}

	if (mCountdown <= 0)
	{
		mCountdown = std::max(mInterval, mWindup + 1);
		mShotsLeft = std::max(mShotsPerVolley, 1);
		mShotTimer = 1;
	}
}

void ShooterComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mProjectileType);
	theArchive.Value(mArc);
	theArchive.Value(mInterval);
	theArchive.Value(mShotsPerVolley);
	theArchive.Value(mShotSpacing);
	theArchive.Value(mWindup);
	theArchive.Value(mRange);
	theArchive.Value(mDamageRangeFlags);
	theArchive.Value(mRequiresTarget);
	theArchive.Value(mMuzzleX);
	theArchive.Value(mMuzzleY);
	theArchive.Value(mCountdown);
	theArchive.Value(mShotsLeft);
	theArchive.Value(mShotTimer);
}
