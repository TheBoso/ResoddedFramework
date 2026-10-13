#include "PlantBehaviours.h"
#include "Teams.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../System/Achievements.h"
#include "../../Sexy.TodLib/Reanimator.h"

REGISTER_COMPONENT(FuseComponent)
REGISTER_COMPONENT(DoomFuseComponent)
REGISTER_COMPONENT(ExplodeComponent)
REGISTER_COMPONENT(BurnRowComponent)
REGISTER_COMPONENT(FreezeAllComponent)
REGISTER_COMPONENT(SquashBehaviour)
REGISTER_COMPONENT(ChomperBehaviour)
REGISTER_COMPONENT(FlowerPotBehaviour)
REGISTER_COMPONENT(LilyPadBehaviour)
REGISTER_COMPONENT(ImitaterBehaviour)
REGISTER_COMPONENT(CobCannonBehaviour)
REGISTER_COMPONENT(CactusBehaviour)
REGISTER_COMPONENT(MagnetShroomBehaviour)
REGISTER_COMPONENT(GoldMagnetBehaviour)
REGISTER_COMPONENT(SunShroomGrowthBehaviour)
REGISTER_COMPONENT(GraveBusterBehaviour)
REGISTER_COMPONENT(PotatoMineArmingBehaviour)
REGISTER_COMPONENT(SpikeweedBehaviour)
REGISTER_COMPONENT(TangleKelpBehaviour)
REGISTER_COMPONENT(ScaredyShroomBehaviour)
REGISTER_COMPONENT(BloverBehaviour)
REGISTER_COMPONENT(UmbrellaBehaviour)
REGISTER_COMPONENT(CoffeeBeanBehaviour)

// ─── FuseComponent ───────────────────────────────────────────────────────────

void FuseComponent::UpdateBehaviour(Plant *thePlant)
{
	if (thePlant->mIsAsleep || thePlant->mState == PlantState::STATE_DOINGSPECIAL)
		return;

	thePlant->mState = PlantState::STATE_DOINGSPECIAL;
	thePlant->mDoSpecialCountdown = mFuseTicks;
	OnLit(thePlant);
}

void FuseComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("fuseTicks", mFuseTicks);
}

void DoomFuseComponent::OnLit(Plant *thePlant)
{
	Reanimation *aBodyReanim = GetApp()->ReanimationTryToGet(thePlant->mBodyReanimID);
	if (aBodyReanim && aBodyReanim->TrackExists("anim_explode"))
	{
		aBodyReanim->SetFramesForLayer("anim_explode");
		aBodyReanim->mAnimRate = 23.0f;
		aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
		aBodyReanim->SetShakeOverride("DoomShroom_head1", 1.0f);
		aBodyReanim->SetShakeOverride("DoomShroom_head2", 2.0f);
		aBodyReanim->SetShakeOverride("DoomShroom_head3", 2.0f);
	}
	GetApp()->PlayFoley(FoleyType::FOLEY_REVERSE_EXPLOSION);
}

// ─── ExplodeComponent ────────────────────────────────────────────────────────

ExplodeComponent *ExplodeComponent::CherryBomb()
{
	auto *anExplode = new ExplodeComponent();
	anExplode->mAchievement = AchievementID::ACHIEVEMENT_EXPLODONATOR;
	anExplode->mAchievementMinZombies = 10;
	anExplode->mAchievementRadius = 250;
	anExplode->mAchievementRowRange = 3;
	return anExplode;
}

ExplodeComponent *ExplodeComponent::DoomShroom()
{
	auto *anExplode = new ExplodeComponent();
	anExplode->mRadius = 250;
	anExplode->mRowRange = 3;
	anExplode->mFoley1 = FoleyType::NUM_FOLEY;
	anExplode->mFoley2 = FoleyType::NUM_FOLEY;
	anExplode->mSoundId = ResourceId::SOUND_DOOMSHROOM_ID;
	anExplode->mParticle = ParticleEffect::PARTICLE_DOOM;
	anExplode->mCraterTicks = 18000;
	anExplode->mKillsNearbyPlants = true;
	return anExplode;
}

ExplodeComponent *ExplodeComponent::PotatoMine()
{
	auto *anExplode = new ExplodeComponent();
	anExplode->mRadius = 60;
	anExplode->mRowRange = 0;
	anExplode->mBurns = false;
	anExplode->mCenterOffsetX = -20;
	anExplode->mFoley1 = FoleyType::NUM_FOLEY;
	anExplode->mFoley2 = FoleyType::NUM_FOLEY;
	anExplode->mSoundId = ResourceId::SOUND_POTATO_MINE_ID;
	anExplode->mParticle = ParticleEffect::PARTICLE_POTATO_MINE;
	anExplode->mParticleInRow = true;
	anExplode->mAchievement = AchievementID::ACHIEVEMENT_SPUDOW;
	anExplode->mAchievementInIZombie = false;
	return anExplode;
}

