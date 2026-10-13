#include "ArmorComponents.h"
#include "ComponentRegistry.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodParticle.h"

REGISTER_COMPONENT(HelmComponent)
REGISTER_COMPONENT(ShieldComponent)
REGISTER_COMPONENT(FlyingComponent)

// ─── Looks of the vanilla armour ─────────────────────────────────────────────

struct HelmLook
{
	HelmType mType;
	/// Reanimation track the helm is drawn on (and gets its damaged images on)
	const char *mTrack;
	/// mTrack is a prefix (all tracks starting with it) rather than one track
	bool mTrackIsPrefix;
	/// The helm is part of a plant head (Wall-nut/Tall-nut zombies): damaged images go on that animation,
	/// and nothing falls off when it breaks
	bool mOnPlantHead;
	/// Show the zombie's hair once the helm is gone
	bool mShowsHair;
	ParticleEffect mDropParticle;
	Image **mDamagedImage1;
	Image **mDamagedImage2;
};

static const HelmLook gHelmLooks[] = {
	{ HelmType::HELMTYPE_TRAFFIC_CONE, "anim_cone", true, false, true, ParticleEffect::PARTICLE_ZOMBIE_TRAFFIC_CONE,
	  &IMAGE_REANIM_ZOMBIE_CONE2, &IMAGE_REANIM_ZOMBIE_CONE3 },
	{ HelmType::HELMTYPE_PAIL, "anim_bucket", true, false, true, ParticleEffect::PARTICLE_ZOMBIE_PAIL,
	  &IMAGE_REANIM_ZOMBIE_BUCKET2, &IMAGE_REANIM_ZOMBIE_BUCKET3 },
	{ HelmType::HELMTYPE_FOOTBALL, "zombie_football_helmet", true, false, true, ParticleEffect::PARTICLE_ZOMBIE_HELMET,
	  &IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2, &IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3 },
	{ HelmType::HELMTYPE_DIGGER, "Zombie_digger_hardhat", false, false, false, ParticleEffect::PARTICLE_ZOMBIE_HEADLIGHT,
	  &IMAGE_REANIM_ZOMBIE_DIGGER_HARDHAT2, &IMAGE_REANIM_ZOMBIE_DIGGER_HARDHAT3 },
	{ HelmType::HELMTYPE_WALLNUT, "anim_face", false, true, false, ParticleEffect::PARTICLE_NONE,
	  &IMAGE_REANIM_WALLNUT_CRACKED1, &IMAGE_REANIM_WALLNUT_CRACKED2 },
	{ HelmType::HELMTYPE_TALLNUT, "anim_idle", false, true, false, ParticleEffect::PARTICLE_NONE,
	  &IMAGE_REANIM_TALLNUT_CRACKED1, &IMAGE_REANIM_TALLNUT_CRACKED2 },
};

struct ShieldLook
{
	ShieldType mType;
	const char *mTrack;
	Image **mDamagedImage1;
	Image **mDamagedImage2;
	/// Clang when hit by a direct shot
	bool mPlaysHitSound;
};

static const ShieldLook gShieldLooks[] = {
	{ ShieldType::SHIELDTYPE_DOOR, "anim_screendoor", &IMAGE_REANIM_ZOMBIE_SCREENDOOR2,
	  &IMAGE_REANIM_ZOMBIE_SCREENDOOR3, true },
	{ ShieldType::SHIELDTYPE_NEWSPAPER, "Zombie_paper_paper", &IMAGE_REANIM_ZOMBIE_PAPER_PAPER2,
	  &IMAGE_REANIM_ZOMBIE_PAPER_PAPER3, false },
	{ ShieldType::SHIELDTYPE_LADDER, "Zombie_ladder_1", &IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1,
	  &IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2, true },
};

static const HelmLook *FindHelmLook(HelmType theType)
{
	for (const HelmLook &aLook : gHelmLooks)
	{
		if (aLook.mType == theType)
			return &aLook;
	}
	return nullptr;
}

