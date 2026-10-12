// Plant shooting, moved out of Plant.cpp (it was Plant::UpdateShooter, FindTargetAndFire, UpdateShooting, Fire,
// LaunchThreepeater, LaunchStarFruit and StarFruitFire). The per-plant differences are the data set up in
// PlantShooterComponent::CreateVanilla.

#include <cmath>
#include <cstring>
#include "PlantShooterComponent.h"
#include "Archetypes.h"
#include "Teams.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"

REGISTER_COMPONENT(PlantShooterComponent)

// ─── The vanilla shooters ────────────────────────────────────────────────────

static void SetMuzzle(PlantWeaponDef &theWeapon, MuzzleAnchor theAnchor, int theX, int theY)
{
	theWeapon.mMuzzleAnchor = theAnchor;
	theWeapon.mMuzzleX = theX;
	theWeapon.mMuzzleY = theY;
}

static void SetAnim(PlantWeaponDef &theWeapon, ShotAnimTarget theTarget, const char *theAnim, float theRate,
                    int theShootDelay)
{
	theWeapon.mAnimTarget = theTarget;
	strncpy(theWeapon.mAnim, theAnim, sizeof(theWeapon.mAnim) - 1);
	theWeapon.mAnim[sizeof(theWeapon.mAnim) - 1] = '\0';
	theWeapon.mAnimRate = theRate;
	theWeapon.mShootDelay = theShootDelay;
}

static void SetMuzzleParticle(PlantWeaponDef &theWeapon, ParticleEffect theParticle, int theX, int theY)
{
	theWeapon.mMuzzleParticle = theParticle;
	theWeapon.mMuzzleParticleX = theX;
	theWeapon.mMuzzleParticleY = theY;
}

