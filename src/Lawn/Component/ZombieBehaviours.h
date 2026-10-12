#pragma once

#include "Component.h"

class Zombie;

/// @brief Base of zombie behaviours (what used to be the zombie-type dispatch in Zombie::UpdateActions,
/// Zombie::UpdatePlaying and Zombie::Update). The hooks run exactly where the old dispatch was, so timing and the
/// frozen/buttered rules are unchanged:
///
///  - UpdateActions: every tick the zombie can act on the board (not frozen, buttered or rising from a grave)
///  - UpdatePlaying: every tick on the board, even while frozen (Dr. Zomboss keeps scheming)
///  - UpdateAlways:  every tick the zombie updates at all, including outside a level (Almanac previews)
///
/// A zombie can have several behaviours; they run in the order they were added.
class ZombieBehaviourComponent : public Component
{
	COMPONENT_TYPE(ZombieBehaviourComponent)

	virtual void UpdateActions(Zombie *theZombie)
	{
		(void)theZombie;
	}
	virtual void UpdatePlaying(Zombie *theZombie)
	{
		(void)theZombie;
	}
	virtual void UpdateAlways(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief The zombie just lost its head (called from Zombie::DropHead before the head flies off)
	virtual void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
	{
		(void)theZombie;
		(void)theDamageFlags;
	}
};

// The behaviours of the vanilla zombies (implemented in VanillaZombieBehaviours.cpp, moved from Zombie.cpp).
// They play the original zombie's animation tracks, so on a zombie whose animation lacks those tracks they act
// without animating.

#define ZOMBIE_BEHAVIOUR(ClassName, Hook)                                                                              \
	class ClassName : public ZombieBehaviourComponent                                                                  \
	{                                                                                                                  \
		COMPONENT_SUBTYPE(ClassName, ZombieBehaviourComponent)                                                         \
		void Hook(Zombie *theZombie) override;                                                                         \
	};

ZOMBIE_BEHAVIOUR(CatapultBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(DolphinRiderBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(SnorkelBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(NewspaperBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(DiggerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(JackInTheBoxBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(GargantuarBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(BobsledBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(ZamboniBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(LadderBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(YetiBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(DancerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(BackupDancerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(ImpBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(PeaHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(JalapenoHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(GatlingHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(SquashHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(BossBehaviour, UpdatePlaying)
ZOMBIE_BEHAVIOUR(BungeeBehaviour, UpdateAlways)

#undef ZOMBIE_BEHAVIOUR

/// @brief Pole Vaulter: runs, vaults over the first plant, then walks. Drops its pole with its head.
class PolevaulterBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PolevaulterBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
};

/// @brief Balloon Zombie: floats over plants until the balloon pops. Loses its hat with its head.
class BalloonBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BalloonBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
};

/// @brief Pogo Zombie: bounces over plants. The stick breaks when it loses its head.
class PogoBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PogoBehaviour, ZombieBehaviourComponent)
	void UpdateAlways(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
};

/// @brief Flag Zombie: drops its flag with its head
class FlagBearerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(FlagBearerBehaviour, ZombieBehaviourComponent)
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
};
