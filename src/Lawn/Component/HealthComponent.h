#pragma once

#include "Component.h"

/// @brief Hit points. Plants and zombies always have one (GameObject::Health() creates it on first use);
/// any other entity can get one too.
///
/// Zombies stack armour on top of the body. Damage goes Flying (balloon) -> Shield (screen door, newspaper...) ->
/// Helm (cone, bucket...) -> Body, see Zombie::TakeDamage. What a layer *is* (its look, what drops off) still comes
/// from Zombie::mHelmType / mShieldType.
///
///     zombie->Health().mHealth = 500;
///     zombie->Health().mHelmHealth = zombie->Health().mHelmMaxHealth = 1100; // give it a bucket's worth of armour
class HealthComponent : public Component
{
	COMPONENT_TYPE(HealthComponent)

	HealthComponent() = default;
	explicit HealthComponent(int theHealth) : mHealth(theHealth), mMaxHealth(theHealth)
	{
	}

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief Body health plus every armour layer
	int GetTotalHealth() const
	{
		return mHealth + mHelmHealth + mShieldHealth + mFlyingHealth;
	}
	/// @brief Body health in [0, 1] of its maximum
	float GetHealthFraction() const
	{
		return mMaxHealth > 0 ? (float)mHealth / (float)mMaxHealth : 0.0f;
	}
	/// @brief Set body health and its maximum
	void SetHealth(int theHealth)
	{
		mHealth = mMaxHealth = theHealth;
	}

	// Body (plants only use this)
	int32_t mHealth = 0;
	int32_t mMaxHealth = 0;

	// Zombie armour layers
	int32_t mHelmHealth = 0;
	int32_t mHelmMaxHealth = 0;
	int32_t mShieldHealth = 0;
	int32_t mShieldMaxHealth = 0;
	int32_t mFlyingHealth = 0;
	int32_t mFlyingMaxHealth = 0;
};