PlantShooterComponent *PlantShooterComponent::CreateVanilla(SeedType theSeedType)
{
	auto *aShooter = new PlantShooterComponent();
	PlantWeaponDef &aMain = aShooter->mWeapons[0];
	PlantWeaponDef &aAlt = aShooter->mWeapons[1];
	auto anAddExtraLaunch = [aShooter](int theLaunchCounter, int theWeapon)
	{
		if (aShooter->mExtraLaunchCount < kMaxExtraLaunches)
		{
			aShooter->mExtraLaunches[aShooter->mExtraLaunchCount++] = { theLaunchCounter, theWeapon };
		}
	};
	auto aSetTicks = [](int32_t *theTicks, int32_t &theCount, std::initializer_list<int> theValues)
	{
		theCount = 0;
		for (int aValue : theValues)
		{
			theTicks[theCount++] = aValue;
		}
	};

	switch (theSeedType)
	{
	case SeedType::SEED_PEASHOOTER:
		SetMuzzle(aMain, MuzzleAnchor::PeaHead, 24, -33);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 33);
		break;
	case SeedType::SEED_SNOWPEA:
		aMain.mProjectile = ProjectileType::PROJECTILE_SNOWPEA;
		aMain.mExtraFoley = FoleyType::FOLEY_SNOW_PEA_SPARKLES;
		SetMuzzle(aMain, MuzzleAnchor::PeaHead, 24, -33);
		SetMuzzleParticle(aMain, ParticleEffect::PARTICLE_SNOWPEA_PUFF, 8, 13);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 33);
		break;
	case SeedType::SEED_REPEATER:
		SetMuzzle(aMain, MuzzleAnchor::PeaHead, 24, -33);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 45.0f, 26);
		anAddExtraLaunch(25, PlantWeapon::WEAPON_PRIMARY);
		break;
	case SeedType::SEED_LEFTPEATER:
		aMain.mMotion = PlantShotMotion::Backwards;
		SetMuzzle(aMain, MuzzleAnchor::PeaHeadMirrored, 27, -33);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 45.0f, 26);
		anAddExtraLaunch(25, PlantWeapon::WEAPON_PRIMARY);
		break;
	case SeedType::SEED_GATLINGPEA:
		SetMuzzle(aMain, MuzzleAnchor::PeaHead, 34, -33);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 38.0f, 100);
		aSetTicks(aShooter->mFireTicks, aShooter->mFireTickCount, { 18, 35, 51, 68 });
		break;
	case SeedType::SEED_SPLITPEA:
		aShooter->mPattern = PlantShotPattern::SplitPea;
		SetMuzzle(aMain, MuzzleAnchor::PeaHead, 24, -33);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 45.0f, 26);
		aAlt.mMotion = PlantShotMotion::Backwards;
		SetMuzzle(aAlt, MuzzleAnchor::PeaHead, -64, -33);
		SetAnim(aAlt, ShotAnimTarget::Head2, "anim_splitpea_shooting", 35.0f, 26);
		aShooter->mLaunchWeapon = PlantWeapon::WEAPON_SECONDARY;
		anAddExtraLaunch(25, PlantWeapon::WEAPON_PRIMARY);
		anAddExtraLaunch(25, PlantWeapon::WEAPON_SECONDARY);
		break;
	case SeedType::SEED_THREEPEATER:
		aShooter->mPattern = PlantShotPattern::Threepeater;
		aMain.mMotion = PlantShotMotion::ByRow;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 45, 10);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 20.0f, 35);
		break;
	case SeedType::SEED_PUFFSHROOM:
		aMain.mProjectile = ProjectileType::PROJECTILE_PUFF;
		aMain.mMotion = PlantShotMotion::Puff;
		aMain.mExtraFoley = FoleyType::FOLEY_PUFF;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 40, 40);
		SetMuzzleParticle(aMain, ParticleEffect::PARTICLE_PUFFSHROOM_MUZZLE, 18, 13);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 29);
		break;
	case SeedType::SEED_SEASHROOM:
		aMain.mProjectile = ProjectileType::PROJECTILE_PUFF;
		aMain.mMotion = PlantShotMotion::Puff;
		aMain.mExtraFoley = FoleyType::FOLEY_PUFF;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 45, 63);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 29);
		break;
	case SeedType::SEED_SCAREDYSHROOM:
		aMain.mProjectile = ProjectileType::PROJECTILE_PUFF;
		aMain.mExtraFoley = FoleyType::FOLEY_PUFF;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 29, 21);
		SetMuzzleParticle(aMain, ParticleEffect::PARTICLE_PUFFSHROOM_MUZZLE, 27, 13);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 25);
		break;
	case SeedType::SEED_FUMESHROOM:
		aMain.mProjectile = kNoProjectile;
		aMain.mAreaDamage = 20;
		aMain.mFoley = FoleyType::FOLEY_FUME;
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 50);
		aShooter->mCloudParticle = ParticleEffect::PARTICLE_FUMECLOUD;
		aShooter->mCloudX = 85;
		aShooter->mCloudY = 31;
		aSetTicks(aShooter->mCloudTicks, aShooter->mCloudTickCount, { 15 });
		break;
	case SeedType::SEED_GLOOMSHROOM:
		aMain.mProjectile = kNoProjectile;
		aMain.mAreaDamage = 20;
		aMain.mFoley = FoleyType::NUM_FOLEY;
		SetAnim(aMain, ShotAnimTarget::Body, "anim_shooting", 14.0f, 200);
		aSetTicks(aShooter->mFireTicks, aShooter->mFireTickCount, { 126, 98, 70, 42 });
		aShooter->mCloudParticle = ParticleEffect::PARTICLE_GLOOMCLOUD;
		aShooter->mCloudX = 40;
		aShooter->mCloudY = 40;
		aSetTicks(aShooter->mCloudTicks, aShooter->mCloudTickCount, { 136, 108, 80, 52 });
		break;
	case SeedType::SEED_CACTUS:
		aShooter->mPattern = PlantShotPattern::Cactus;
		aMain.mProjectile = ProjectileType::PROJECTILE_SPIKE;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 93, -50);
		SetAnim(aMain, ShotAnimTarget::Body, "anim_shootinghigh", 35.0f, 23);
		aAlt.mProjectile = ProjectileType::PROJECTILE_SPIKE;
		SetMuzzle(aAlt, MuzzleAnchor::Plant, 70, 23);
		SetAnim(aAlt, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 35);
		break;
	case SeedType::SEED_CATTAIL:
		aMain.mProjectile = ProjectileType::PROJECTILE_SPIKE;
		aMain.mMotion = PlantShotMotion::Homing;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 20, -3);
		SetAnim(aMain, ShotAnimTarget::Body, "anim_shooting", 30.0f, 50);
		aSetTicks(aShooter->mFireTicks, aShooter->mFireTickCount, { 19 });
		aShooter->mFireNeedsTarget = true;
		anAddExtraLaunch(50, PlantWeapon::WEAPON_PRIMARY);
		break;
	case SeedType::SEED_STARFRUIT:
		aShooter->mPattern = PlantShotPattern::Starfruit;
		aMain.mProjectile = ProjectileType::PROJECTILE_STAR;
		aMain.mMotion = PlantShotMotion::Star;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 25, 25);
		SetAnim(aMain, ShotAnimTarget::Body, "anim_shoot", 28.0f, 40);
		break;
	case SeedType::SEED_CABBAGEPULT:
		aShooter->mPattern = PlantShotPattern::Catapult;
		aMain.mProjectile = ProjectileType::PROJECTILE_CABBAGE;
		aMain.mMotion = PlantShotMotion::Lobbed;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 5, -12);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 32);
		break;
	case SeedType::SEED_KERNELPULT:
		aShooter->mPattern = PlantShotPattern::Catapult;
		aMain.mProjectile = ProjectileType::PROJECTILE_KERNEL;
		aMain.mMotion = PlantShotMotion::Lobbed;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 19, -37);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 30);
		aAlt = aMain;
		aAlt.mProjectile = ProjectileType::PROJECTILE_BUTTER;
		SetMuzzle(aAlt, MuzzleAnchor::Plant, 12, -56);
		aShooter->mAltWeaponChance = 4;
		strcpy(aShooter->mAltWeaponPrefix, "Cornpult_butter");
		strcpy(aShooter->mMainWeaponPrefix, "Cornpult_kernal");
		break;
	case SeedType::SEED_MELONPULT:
		aShooter->mPattern = PlantShotPattern::Catapult;
		aMain.mProjectile = ProjectileType::PROJECTILE_MELON;
		aMain.mMotion = PlantShotMotion::Lobbed;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 25, -46);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 36);
		break;
	case SeedType::SEED_WINTERMELON:
		aShooter->mPattern = PlantShotPattern::Catapult;
		aMain.mProjectile = ProjectileType::PROJECTILE_WINTERMELON;
		aMain.mMotion = PlantShotMotion::Lobbed;
		aMain.mExtraFoley = FoleyType::FOLEY_SNOW_PEA_SPARKLES;
		SetMuzzle(aMain, MuzzleAnchor::Plant, 25, -46);
		SetAnim(aMain, ShotAnimTarget::Auto, "anim_shooting", 35.0f, 36);
		break;
	case SeedType::SEED_COBCANNON:
		aShooter->mPattern = PlantShotPattern::CobCannon;
		aShooter->mAutoLaunch = false;
		aMain.mProjectile = ProjectileType::PROJECTILE_COBBIG;
		aMain.mMotion = PlantShotMotion::Cob;
		SetMuzzle(aMain, MuzzleAnchor::Plant, -44, -184);
		break;
	default:
		delete aShooter;
		return nullptr;
	}

	// The generic ShooterComponent fields describe the main weapon, for code that looks at any shooter
	aShooter->mProjectileType = aMain.mProjectile;
	aShooter->mArc = aMain.mMotion == PlantShotMotion::Lobbed ? ShotArc::Lobbed : ShotArc::Straight;
	return aShooter;
}

