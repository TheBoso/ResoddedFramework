#pragma once

#include "Component.h"

namespace Sexy
{
class Graphics;
}

class Zombie;
class ZombieDrawPosition;

/// @brief Base of zombie behaviours (what used to be the zombie-type checks in Zombie.cpp). The hooks run exactly
/// where those checks were, so timing and the frozen/buttered rules are unchanged.
///
/// Every tick:
///  - UpdateActions: the zombie can act on the board (not frozen, buttered or rising from a grave)
///  - UpdatePlaying: on the board, even while frozen (Dr. Zomboss keeps scheming)
///  - UpdateAlways:  the zombie updates at all, including outside a level (Almanac previews)
///
/// Events (the bool ones: return true to say the behaviour took care of it):
///  - OnLoseHead, OnBodyDamaged, OnHitByMower, OnMowed, OnBurned
///  - HasDeathAnim, OnDying, UpdateDying, OnRemoved
///  - PlaceBody, DrawBody (zombies drawn their own way)
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
	/// @brief The zombie's body took damage (its health is already lowered). Return true when the behaviour shows the
	/// damage itself (Zamboni, Gargantuar...) instead of the regular damage states (losing an arm, then the head).
	virtual bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore,
	                           int theDamageIndexBefore, int theDamageIndexAfter)
	{
		(void)theZombie;
		(void)theDamageFlags;
		(void)theHealthBefore;
		(void)theDamageIndexBefore;
		(void)theDamageIndexAfter;
		return false;
	}
	/// @brief A lawn mower reached the zombie. Return true when the behaviour deals with it and the zombie is not cut
	/// down the regular way (Dr. Zomboss ignores mowers, vehicles blow up).
	virtual bool OnHitByMower(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief A mower is cutting the zombie down: drop what it carries
	virtual void OnMowed(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief Burned (Jalapeno, Cob Cannon...), after the zombie was charred or burned where it stands
	virtual void OnBurned(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief false: the zombie disappears instead of playing its death animation (Dolphin Rider out of the pool)
	virtual bool HasDeathAnim(Zombie *theZombie)
	{
		(void)theZombie;
		return true;
	}
	/// @brief The zombie starts its death animation
	virtual void OnDying(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief Every tick of the death animation. Return true when the behaviour ends the zombie itself (Dr. Zomboss'
	/// finale, vehicles blowing up) instead of the regular fade out.
	virtual bool UpdateDying(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief The zombie is being removed from the board (Zombie::DieNoLoot; it can be called more than once)
	virtual void OnRemoved(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief Where the body is drawn and clipped this frame (Zombie::GetDrawPos). Return true when the behaviour
	/// placed it (Dolphin Rider, Snorkel, Bungee) instead of the regular placement.
	virtual bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos)
	{
		(void)theZombie;
		(void)theDrawPos;
		return false;
	}
	/// @brief Draw the body animation. Return true when the behaviour drew it (Bobsled with its sled, Bungee with its
	/// cord, Dancer with its spotlight) instead of the regular drawing.
	virtual bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	                      int theBaseRenderGroup)
	{
		(void)theZombie;
		(void)g;
		(void)theDrawPos;
		(void)theBaseRenderGroup;
		return false;
	}
};

// The behaviours of the vanilla zombies (implemented in VanillaZombieBehaviours.cpp, moved from Zombie.cpp).
// They play the original zombie's animation tracks, so on a zombie whose animation lacks those tracks they act
// without animating.

#define ZOMBIE_BEHAVIOUR(ClassName, Hook)                      \
	class ClassName : public ZombieBehaviourComponent          \
	{                                                          \
		COMPONENT_SUBTYPE(ClassName, ZombieBehaviourComponent) \
		void Hook(Zombie *theZombie) override;                 \
	};

ZOMBIE_BEHAVIOUR(DiggerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(JackInTheBoxBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(LadderBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(YetiBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(BackupDancerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(ImpBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(PeaHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(JalapenoHeadBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(GatlingHeadBehaviour, UpdateActions)

#undef ZOMBIE_BEHAVIOUR

/// @brief Pole Vaulter: runs, vaults over the first plant, then walks. Drops its pole with its head.
class PolevaulterBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PolevaulterBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Newspaper Zombie: gets angry and fast when it loses its paper
class NewspaperBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(NewspaperBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Balloon Zombie: floats over plants until the balloon pops. Loses its hat with its head.
class BalloonBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BalloonBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnMowed(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
};

/// @brief Pogo Zombie: bounces over plants. The stick breaks when it loses its head.
class PogoBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PogoBehaviour, ZombieBehaviourComponent)
	void UpdateAlways(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnMowed(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
};

/// @brief Flag Zombie: drops its flag with its head
class FlagBearerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(FlagBearerBehaviour, ZombieBehaviourComponent)
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Dolphin Rider: rides into the pool. Out of the pool it has no death animation.
class DolphinRiderBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(DolphinRiderBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool HasDeathAnim(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
};

/// @brief Snorkel Zombie: dives into the pool and swims under water, coming up to eat
class SnorkelBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(SnorkelBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
};

/// @brief Dancer: moonwalks in, summons Backup Dancers and leads the troupe under its spotlight
class DancerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(DancerBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Catapult: lobs basketballs at plants; blows up when mowed or destroyed
class CatapultBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(CatapultBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
	bool OnHitByMower(Zombie *theZombie) override;
	bool UpdateDying(Zombie *theZombie) override;
};

/// @brief Zamboni: drives over plants leaving an ice trail; blows up when mowed or destroyed
class ZamboniBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(ZamboniBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
	bool OnHitByMower(Zombie *theZombie) override;
	bool UpdateDying(Zombie *theZombie) override;
};

/// @brief Gargantuar: smashes plants, throws its Imp; its body cracks as it takes damage
class GargantuarBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(GargantuarBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
};

/// @brief Bobsled team: slides in on the ice; the team dies and burns together
class BobsledBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BobsledBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Squash head: jumps on a plant. A headless one loses its squash when it dies.
class SquashHeadBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(SquashHeadBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
};

/// @brief Bungee Zombie: drops from the sky and steals a plant; takes the plant with it when it dies
class BungeeBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BungeeBehaviour, ZombieBehaviourComponent)
	void UpdateAlways(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Dr. Zomboss: the whole boss fight, its damaged look and its finale
class BossBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BossBehaviour, ZombieBehaviourComponent)
	void UpdatePlaying(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
	bool OnHitByMower(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
	bool UpdateDying(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
};
