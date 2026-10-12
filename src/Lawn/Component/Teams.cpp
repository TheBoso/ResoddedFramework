#include "Teams.h"
#include "Archetypes.h"
#include "ComponentRegistry.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Resources.h"

int Teams::Of(Plant *thePlant)
{
	TeamComponent *aTeam = thePlant->GetComponent<TeamComponent>();
	return aTeam ? aTeam->mTeam : TEAM_PLANTS;
}

int Teams::Of(Zombie *theZombie)
{
	TeamComponent *aTeam = theZombie->GetComponent<TeamComponent>();
	if (aTeam)
		return aTeam->mTeam;
	return theZombie->mMindControlled ? TEAM_PLANTS : TEAM_ZOMBIES;
}

int Teams::Of(Projectile *theProjectile)
{
	TeamComponent *aTeam = theProjectile->GetComponent<TeamComponent>();
	return aTeam ? aTeam->mTeam : Archetypes::DefaultProjectileTeam(theProjectile->mProjectileType);
}

static void SetTeam(GameObject *theObject, int theTeam)
{
	if (TeamComponent *aTeam = theObject->GetComponent<TeamComponent>())
	{
		aTeam->mTeam = theTeam;
	}
	else
	{
		theObject->AddComponent<TeamComponent>(theTeam);
	}
}

void Teams::Set(Plant *thePlant, int theTeam)
{
	SetTeam(thePlant, theTeam);
}

void Teams::Set(Zombie *theZombie, int theTeam)
{
	SetTeam(theZombie, theTeam);
}

void Teams::Set(Projectile *theProjectile, int theTeam)
{
	SetTeam(theProjectile, theTeam);
}

int Teams::FacingOf(Plant *thePlant)
{
	FacingComponent *aFacing = thePlant->GetComponent<FacingComponent>();
	return aFacing ? aFacing->mDirection : 1;
}

int Teams::FacingOf(Zombie *theZombie)
{
	FacingComponent *aFacing = theZombie->GetComponent<FacingComponent>();
	if (aFacing)
		return aFacing->mDirection;
	return theZombie->mMindControlled ? 1 : -1;
}

bool Teams::IsMirrored(Plant *thePlant)
{
	FacingComponent *aFacing = thePlant->GetComponent<FacingComponent>();
	return aFacing && aFacing->mDirection != aFacing->mNativeDirection;
}

bool Teams::IsConverted(Plant *thePlant)
{
	TeamComponent *aTeam = thePlant->GetComponent<TeamComponent>();
	return aTeam && aTeam->mTeam != aTeam->mNativeTeam;
}

bool Teams::CanAttack(int theAttackerTeam, Zombie *theZombie, unsigned int theDamageRangeFlags)
{
	return AreHostile(theAttackerTeam, Of(theZombie)) && theZombie->IsTargetableBy(theDamageRangeFlags);
}

bool Teams::CanAttack(int theAttackerTeam, Plant *thePlant)
{
	return !thePlant->mDead && AreHostile(theAttackerTeam, Of(thePlant));
}

// ─── Hypnosis ────────────────────────────────────────────────────────────────

bool Hypnosis::HypnotizesEater(Plant *thePlant, Zombie *theEater)
{
	HypnotizeOnEatenComponent *aTrap = thePlant->GetComponent<HypnotizeOnEatenComponent>();
	if (aTrap == nullptr || (aTrap->mRequiresAwake && thePlant->mIsAsleep))
		return false;

	HypnotizableComponent *aHypnotizable = theEater->GetComponent<HypnotizableComponent>();
	return aHypnotizable && Teams::Of(theEater) != aHypnotizable->mHypnotizedTeam;
}

void Hypnosis::OnZombieHypnotized(Zombie *theZombie)
{
	HypnotizableComponent *aHypnotizable = theZombie->GetComponent<HypnotizableComponent>();
	int aTeam = aHypnotizable ? aHypnotizable->mHypnotizedTeam : TEAM_PLANTS;
	Teams::Set(theZombie, aTeam);

	// Vanilla zombies always walk back when hypnotized, so Facing follows mMindControlled
	if (FacingComponent *aFacing = theZombie->GetComponent<FacingComponent>())
	{
		aFacing->mDirection = 1;
	}

	theZombie->ForEachComponent([](Component &theComponent) { theComponent.OnHypnotized(); });
}

bool Hypnosis::HypnotizePlant(Plant *thePlant)
{
	HypnotizableComponent *aHypnotizable = thePlant->GetComponent<HypnotizableComponent>();
	if (aHypnotizable == nullptr || Teams::Of(thePlant) == aHypnotizable->mHypnotizedTeam)
		return false;

	Teams::Set(thePlant, aHypnotizable->mHypnotizedTeam);
	if (aHypnotizable->mFlipFacing)
	{
		FacingComponent *aFacing = thePlant->GetComponent<FacingComponent>();
		if (aFacing)
		{
			aFacing->mDirection = -aFacing->mDirection;
		}
	}

	thePlant->ForEachComponent([](Component &theComponent) { theComponent.OnHypnotized(); });

	thePlant->mApp->PlaySample(SOUND_MINDCONTROLLED);
	thePlant->mApp->AddTodParticle(thePlant->mX + 40.0f, thePlant->mY + 40.0f, thePlant->mRenderOrder + 1,
	                               ParticleEffect::PARTICLE_MIND_CONTROL);
	thePlant->UpdateReanimColor();
	return true;
}

// ─── Combat ──────────────────────────────────────────────────────────────────

void Combat::Damage(GameObject *theTarget, int theDamage, unsigned int theDamageFlags)
{
	EntityRef anEntity = theTarget ? theTarget->GetEntityRef() : EntityRef();
	if (anEntity.mKind == EntityKind::Zombie)
	{
		static_cast<Zombie *>(theTarget)->TakeDamage(theDamage, theDamageFlags);
	}
	else if (anEntity.mKind == EntityKind::Plant)
	{
		DamagePlant(static_cast<Plant *>(theTarget), theDamage,
		            !TestBit(theDamageFlags, (int)DamageFlags::DAMAGE_DOESNT_CAUSE_FLASH));
	}
	else if (HealthComponent *aHealth = theTarget ? theTarget->GetComponent<HealthComponent>() : nullptr)
	{
		aHealth->mHealth -= theDamage;
		theTarget->ForEachComponent([theDamage](Component &theComponent) { theComponent.OnDamaged(theDamage); });
	}
}

void Combat::DamagePlant(Plant *thePlant, int theDamage, bool theFlash)
{
	thePlant->Health().mHealth -= theDamage;
	if (theFlash)
	{
		thePlant->mEatenFlashCountdown = std::max(thePlant->mEatenFlashCountdown, 25);
	}
	thePlant->ForEachComponent([theDamage](Component &theComponent) { theComponent.OnDamaged(theDamage); });
}
