#include "HealthComponent.h"

REGISTER_COMPONENT(HealthComponent)

void HealthComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mHealth);
	theArchive.Value(mMaxHealth);
	theArchive.Value(mHelmHealth);
	theArchive.Value(mHelmMaxHealth);
	theArchive.Value(mShieldHealth);
	theArchive.Value(mShieldMaxHealth);
	theArchive.Value(mFlyingHealth);
	theArchive.Value(mFlyingMaxHealth);
}
