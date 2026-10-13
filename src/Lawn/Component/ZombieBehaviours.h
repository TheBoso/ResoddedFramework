#pragma once

#include "Component.h"
#include "../../ConstEnums.h"

namespace Sexy
{
class Graphics;
class Color;
} // namespace Sexy

class Zombie;
class ZombieDrawPosition;

/// @brief What a spawning zombie's behaviours can still change (Zombie::ZombieInitialize)
struct ZombieSpawn
{
	/// The zombie that brought this one along (the Bobsled leader for its team)
	Zombie *mParent = nullptr;
	RenderLayer mRenderLayer = RenderLayer::RENDER_LAYER_ZOMBIE;
	int mRenderOffset = 4;
	/// The zombie removed itself while spawning (a Bungee with nothing to grab)
	bool mCancelled = false;
};

/// @brief Base of zombie behaviours (what used to be the zombie-type checks in Zombie.cpp). The hooks run exactly
/// where those checks were, so timing and the frozen/buttered rules are unchanged.
///
/// Every tick:
///  - UpdateActions: the zombie can act on the board (not frozen, buttered or rising from a grave)
///  - UpdatePlaying: on the board, even while frozen (Dr. Zomboss keeps scheming)
///  - UpdateAlways:  the zombie updates at all, including outside a level (Almanac previews)
///
/// Events (the bool ones: return true to say the behaviour took care of it):
///  - OnSpawn (what the zombie does as it appears, after its body data was applied)
///  - OnLoseHead, OnLoseArm, OnButtered, OnButterRemoved, OnBodyDamaged, OnHitByMower, OnMowed, OnBurned
///  - HasDeathAnim, OnDying, UpdateDying, OnRemoved
///  - PlaceBody, DrawBody (zombies drawn their own way)
///  - CanBeTargeted, CanBeTargetedOffScreen (Zombie::IsTargetableBy)
///  - Smaller rules: PickSpeed, ControlsAltitude, IsWalkingBackwards, CanBeChilled, CanBeFrozen, HasShadow,
///    AdjustShadow, OnFrozen, OnThawed, OnKilled, DropOwnLoot, OverrideBodyColor, UpdateBodyReanim,
///    UpdatesInLevelIntro
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
	/// @brief The zombie is spawning: its body data (health, hitbox, where it enters...) is applied and its
	/// animation loaded. Set up its starting phase and animation here.
	virtual void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn)
	{
		(void)theZombie;
		(void)theSpawn;
	}
	/// @brief The zombie just lost its head (called from Zombie::DropHead before the head flies off)
	virtual void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags)
	{
		(void)theZombie;
		(void)theDamageFlags;
	}
	/// @brief The zombie just lost its arm (called from Zombie::SetupReanimForLostArm once the arm is gone)
	virtual void OnLoseArm(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief Butter landed on the zombie's head (it can't move until it wears off)
	virtual void OnButtered(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief The butter wore off
	virtual void OnButterRemoved(Zombie *theZombie)
	{
		(void)theZombie;
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
	/// @brief false: nothing can target the zombie right now (a Bungee in the air, Dr. Zomboss without his head
	/// down, the Bobsled team behind the leader)
	virtual bool CanBeTargeted(Zombie *theZombie)
	{
		(void)theZombie;
		return true;
	}
	/// @brief true: the zombie can be targeted before it walks onto the screen (the Bobsled team)
	virtual bool CanBeTargetedOffScreen(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief Return true after setting theZombie->mVelX to override the regular speed pick (I, Zombie Imps)
	virtual bool PickSpeed(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief true: the behaviour sets the zombie's altitude itself, so high ground leaves it alone (Pogo)
	virtual bool ControlsAltitude(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief true: the zombie walks towards the right right now (Digger after surfacing, Yeti running away)
	virtual bool IsWalkingBackwards(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
	}
	/// @brief false: the zombie can't be chilled right now (Zamboni, Dr. Zomboss without his head down)
	virtual bool CanBeChilled(Zombie *theZombie)
	{
		(void)theZombie;
		return true;
	}
	/// @brief false: the zombie can't be frozen right now (a Bungee in the air)
	virtual bool CanBeFrozen(Zombie *theZombie)
	{
		(void)theZombie;
		return true;
	}
	/// @brief false: the zombie has no shadow right now (Dr. Zomboss, a Bungee off the board)
	virtual bool HasShadow(Zombie *theZombie)
	{
		(void)theZombie;
		return true;
	}
	/// @brief Change where the shadow is drawn and how big (a dropping Bungee's shadow grows)
	virtual void AdjustShadow(Zombie *theZombie, float &theOffsetX, float &theScale)
	{
		(void)theZombie;
		(void)theOffsetX;
		(void)theScale;
	}
	/// @brief The zombie got frozen in an ice block / thawed out
	virtual void OnFrozen(Zombie *theZombie)
	{
		(void)theZombie;
	}
	virtual void OnThawed(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief The player defeated the zombie (Zombie::DropLoot, once its loot is decided)
	virtual void OnKilled(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief Return true after dropping the zombie's own loot instead of the regular coins (the Yeti's diamonds)
	virtual bool DropOwnLoot(Zombie *theZombie, int theCenterX, int theCenterY)
	{
		(void)theZombie;
		(void)theCenterX;
		(void)theCenterY;
		return false;
	}
	/// @brief Return true after setting the body's tint (Dr. Zomboss flashing when badly hurt)
	virtual bool OverrideBodyColor(Zombie *theZombie, Sexy::Color &theColor, int theAlpha)
	{
		(void)theZombie;
		(void)theColor;
		(void)theAlpha;
		return false;
	}
	/// @brief Every tick, before the body animation is positioned (Zombie::UpdateReanim)
	virtual void UpdateBodyReanim(Zombie *theZombie)
	{
		(void)theZombie;
	}
	/// @brief true: the zombie keeps updating during the level intro (Dr. Zomboss)
	virtual bool UpdatesInLevelIntro(Zombie *theZombie)
	{
		(void)theZombie;
		return false;
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

#define ZOMBIE_BEHAVIOUR(ClassName, Hook)                                \
	class ClassName : public ZombieBehaviourComponent                    \
	{                                                                    \
		COMPONENT_SUBTYPE(ClassName, ZombieBehaviourComponent)           \
		void Hook(Zombie *theZombie) override;                           \
		void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override; \
	};

ZOMBIE_BEHAVIOUR(JackInTheBoxBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(LadderBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(BackupDancerBehaviour, UpdateActions)
ZOMBIE_BEHAVIOUR(JalapenoHeadBehaviour, UpdateActions)

#undef ZOMBIE_BEHAVIOUR

/// @brief Digger: tunnels to the house end, surfaces and walks back
class DiggerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(DiggerBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	bool IsWalkingBackwards(Zombie *theZombie) override;
};

/// @brief Yeti: wanders in, then runs away; drops diamonds
class YetiBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(YetiBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	bool IsWalkingBackwards(Zombie *theZombie) override;
	void OnKilled(Zombie *theZombie) override;
	bool DropOwnLoot(Zombie *theZombie, int theCenterX, int theCenterY) override;
};

/// @brief Imp: thrown by the Gargantuar; sprints in I, Zombie
class ImpBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(ImpBehaviour, ZombieBehaviourComponent)
	void UpdateActions(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	bool PickSpeed(Zombie *theZombie) override;
};

/// @brief Pole Vaulter: runs, vaults over the first plant, then walks. Drops its pole with its head.
class PolevaulterBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PolevaulterBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Newspaper Zombie: gets angry and fast when it loses its paper
class NewspaperBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(NewspaperBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Balloon Zombie: floats over plants until the balloon pops. Loses its hat with its head.
class BalloonBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BalloonBehaviour, ZombieBehaviourComponent)
	void OnFrozen(Zombie *theZombie) override;
	void OnThawed(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnButtered(Zombie *theZombie) override;
	void OnButterRemoved(Zombie *theZombie) override;
	void OnMowed(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
};

/// @brief Pogo Zombie: bounces over plants. The stick breaks when it loses its head.
class PogoBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PogoBehaviour, ZombieBehaviourComponent)
	bool ControlsAltitude(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateAlways(Zombie *theZombie) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnLoseArm(Zombie *theZombie) override;
	void OnButtered(Zombie *theZombie) override;
	void OnMowed(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
};

/// @brief Flag Zombie: drops its flag with its head
class FlagBearerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(FlagBearerBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void OnLoseHead(Zombie *theZombie, unsigned int theDamageFlags) override;
	void OnLoseArm(Zombie *theZombie) override;
	void OnMowed(Zombie *theZombie) override;
};

/// @brief Peashooter zombie: fires peas from its head
class PeaHeadBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(PeaHeadBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnButterRemoved(Zombie *theZombie) override;
};

/// @brief Gatling Pea zombie: fires bursts of peas from its head
class GatlingHeadBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(GatlingHeadBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnButterRemoved(Zombie *theZombie) override;
};

/// @brief Dolphin Rider: rides into the pool. Out of the pool it has no death animation.
class DolphinRiderBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(DolphinRiderBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	bool HasDeathAnim(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
};

/// @brief Snorkel Zombie: dives into the pool and swims under water, coming up to eat
class SnorkelBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(SnorkelBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
};

/// @brief Dancer: moonwalks in, summons Backup Dancers and leads the troupe under its spotlight
class DancerBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(DancerBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Catapult: lobs basketballs at plants; blows up when mowed or destroyed
class CatapultBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(CatapultBehaviour, ZombieBehaviourComponent)
	void UpdateBodyReanim(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
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
	bool CanBeChilled(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
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
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
};

/// @brief Bobsled team: slides in on the ice; the team dies and burns together
class BobsledBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BobsledBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
	bool CanBeTargeted(Zombie *theZombie) override;
	bool CanBeTargetedOffScreen(Zombie *theZombie) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Squash head: jumps on a plant. A headless one loses its squash when it dies.
class SquashHeadBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(SquashHeadBehaviour, ZombieBehaviourComponent)
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateActions(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
	void OnBurned(Zombie *theZombie) override;
};

/// @brief Bungee Zombie: drops from the sky and steals a plant; takes the plant with it when it dies
class BungeeBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BungeeBehaviour, ZombieBehaviourComponent)
	bool CanBeFrozen(Zombie *theZombie) override;
	bool HasShadow(Zombie *theZombie) override;
	void AdjustShadow(Zombie *theZombie, float &theOffsetX, float &theScale) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdateAlways(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
	bool CanBeTargeted(Zombie *theZombie) override;
	bool PlaceBody(Zombie *theZombie, ZombieDrawPosition &theDrawPos) override;
	bool DrawBody(Zombie *theZombie, Sexy::Graphics *g, const ZombieDrawPosition &theDrawPos,
	              int theBaseRenderGroup) override;
};

/// @brief Dr. Zomboss: the whole boss fight, its damaged look and its finale
class BossBehaviour : public ZombieBehaviourComponent
{
	COMPONENT_SUBTYPE(BossBehaviour, ZombieBehaviourComponent)
	bool CanBeChilled(Zombie *theZombie) override;
	bool HasShadow(Zombie *theZombie) override;
	bool OverrideBodyColor(Zombie *theZombie, Sexy::Color &theColor, int theAlpha) override;
	bool UpdatesInLevelIntro(Zombie *theZombie) override;
	void OnSpawn(Zombie *theZombie, ZombieSpawn &theSpawn) override;
	void UpdatePlaying(Zombie *theZombie) override;
	bool OnBodyDamaged(Zombie *theZombie, unsigned int theDamageFlags, int theHealthBefore, int theDamageIndexBefore,
	                   int theDamageIndexAfter) override;
	bool OnHitByMower(Zombie *theZombie) override;
	void OnDying(Zombie *theZombie) override;
	bool UpdateDying(Zombie *theZombie) override;
	void OnRemoved(Zombie *theZombie) override;
	bool CanBeTargeted(Zombie *theZombie) override;
};
