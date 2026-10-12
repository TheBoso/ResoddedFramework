#pragma once

#include "../../ConstEnums.h"
#include "../GameObject.h"

/// @brief Order in which armour soaks up damage: outermost first
enum class ArmorLayer : int32_t
{
	Flying = 0, // balloon
	Shield = 1, // screen door, newspaper, ladder
	Helm = 2    // cone, bucket, football helmet...
};

/// @brief A layer of armour on top of an entity's HealthComponent. Zombie::TakeDamage runs damage through every
/// armour component, outermost layer first, and whatever is left hits the body.
///
///     zombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_PAIL, 1100); // give any zombie a bucket
///
/// New armour kinds derive from this and override the hooks; per-type looks of the vanilla helms and shields are
/// data tables in ArmorComponents.cpp.
class ArmorComponent : public Component
{
	COMPONENT_TYPE(ArmorComponent)

	explicit ArmorComponent(int theHealth = 0) : mHealth(theHealth), mMaxHealth(theHealth)
	{
	}

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	virtual ArmorLayer GetLayer() const
	{
		return ArmorLayer::Helm;
	}
	/// @brief Does a hit with these DamageFlags reach this layer? (e.g. lobbed shots fly over shields)
	virtual bool IsHitBy(unsigned int theDamageFlags)
	{
		(void)theDamageFlags;
		return true;
	}
	/// @brief Absorb damage. Breaks (OnBroken) when health reaches 0.
	/// @return The damage that gets through to the next layer
	virtual int TakeDamage(int theDamage, unsigned int theDamageFlags);
	/// @brief Remove the armour, as if it broke (plays the drop effect unless the flags say otherwise)
	void Break(unsigned int theDamageFlags);

	/// @brief 0 = intact, 1 = damaged, 2 = badly damaged (thresholds at 2/3 and 1/3 of max health)
	int GetDamageStage() const;

	int32_t mHealth;
	int32_t mMaxHealth;

  protected:
	/// @brief Swap to the damaged look
	virtual void OnDamageStageChanged(int theStage)
	{
		(void)theStage;
	}
	/// @brief The armour is gone (health 0 or Break). Play its drop effect; it is removed right after.
	virtual void OnBroken(unsigned int theDamageFlags)
	{
		(void)theDamageFlags;
	}
};

/// @brief Head armour: traffic cone, bucket, football helmet, hard hat, Wall-nut/Tall-nut head, bobsled
class HelmComponent : public ArmorComponent
{
	COMPONENT_SUBTYPE(HelmComponent, ArmorComponent)

	HelmComponent() = default;
	HelmComponent(HelmType theType, int theHealth) : ArmorComponent(theHealth), mType(theType)
	{
	}

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;
	int TakeDamage(int theDamage, unsigned int theDamageFlags) override;

	HelmType mType = HelmType::HELMTYPE_NONE;

  protected:
	void OnDamageStageChanged(int theStage) override;
	void OnBroken(unsigned int theDamageFlags) override;
};

/// @brief Held armour: screen door, newspaper, ladder. Lobbed shots (DAMAGE_BYPASSES_SHIELD) skip it, and
/// splash damage (DAMAGE_HITS_SHIELD_AND_BODY) hits both it and what is behind it in full.
class ShieldComponent : public ArmorComponent
{
	COMPONENT_SUBTYPE(ShieldComponent, ArmorComponent)

	ShieldComponent() = default;
	ShieldComponent(ShieldType theType, int theHealth) : ArmorComponent(theHealth), mType(theType)
	{
	}

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;
	ArmorLayer GetLayer() const override
	{
		return ArmorLayer::Shield;
	}
	bool IsHitBy(unsigned int theDamageFlags) override;
	int TakeDamage(int theDamage, unsigned int theDamageFlags) override;

	ShieldType mType = ShieldType::SHIELDTYPE_NONE;

  protected:
	void OnDamageStageChanged(int theStage) override;
	void OnBroken(unsigned int theDamageFlags) override;
};

namespace Armor
{
/// @brief Call theFn(ArmorComponent&) for each armour layer of the object
template <typename Fn> void ForEach(GameObject *theObject, Fn &&theFn)
{
	theObject->ForEachComponent(
	    [&](Component &theComponent)
	    {
		    if (theComponent.IsA(GetComponentTypeId<ArmorComponent>()))
		    {
			    theFn(static_cast<ArmorComponent &>(theComponent));
		    }
	    });
}
} // namespace Armor

/// @brief The Balloon Zombie's balloon: absorbs hits while flying, the zombie lands when it pops
class FlyingComponent : public ArmorComponent
{
	COMPONENT_SUBTYPE(FlyingComponent, ArmorComponent)
	using ArmorComponent::ArmorComponent;

	ArmorLayer GetLayer() const override
	{
		return ArmorLayer::Flying;
	}
	bool IsHitBy(unsigned int theDamageFlags) override;
	int TakeDamage(int theDamage, unsigned int theDamageFlags) override;

  protected:
	void OnBroken(unsigned int theDamageFlags) override;
};
