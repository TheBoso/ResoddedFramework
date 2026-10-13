#include "StatusEffects.h"
#include "ComponentRegistry.h"
#include "../Board.h"

REGISTER_COMPONENT(ChilledStatus)
REGISTER_COMPONENT(FrozenStatus)
REGISTER_COMPONENT(ButteredStatus)

bool StatusEffectComponent::CanTick()
{
	// Same conditions under which the vanilla counters counted down (Zombie::UpdatePlaying)
	if (Zombie *aZombie = GetZombie())
		return aZombie->IsOnBoard() && !aZombie->IsDeadOrDying();
	if (Plant *aPlant = GetPlant())
		return aPlant->IsOnBoard() && !aPlant->mDead;
	return true;
}

void StatusEffectComponent::OnTick()
{
	if (!CanTick())
		return;

	if (--mTicksLeft <= 0)
	{
		Expire();
	}
}

void StatusEffectComponent::Expire()
{
	mTicksLeft = 0;
	// Remove first, so OnExpired's code (which checks IsChilled() etc.) already sees the effect gone
	RemoveSelf();
	OnExpired();
}

void StatusEffectComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("ticksLeft", mTicksLeft);
}

void ChilledStatus::OnExpired()
{
	if (Zombie *aZombie = GetZombie())
	{
		aZombie->UpdateAnimSpeed();
	}
	else if (Plant *aPlant = GetPlant())
	{
		aPlant->UpdateReanimColor();
	}
}

void FrozenStatus::OnExpired()
{
	if (Zombie *aZombie = GetZombie())
	{
		aZombie->RemoveIceTrap();
		aZombie->AddAttachedParticle(75, 106, ParticleEffect::PARTICLE_ICE_TRAP_RELEASE);
	}
	else if (Plant *aPlant = GetPlant())
	{
		aPlant->UpdateReanimColor();
	}
}

void ButteredStatus::OnExpired()
{
	if (Zombie *aZombie = GetZombie())
	{
		aZombie->RemoveButter();
	}
	else if (Plant *aPlant = GetPlant())
	{
		aPlant->UpdateReanimColor();
	}
}