void ExplodeComponent::DoSpecial(Plant *thePlant)
{
	LawnApp *anApp = GetApp();
	Board *aBoard = GetBoard();
	int aCenterX = thePlant->mX + thePlant->mWidth / 2;
	int aCenterY = thePlant->mY + thePlant->mHeight / 2;
	int aBlastX = aCenterX + mCenterOffsetX;
	int aDamageRangeFlags = thePlant->GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
	int aTeam = Teams::Of(thePlant);

	if (mFoley1 != FoleyType::NUM_FOLEY)
		anApp->PlayFoley(mFoley1);
	if (mFoley2 != FoleyType::NUM_FOLEY)
		anApp->PlayFoley(mFoley2);
	if (mSoundId >= 0)
		anApp->PlaySample(GetSoundById(mSoundId));

	if (mAchievement >= 0 && (mAchievementInIZombie || !anApp->IsIZombieLevel()))
	{
		bool isEarned = mAchievementMinZombies < 0;
		if (!isEarned)
		{
			std::vector<Zombie *> aZombies = aBoard->GetZombiesInRadius(
			    aTeam, thePlant->mRow, aBlastX, aCenterY, mAchievementRadius, mAchievementRowRange, aDamageRangeFlags);
			isEarned = (int)aZombies.size() > mAchievementMinZombies;
		}
		if (isEarned)
		{
			anApp->mAchievements->GiveAchievement((AchievementID)mAchievement);
		}
	}

	// Hits whatever is on the other side: zombies, or your plants if this plant was hypnotized
	aBoard->KillAllZombiesInRadius(aTeam, thePlant->mRow, aBlastX, aCenterY, mRadius, mRowRange, mBurns,
	                               aDamageRangeFlags);
	aBoard->KillAllPlantsInRadius(aTeam, aBlastX, aCenterY, mRadius);
	if (mKillsNearbyPlants)
	{
		thePlant->KillAllPlantsNearDoom();
	}

	int aRenderOrder = mParticleInRow ? Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, thePlant->mRow, 0)
	                                  : (int)RenderLayer::RENDER_LAYER_TOP;
	anApp->AddTodParticle(aCenterX, aCenterY, aRenderOrder, mParticle);
	if (mCraterTicks > 0)
	{
		aBoard->AddACrater(thePlant->mPlantCol, thePlant->mRow)->mGridItemCounter = mCraterTicks;
	}
	aBoard->ShakeBoard(3, -4);

	thePlant->Die();
}

void ExplodeComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("radius", mRadius);
	theArchive.Value("rowRange", mRowRange);
	theArchive.Value("burns", mBurns);
	theArchive.Value("centerOffsetX", mCenterOffsetX);
	theArchive.Value("foley1", mFoley1);
	theArchive.Value("foley2", mFoley2);
	theArchive.Value("soundId", mSoundId);
	theArchive.Value("particle", mParticle);
	theArchive.Value("particleInRow", mParticleInRow);
	theArchive.Value("craterTicks", mCraterTicks);
	theArchive.Value("killsNearbyPlants", mKillsNearbyPlants);
	theArchive.Value("achievement", mAchievement);
	theArchive.Value("achievementMinZombies", mAchievementMinZombies);
	theArchive.Value("achievementRadius", mAchievementRadius);
	theArchive.Value("achievementRowRange", mAchievementRowRange);
	theArchive.Value("achievementInIZombie", mAchievementInIZombie);
}

// ─── BurnRowComponent ────────────────────────────────────────────────────────

void BurnRowComponent::DoSpecial(Plant *thePlant)
{
	LawnApp *anApp = GetApp();
	Board *aBoard = GetBoard();
	anApp->PlayFoley(FoleyType::FOLEY_JALAPENO_IGNITE);
	anApp->PlayFoley(FoleyType::FOLEY_JUICY);

	for (int aRow = thePlant->mRow - mRowSpread; aRow <= thePlant->mRow + mRowSpread; aRow++)
	{
		if (aRow < 0 || aRow >= MAX_GRID_SIZE_Y)
			continue;

		aBoard->DoFwoosh(aRow);
		if (aRow == thePlant->mRow)
		{
			aBoard->ShakeBoard(3, -4);
		}
		thePlant->BurnRow(aRow);

		// Hostile plants burn too (when this plant was hypnotized), like the Jalapeno Zombie's fire
		Plant *aPlant = nullptr;
		while (aBoard->IteratePlants(aPlant))
		{
			if (aPlant->mRow == aRow && !aPlant->NotOnGround() && Teams::CanAttack(Teams::Of(thePlant), aPlant))
			{
				aBoard->mPlantsEaten++;
				aPlant->Die();
			}
		}
		// Melts the ice the Zamboni left behind
		aBoard->mIceTimer[aRow] = 20;
	}

	thePlant->Die();
}

void BurnRowComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("rowSpread", mRowSpread);
}

// ─── FreezeAllComponent ──────────────────────────────────────────────────────

void FreezeAllComponent::DoSpecial(Plant *thePlant)
{
	GetApp()->PlayFoley(FoleyType::FOLEY_FROZEN);
	thePlant->IceZombies();
	GetApp()->AddTodParticle(thePlant->mX + thePlant->mWidth / 2, thePlant->mY + thePlant->mHeight / 2,
	                         (int)RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_ICE_TRAP);
	thePlant->Die();
}