const PlantWeaponDef &PlantShooterComponent::GetWeapon(PlantWeapon theWeapon) const
{
	return mWeapons[theWeapon == PlantWeapon::WEAPON_SECONDARY ? 1 : 0];
}

// ─── Launching (was Plant::UpdateShooter) ────────────────────────────────────

void PlantShooterComponent::UpdateLaunch(Plant *thePlant)
{
	thePlant->mLaunchCounter--;
	if (thePlant->mLaunchCounter <= 0)
	{
		thePlant->mLaunchCounter = thePlant->mLaunchRate - Sexy::Rand(15);

		switch (mPattern)
		{
		case PlantShotPattern::Threepeater:
			LaunchThreepeater(thePlant);
			break;
		case PlantShotPattern::Starfruit:
			if (thePlant->FindStarFruitTarget())
			{
				PlayShootAnim(thePlant, mWeapons[0]);
			}
			break;
		case PlantShotPattern::Cactus:
			if (thePlant->mState == PlantState::STATE_CACTUS_HIGH)
			{
				FindTargetAndFire(thePlant, thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
			}
			else if (thePlant->mState == PlantState::STATE_CACTUS_LOW)
			{
				FindTargetAndFire(thePlant, thePlant->mRow, PlantWeapon::WEAPON_SECONDARY);
			}
			break;
		default:
			FindTargetAndFire(thePlant, thePlant->mRow, (PlantWeapon)mLaunchWeapon);
			break;
		}
	}

	for (int i = 0; i < mExtraLaunchCount; i++)
	{
		if (thePlant->mLaunchCounter == mExtraLaunches[i].mLaunchCounter)
		{
			FindTargetAndFire(thePlant, thePlant->mRow, (PlantWeapon)mExtraLaunches[i].mWeapon);
		}
	}
}

void PlantShooterComponent::LaunchThreepeater(Plant *thePlant)
{
	Board *aBoard = GetBoard();
	int aRowAbove = thePlant->mRow - 1;
	int aRowBelow = thePlant->mRow + 1;

	if (thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY) ||
	    (aBoard->RowCanHaveZombies(aRowAbove) && thePlant->FindTargetZombie(aRowAbove, PlantWeapon::WEAPON_PRIMARY)) ||
	    (aBoard->RowCanHaveZombies(aRowBelow) && thePlant->FindTargetZombie(aRowBelow, PlantWeapon::WEAPON_PRIMARY)))
	{
		LawnApp *anApp = GetApp();
		Reanimation *aHeads[3] = { anApp->ReanimationTryToGet(thePlant->mHeadReanimID),
		                           anApp->ReanimationTryToGet(thePlant->mHeadReanimID2),
		                           anApp->ReanimationTryToGet(thePlant->mHeadReanimID3) };
		// Head 1 aims at the row below, head 3 at the row above
		bool aRowExists[3] = { aBoard->RowCanHaveZombies(aRowBelow), true, aBoard->RowCanHaveZombies(aRowAbove) };
		const char *aAnims[3] = { "anim_shooting1", "anim_shooting2", "anim_shooting3" };
		for (int i = 0; i < 3; i++)
		{
			if (aHeads[i] && aRowExists[i])
			{
				aHeads[i]->StartBlend(10);
				aHeads[i]->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
				aHeads[i]->mAnimRate = mWeapons[0].mAnimRate;
				aHeads[i]->SetFramesForLayer(aAnims[i]);
			}
		}

		thePlant->mShootingCounter = mWeapons[0].mShootDelay;
	}
}

