#pragma once

#include "Component.h"

/// @brief Hit points. Plants and zombies always have one (GameObject::Health() creates it on first use);
/// any other entity can get one too. Armour (cones, doors, balloons) is separate: see ArmorComponents.h.
///
///     zombie->Health().SetHealth(500);
class HealthComponent : public Component
{
	COMPONENT_TYPE(HealthComponent)

	HealthComponent() = default;
	explicit HealthComponent(int theHealth) : mHealth(theHealth), mMaxHealth(theHealth)
	{
	}

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

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

	int32_t mHealth = 0;
	int32_t mMaxHealth = 0;
};