static const ShieldLook *FindShieldLook(ShieldType theType)
{
	for (const ShieldLook &aLook : gShieldLooks)
	{
		if (aLook.mType == theType)
			return &aLook;
	}
	return nullptr;
}

static void SetDamagedImage(Reanimation *theReanim, const char *theTrack, int theStage, Image **theImage1,
                            Image **theImage2)
{
	Image **anImage = theStage == 1 ? theImage1 : theStage == 2 ? theImage2 : nullptr;
	if (theReanim && anImage && *anImage)
	{
		theReanim->SetImageOverride(theTrack, *anImage);
	}
}

/// @brief The flash a zombie does when hit
static void FlashOnHit(Zombie *theZombie, unsigned int theDamageFlags)
{
	if (theZombie && !TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH))
	{
		theZombie->mJustGotShotCounter = 25;
	}
}

// ─── ArmorComponent ──────────────────────────────────────────────────────────

int ArmorComponent::GetDamageStage() const
{
	if (mHealth < mMaxHealth / 3)
		return 2;
	if (mHealth < mMaxHealth * 2 / 3)
		return 1;
	return 0;
}

int ArmorComponent::TakeDamage(int theDamage, unsigned int theDamageFlags)
{
	int aStageBefore = GetDamageStage();
	int aDamageActual = std::min(mHealth, theDamage);
	mHealth -= aDamageActual;
	if (mHealth <= 0)
	{
		Break(theDamageFlags);
		return theDamage - aDamageActual;
	}

	int aStageAfter = GetDamageStage();
	if (aStageAfter != aStageBefore)
	{
		OnDamageStageChanged(aStageAfter);
	}
	return theDamage - aDamageActual;
}

void ArmorComponent::Break(unsigned int theDamageFlags)
{
	mHealth = 0;
	// OnBroken first: the drop code still sees the armour on the entity (Zombie::DropShield removes it itself)
	OnBroken(theDamageFlags);
	RemoveSelf();
}

void ArmorComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("health", mHealth);
	theArchive.Value("maxHealth", mMaxHealth);
}

// ─── HelmComponent ───────────────────────────────────────────────────────────

int HelmComponent::TakeDamage(int theDamage, unsigned int theDamageFlags)
{
	Zombie *aZombie = GetZombie();
	FlashOnHit(aZombie, theDamageFlags);
	if (aZombie && TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_FREEZE))
	{
		aZombie->ApplyChill(false);
	}
	return ArmorComponent::TakeDamage(theDamage, theDamageFlags);
}

void HelmComponent::OnDamageStageChanged(int theStage)
{
	Zombie *aZombie = GetZombie();
	const HelmLook *aLook = FindHelmLook(mType);
	if (aZombie == nullptr || aLook == nullptr)
		return;

	ReanimationID aReanimID = aLook->mOnPlantHead ? aZombie->mSpecialHeadReanimID : aZombie->mBodyReanimID;
	SetDamagedImage(GetApp()->ReanimationTryToGet(aReanimID), aLook->mTrack, theStage, aLook->mDamagedImage1,
	                aLook->mDamagedImage2);
}