// ─── Aiming (was Plant::FindTargetAndFire) ───────────────────────────────────

bool PlantShooterComponent::FindTargetAndFire(Plant *thePlant, int theRow, PlantWeapon theWeapon)
{
	Zombie *aZombie = thePlant->FindTargetZombie(theRow, theWeapon);
	if (aZombie == nullptr && thePlant->FindTargetPlant(theRow, theWeapon) == nullptr)
		return false;

	thePlant->EndBlink();
	const PlantWeaponDef &aWeapon = GetWeapon(theWeapon);
	if (!PlayShootAnim(thePlant, aWeapon))
	{
		// No shooting animation: fire right away
		Fire(thePlant, aZombie, theRow, theWeapon);
	}
	return true;
}

bool PlantShooterComponent::PlayShootAnim(Plant *thePlant, const PlantWeaponDef &theWeapon)
{
	LawnApp *anApp = GetApp();
	Reanimation *aBodyReanim = anApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	Reanimation *aHeadReanim = anApp->ReanimationTryToGet(thePlant->mHeadReanimID);
	bool isPlaying = false;

	auto aPlayOnHead = [&](Reanimation *theHead)
	{
		theHead->StartBlend(20);
		theHead->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
		theHead->mAnimRate = theWeapon.mAnimRate;
		theHead->SetFramesForLayer(theWeapon.mAnim);
		isPlaying = true;
	};

	if (theWeapon.mAnimTarget == ShotAnimTarget::Head2)
	{
		if (Reanimation *aHead2 = anApp->ReanimationTryToGet(thePlant->mHeadReanimID2))
		{
			aPlayOnHead(aHead2);
		}
	}
	else if (theWeapon.mAnimTarget == ShotAnimTarget::Auto && aHeadReanim && aHeadReanim->TrackExists(theWeapon.mAnim))
	{
		aPlayOnHead(aHeadReanim);
	}
	else if (aBodyReanim &&
	         (theWeapon.mAnimTarget == ShotAnimTarget::Body || aBodyReanim->TrackExists(theWeapon.mAnim)))
	{
		thePlant->PlayBodyReanim(theWeapon.mAnim, ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, theWeapon.mAnimRate);
		isPlaying = true;

		// Catapult: some throws use the second weapon (Kernel-pult swaps its kernel for butter)
		if (mPattern == PlantShotPattern::Catapult && mAltWeaponChance > 0 && Sexy::Rand(mAltWeaponChance) == 0)
		{
			aBodyReanim->AssignRenderGroupToPrefix(mAltWeaponPrefix, RENDER_GROUP_NORMAL);
			aBodyReanim->AssignRenderGroupToPrefix(mMainWeaponPrefix, RENDER_GROUP_HIDDEN);
			thePlant->mState = PlantState::STATE_KERNELPULT_BUTTER;
		}
	}

	if (isPlaying)
	{
		thePlant->mShootingCounter = theWeapon.mShootDelay;
	}
	return isPlaying;
}

