#pragma once

#include <algorithm>
#include "../GameObject.h"

/// @brief Base of timed status effects: counts down in OnTick and removes itself when it runs out.
/// GetComponent<StatusEffectComponent>() finds any active effect.
///
///     StatusEffects::Extend<ChilledStatus>(zombie, 1000); // chill for at least 10 seconds
///     if (plant->IsFrozen()) ...
///
/// Effects only tick while their owner is on the board and alive (dying zombies keep their look).
class StatusEffectComponent : public Component
{
	COMPONENT_TYPE(StatusEffectComponent)

	explicit StatusEffectComponent(int theTicks = 0) : mTicksLeft(theTicks)
	{
	}

	void OnTick() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief End the effect now (runs OnExpired)
	void Expire();

	/// Ticks until the effect wears off (100 ticks = 1 second)
	int32_t mTicksLeft;

  protected:
	/// @brief Undo the effect's side effects (animation speed, colors...). The component is removed right after.
	virtual void OnExpired()
	{
	}
	/// @brief Is the owner in a state where the effect wears off?
	virtual bool CanTick();
};

/// @brief Slowed down: zombies walk, eat and shoot at half speed; plants act at half speed. Tinted blue.
class ChilledStatus : public StatusEffectComponent
{
	COMPONENT_SUBTYPE(ChilledStatus, StatusEffectComponent)
	using StatusEffectComponent::StatusEffectComponent;

  protected:
	void OnExpired() override;
};

/// @brief Frozen solid in an ice block (Ice-shroom): can't move or act.
class FrozenStatus : public StatusEffectComponent
{
	COMPONENT_SUBTYPE(FrozenStatus, StatusEffectComponent)
	using StatusEffectComponent::StatusEffectComponent;

  protected:
	void OnExpired() override;
};

/// @brief Stuck in butter (Kernel-pult): can't move or act.
class ButteredStatus : public StatusEffectComponent
{
	COMPONENT_SUBTYPE(ButteredStatus, StatusEffectComponent)
	using StatusEffectComponent::StatusEffectComponent;

  protected:
	void OnExpired() override;
};

/// @brief Helpers to apply effects without caring whether one is already active
namespace StatusEffects
{
/// @brief Ticks left on the effect, 0 if it isn't active
template <typename T> int GetTicks(GameObject *theObject)
{
	T *anEffect = theObject->GetComponent<T>();
	return anEffect ? anEffect->mTicksLeft : 0;
}

/// @brief Make the effect last exactly theTicks (0 or less removes it without running OnExpired)
template <typename T> void Set(GameObject *theObject, int theTicks)
{
	if (theTicks <= 0)
	{
		theObject->RemoveComponent<T>();
	}
	else if (T *anEffect = theObject->GetComponent<T>())
	{
		anEffect->mTicksLeft = theTicks;
	}
	else
	{
		theObject->AddComponent<T>(theTicks);
	}
}

/// @brief Make the effect last at least theTicks (a weaker re-application doesn't shorten it)
template <typename T> void Extend(GameObject *theObject, int theTicks)
{
	Set<T>(theObject, std::max(theTicks, GetTicks<T>(theObject)));
}
} // namespace StatusEffects
