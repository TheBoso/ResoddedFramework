#include "HealthComponent.h"

REGISTER_COMPONENT(HealthComponent)

void HealthComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("health", mHealth);
	theArchive.Value("maxHealth", mMaxHealth);
}