void HelmComponent::OnBroken(unsigned int theDamageFlags)
{
	Zombie *aZombie = GetZombie();
	if (aZombie == nullptr)
		return;

	ZombieDrawPosition aDrawPos;
	aZombie->GetDrawPos(aDrawPos);
	float aPosX = aZombie->mPosX + aDrawPos.mImageOffsetX + aDrawPos.mHeadX + 14.0f;
	float aPosY = aZombie->mPosY + aDrawPos.mImageOffsetY + aDrawPos.mHeadY + aDrawPos.mBodyY + 18.0f;
	ParticleEffect anEffect = ParticleEffect::PARTICLE_NONE;

	const HelmLook *aLook = FindHelmLook(mType);
	if (aLook && !aLook->mOnPlantHead)
	{
		aZombie->GetTrackPosition(aLook->mTrack, aPosX, aPosY);
		if (aLook->mTrackIsPrefix)
		{
			aZombie->ReanimShowPrefix(aLook->mTrack, RENDER_GROUP_HIDDEN);
		}
		else
		{
			aZombie->ReanimShowTrack(aLook->mTrack, RENDER_GROUP_HIDDEN);
		}
		if (aLook->mShowsHair)
		{
			aZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_NORMAL);
		}
		anEffect = aLook->mDropParticle;
	}

	bool isLeavingBody = !TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_LEAVE_BODY);
	if (mType == HelmType::HELMTYPE_BOBSLED && isLeavingBody)
	{
		aZombie->BobsledCrash();
	}

	if (isLeavingBody && anEffect != ParticleEffect::PARTICLE_NONE)
	{
		TodParticleSystem *aParticle = GetApp()->AddTodParticle(aPosX, aPosY, aZombie->mRenderOrder + 1, anEffect);
		aZombie->OverrideParticleScale(aParticle);
	}
}

void HelmComponent::Serialize(ComponentArchive &theArchive)
{
	ArmorComponent::Serialize(theArchive);
	theArchive.Value("helmType", mType);
}

// ─── ShieldComponent ─────────────────────────────────────────────────────────

bool ShieldComponent::IsHitBy(unsigned int theDamageFlags)
{
	return !TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_BYPASSES_SHIELD);
}

int ShieldComponent::TakeDamage(int theDamage, unsigned int theDamageFlags)
{
	Zombie *aZombie = GetZombie();
	if (aZombie && !TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH))
	{
		aZombie->mShieldJustGotShotCounter = 25;
		if (aZombie->mJustGotShotCounter < 0)
		{
			aZombie->mJustGotShotCounter = 0;
		}

		if (!TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY))
		{
			aZombie->mShieldRecoilCounter = 12;
			const ShieldLook *aLook = FindShieldLook(mType);
			if (aLook && aLook->mPlaysHitSound)
			{
				GetApp()->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
			}
		}
	}

	int aDamageRemaining = ArmorComponent::TakeDamage(theDamage, theDamageFlags);
	// Splash hits the shield and, in full, whatever is behind it
	return TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY) ? theDamage : aDamageRemaining;
}

void ShieldComponent::OnDamageStageChanged(int theStage)
{
	Zombie *aZombie = GetZombie();
	const ShieldLook *aLook = FindShieldLook(mType);
	if (aZombie && aLook)
	{
		SetDamagedImage(GetApp()->ReanimationTryToGet(aZombie->mBodyReanimID), aLook->mTrack, theStage,
		                aLook->mDamagedImage1, aLook->mDamagedImage2);
	}
}

void ShieldComponent::OnBroken(unsigned int theDamageFlags)
{
	// Dropping a shield changes how the zombie behaves (newspaper rage, door detaching...), so the zombie does it
	if (Zombie *aZombie = GetZombie())
	{
		aZombie->DropShield(theDamageFlags);
	}
}

void ShieldComponent::Serialize(ComponentArchive &theArchive)
{
	ArmorComponent::Serialize(theArchive);
	theArchive.Value("shieldType", mType);
}

// ─── FlyingComponent ─────────────────────────────────────────────────────────

bool FlyingComponent::IsHitBy(unsigned int theDamageFlags)
{
	(void)theDamageFlags;
	Zombie *aZombie = GetZombie();
	return aZombie && aZombie->IsFlying();
}

int FlyingComponent::TakeDamage(int theDamage, unsigned int theDamageFlags)
{
	FlashOnHit(GetZombie(), theDamageFlags);
	return ArmorComponent::TakeDamage(theDamage, theDamageFlags);
}

void FlyingComponent::OnBroken(unsigned int theDamageFlags)
{
	if (Zombie *aZombie = GetZombie())
	{
		aZombie->LandFlyer(theDamageFlags);
	}
}