// ─── The shooting animation (was Plant::UpdateShooting) ──────────────────────

void PlantShooterComponent::UpdateShooting(Plant *thePlant)
{
	if (thePlant->NotOnGround() || thePlant->mShootingCounter == 0)
		return;

	thePlant->mShootingCounter--;
	int aCounter = thePlant->mShootingCounter;

	for (int i = 0; i < mCloudTickCount; i++)
	{
		if (aCounter == mCloudTicks[i])
		{
			int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, thePlant->mRow, 0);
			thePlant->AddAttachedParticle(thePlant->mX + mCloudX, thePlant->mY + mCloudY, aRenderPosition,
			                              mCloudParticle);
		}
	}

	if (mFireTickCount > 0)
	{
		for (int i = 0; i < mFireTickCount; i++)
		{
			if (aCounter != mFireTicks[i])
				continue;

			if (mFireNeedsTarget)
			{
				Zombie *aZombie = thePlant->FindTargetZombie(thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
				if (aZombie)
				{
					Fire(thePlant, aZombie, thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
				}
			}
			else
			{
				Fire(thePlant, nullptr, thePlant->mRow, PlantWeapon::WEAPON_PRIMARY);
			}
		}
	}
	else if (aCounter == 1)
	{
		FireAtAnimEnd(thePlant);
		return;
	}

	if (aCounter != 0)
		return;

	RestoreIdle(thePlant);
}

void PlantShooterComponent::FireAtAnimEnd(Plant *thePlant)
{
	LawnApp *anApp = GetApp();
	int aRow = thePlant->mRow;

	if (mPattern == PlantShotPattern::Threepeater)
	{
		// Each head that played its shooting animation fires into its row
		ReanimationID aHeadIDs[3] = { thePlant->mHeadReanimID, thePlant->mHeadReanimID2, thePlant->mHeadReanimID3 };
		int aRows[3] = { aRow + 1, aRow, aRow - 1 };
		for (int i = 0; i < 3; i++)
		{
			Reanimation *aHead = anApp->ReanimationTryToGet(aHeadIDs[i]);
			if (aHead && aHead->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
			{
				Fire(thePlant, nullptr, aRows[i], PlantWeapon::WEAPON_PRIMARY);
			}
		}
	}
	else if (mPattern == PlantShotPattern::SplitPea)
	{
		Reanimation *aHeadFront = anApp->ReanimationTryToGet(thePlant->mHeadReanimID);
		Reanimation *aHeadBack = anApp->ReanimationTryToGet(thePlant->mHeadReanimID2);
		if (aHeadFront && aHeadFront->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD &&
		    thePlant->mLaunchCounter <= 1)
		{
			Fire(thePlant, nullptr, aRow, PlantWeapon::WEAPON_PRIMARY);
		}
		if (aHeadBack && aHeadBack->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
		{
			Fire(thePlant, nullptr, aRow, PlantWeapon::WEAPON_SECONDARY);
		}
	}
	else if (thePlant->mState == PlantState::STATE_CACTUS_LOW)
	{
		Fire(thePlant, nullptr, aRow, PlantWeapon::WEAPON_SECONDARY);
	}
	else if (mPattern == PlantShotPattern::Catapult)
	{
		PlantWeapon aWeapon = PlantWeapon::WEAPON_PRIMARY;
		if (thePlant->mState == PlantState::STATE_KERNELPULT_BUTTER)
		{
			Reanimation *aBodyReanim = anApp->ReanimationGet(thePlant->mBodyReanimID);
			aBodyReanim->AssignRenderGroupToPrefix(mAltWeaponPrefix, RENDER_GROUP_HIDDEN);
			aBodyReanim->AssignRenderGroupToPrefix(mMainWeaponPrefix, RENDER_GROUP_NORMAL);
			thePlant->mState = PlantState::STATE_NOTREADY;
			aWeapon = PlantWeapon::WEAPON_SECONDARY;
		}

		Zombie *aZombie = thePlant->FindTargetZombie(aRow, aWeapon);
		Fire(thePlant, aZombie, aRow, aWeapon);
	}
	else
	{
		Fire(thePlant, nullptr, aRow, PlantWeapon::WEAPON_PRIMARY);
	}
}

void PlantShooterComponent::RestoreIdle(Plant *thePlant)
{
	LawnApp *anApp = GetApp();
	Reanimation *aBodyReanim = anApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	Reanimation *aHeadReanim = anApp->ReanimationTryToGet(thePlant->mHeadReanimID);
	auto aBackToIdle = [aBodyReanim](Reanimation *theHead, const char *theIdle)
	{
		theHead->StartBlend(20);
		theHead->mLoopType = ReanimLoopType::REANIM_LOOP;
		theHead->SetFramesForLayer(theIdle);
		theHead->mAnimRate = aBodyReanim->mAnimRate;
		theHead->mAnimTime = aBodyReanim->mAnimTime;
	};

	if (mPattern == PlantShotPattern::Threepeater)
	{
		Reanimation *aHeadReanim2 = anApp->ReanimationTryToGet(thePlant->mHeadReanimID2);
		Reanimation *aHeadReanim3 = anApp->ReanimationTryToGet(thePlant->mHeadReanimID3);
		if (aBodyReanim && aHeadReanim && aHeadReanim2 && aHeadReanim3 && aHeadReanim2->mLoopCount > 0)
		{
			if (aHeadReanim->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
			{
				aBackToIdle(aHeadReanim, "anim_head_idle1");
			}
			aBackToIdle(aHeadReanim2, "anim_head_idle2");
			if (aHeadReanim3->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
			{
				aBackToIdle(aHeadReanim3, "anim_head_idle3");
			}
			return;
		}
	}
	else if (mPattern == PlantShotPattern::SplitPea)
	{
		Reanimation *aHeadReanim2 = anApp->ReanimationTryToGet(thePlant->mHeadReanimID2);
		if (aBodyReanim && aHeadReanim && aHeadReanim->mLoopCount > 0)
		{
			aBackToIdle(aHeadReanim, "anim_head_idle");
		}
		if (aBodyReanim && aHeadReanim2 && aHeadReanim2->mLoopCount > 0)
		{
			aBackToIdle(aHeadReanim2, "anim_splitpea_idle");
		}
		return;
	}
	else if (thePlant->mState == PlantState::STATE_CACTUS_HIGH)
	{
		if (aBodyReanim && aBodyReanim->mLoopCount > 0)
		{
			thePlant->PlayBodyReanim("anim_idlehigh", ReanimLoopType::REANIM_LOOP, 20, 0.0f);
			aBodyReanim->mAnimRate = aBodyReanim->mDefinition->mFPS;
			if (anApp->IsIZombieLevel())
			{
				aBodyReanim->mAnimRate = 0.0f;
			}
			return;
		}
	}
	else if (aHeadReanim)
	{
		if (aBodyReanim && aHeadReanim->mLoopCount > 0)
		{
			aBackToIdle(aHeadReanim, "anim_head_idle");
			return;
		}
	}
	else if (mPattern == PlantShotPattern::CobCannon)
	{
		if (aBodyReanim && aBodyReanim->mLoopCount > 0)
		{
			thePlant->mState = PlantState::STATE_COBCANNON_ARMING;
			thePlant->mStateCountdown = 3000;
			thePlant->PlayBodyReanim("anim_unarmed_idle", ReanimLoopType::REANIM_LOOP, 20,
			                         aBodyReanim->mDefinition->mFPS);
			return;
		}
	}
	else if (aBodyReanim && aBodyReanim->mLoopCount > 0)
	{
		thePlant->PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
		return;
	}

	// The animation hasn't finished yet: check again next tick
	thePlant->mShootingCounter = 1;
}

// ─── Firing (was Plant::Fire) ────────────────────────────────────────────────

void PlantShooterComponent::Fire(Plant *thePlant, Zombie *theTargetZombie, int theRow, PlantWeapon theWeapon)
{
	LawnApp *anApp = GetApp();
	Board *aBoard = GetBoard();
	const PlantWeaponDef &aWeapon = GetWeapon(theWeapon);

	if (aWeapon.mProjectile == kNoProjectile)
	{
		// Fume/Gloom-shroom: the fumes hit everything in range directly
		if (aWeapon.mAreaDamage > 0)
		{
			thePlant->DoRowAreaDamage(aWeapon.mAreaDamage, GetBit(DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY));
		}
		if (aWeapon.mFoley != FoleyType::NUM_FOLEY)
		{
			anApp->PlayFoley(aWeapon.mFoley);
		}
		return;
	}

	if (aWeapon.mMotion == PlantShotMotion::Star)
	{
		FireStars(thePlant);
		return;
	}

	if (aWeapon.mFoley != FoleyType::NUM_FOLEY)
	{
		anApp->PlayFoley(aWeapon.mFoley);
	}
	if (aWeapon.mExtraFoley != FoleyType::NUM_FOLEY)
	{
		anApp->PlayFoley(aWeapon.mExtraFoley);
	}

	int aOriginX = thePlant->mX + aWeapon.mMuzzleX;
	int aOriginY = thePlant->mY + aWeapon.mMuzzleY;
	if (aWeapon.mMuzzleAnchor != MuzzleAnchor::Plant)
	{
		int aHeadX, aHeadY;
		thePlant->GetPeaHeadOffset(aHeadX, aHeadY);
		aOriginX += aWeapon.mMuzzleAnchor == MuzzleAnchor::PeaHeadMirrored ? -aHeadX : aHeadX;
		aOriginY += aHeadY;
	}
	if (aBoard->GetFlowerPotAt(thePlant->mPlantCol, thePlant->mRow))
	{
		aOriginY -= 5;
	}

	if (aWeapon.mMuzzleParticle != ParticleEffect::PARTICLE_NONE)
	{
		int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN_MOWER, thePlant->mRow, 1);
		anApp->AddTodParticle(aOriginX + aWeapon.mMuzzleParticleX, aOriginY + aWeapon.mMuzzleParticleY,
		                      aRenderPosition, aWeapon.mMuzzleParticle);
	}

	// A hypnotized plant fires the other way: mirror the muzzle around the plant's center
	int aDirection = Teams::IsMirrored(thePlant) ? -1 : 1;
	if (aDirection < 0)
	{
		aOriginX = 2 * (thePlant->mX + thePlant->mWidth / 2) - aOriginX - 30;
	}

	Projectile *aProjectile =
	    aBoard->AddProjectile(aOriginX, aOriginY, thePlant->mRenderOrder - 1, theRow, aWeapon.mProjectile);
	aProjectile->mDamageRangeFlags = thePlant->GetDamageRangeFlags(theWeapon);
	Teams::Set(aProjectile, Teams::Of(thePlant));

	switch (aWeapon.mMotion)
	{
	case PlantShotMotion::Lobbed:
	{
		float aRangeX, aRangeY;
		if (theTargetZombie)
		{
			Rect aZombieRect = theTargetZombie->GetZombieRect();
			aRangeX = (theTargetZombie->ZombieTargetLeadX(50.0f) - aOriginX) * aDirection - 30.0f;
			aRangeY = aZombieRect.mY - aOriginY;

			if (theTargetZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
			{
				aRangeX -= 60.0f;
			}
			if (theTargetZombie->mZombieType == ZombieType::ZOMBIE_POGO && theTargetZombie->mHasObject)
			{
				aRangeX -= 60.0f;
			}
			if (theTargetZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL)
			{
				aRangeX -= 40.0f;
			}
			if (theTargetZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
			{
				aRangeY = aBoard->GridToPixelY(8, thePlant->mRow) - aOriginY;
			}
		}
		else if (Plant *aTargetPlant = thePlant->FindTargetPlant(theRow, theWeapon))
		{
			// No zombie to hit: lob at a hostile plant (hypnotized plants, or this plant is the hypnotized one)
			aRangeX = (aTargetPlant->mX + aTargetPlant->mWidth / 2 - aOriginX) * aDirection - 30.0f;
			aRangeY = aTargetPlant->mY - aOriginY;
		}
		else
		{
			aRangeX = aDirection > 0 ? 700.0f - aOriginX : aOriginX - 100.0f;
			aRangeY = 0.0f;
		}
		if (aRangeX < 40.0f)
		{
			aRangeX = 40.0f;
		}

		aProjectile->mMotionType = ProjectileMotion::MOTION_LOBBED;
		aProjectile->mVelX = aDirection * aRangeX / 120.0f;
		aProjectile->mVelY = 0.0f;
		aProjectile->mVelZ = aRangeY / 120.0f - 7.0f;
		aProjectile->mAccZ = 0.115f;
		break;
	}
	case PlantShotMotion::ByRow:
		if (theRow < thePlant->mRow)
		{
			aProjectile->mMotionType = ProjectileMotion::MOTION_THREEPEATER;
			aProjectile->mVelY = -3.0f;
			aProjectile->mShadowY += 80.0f;
		}
		else if (theRow > thePlant->mRow)
		{
			aProjectile->mMotionType = ProjectileMotion::MOTION_THREEPEATER;
			aProjectile->mVelY = 3.0f;
			aProjectile->mShadowY -= 80.0f;
		}
		break;
	case PlantShotMotion::Puff:
		aProjectile->mMotionType = ProjectileMotion::MOTION_PUFF;
		break;
	case PlantShotMotion::Backwards:
		aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
		break;
	case PlantShotMotion::Homing:
		aProjectile->mVelX = 2.0f;
		aProjectile->mMotionType = ProjectileMotion::MOTION_HOMING;
		aProjectile->mTargetZombieID =
		    theTargetZombie ? aBoard->ZombieGetID(theTargetZombie) : ZombieID::ZOMBIEID_NULL;
		break;
	case PlantShotMotion::Cob:
		aProjectile->mVelX = 0.001f;
		aProjectile->mDamageRangeFlags = thePlant->GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
		aProjectile->mMotionType = ProjectileMotion::MOTION_LOBBED;
		aProjectile->mVelY = 0.0f;
		aProjectile->mAccZ = 0.0f;
		aProjectile->mVelZ = -8.0f;
		aProjectile->mCobTargetX = thePlant->mTargetX - 40;
		aProjectile->mCobTargetRow = aBoard->PixelToGridYKeepOnBoard(thePlant->mTargetX, thePlant->mTargetY);
		break;
	default:
		break;
	}

	if (aDirection < 0)
	{
		if (aProjectile->mMotionType == ProjectileMotion::MOTION_STRAIGHT ||
		    aProjectile->mMotionType == ProjectileMotion::MOTION_PUFF)
		{
			aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
		}
		else if (aProjectile->mMotionType == ProjectileMotion::MOTION_BACKWARDS)
		{
			aProjectile->mMotionType = ProjectileMotion::MOTION_STRAIGHT;
		}
	}
}

void PlantShooterComponent::FireStars(Plant *thePlant)
{
	const PlantWeaponDef &aWeapon = mWeapons[0];
	GetApp()->PlayFoley(aWeapon.mFoley != FoleyType::NUM_FOLEY ? aWeapon.mFoley : FoleyType::FOLEY_THROW);

	// Back, down, up and two diagonals forward
	float aDiagonalX = cos(DEG_TO_RAD(30.0f)) * 3.33f;
	float aDiagonalY = sin(DEG_TO_RAD(30.0f)) * 3.33f;
	const float aVelocities[5][2] = {
		{ -3.33f, 0.0f }, { 0.0f, 3.33f }, { 0.0f, -3.33f }, { aDiagonalX, aDiagonalY }, { aDiagonalX, -aDiagonalY }
	};
	for (const auto &aVelocity : aVelocities)
	{
		Projectile *aProjectile =
		    GetBoard()->AddProjectile(thePlant->mX + aWeapon.mMuzzleX, thePlant->mY + aWeapon.mMuzzleY,
		                              thePlant->mRenderOrder - 1, thePlant->mRow, aWeapon.mProjectile);
		aProjectile->mDamageRangeFlags = thePlant->GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
		aProjectile->mMotionType = ProjectileMotion::MOTION_STAR;
		aProjectile->mVelX = aVelocity[0];
		aProjectile->mVelY = aVelocity[1];
		Teams::Set(aProjectile, Teams::Of(thePlant));
	}
}

void PlantShooterComponent::Serialize(ComponentArchive &theArchive)
{
	ShooterComponent::Serialize(theArchive);
	theArchive.Value(mPattern);
	theArchive.Value(mWeapons);
	theArchive.Value(mAutoLaunch);
	theArchive.Value(mLaunchWeapon);
	theArchive.Value(mExtraLaunches);
	theArchive.Value(mExtraLaunchCount);
	theArchive.Value(mFireTicks);
	theArchive.Value(mFireTickCount);
	theArchive.Value(mFireNeedsTarget);
	theArchive.Value(mCloudParticle);
	theArchive.Value(mCloudX);
	theArchive.Value(mCloudY);
	theArchive.Value(mCloudTicks);
	theArchive.Value(mCloudTickCount);
	theArchive.Value(mAltWeaponChance);
	theArchive.Value(mAltWeaponPrefix);
	theArchive.Value(mMainWeaponPrefix);
}
