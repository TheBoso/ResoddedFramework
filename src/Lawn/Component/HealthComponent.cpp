#include "HealthComponent.h"

REGISTER_COMPONENT(HealthComponent)

void HealthComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value(mHealth);
	theArchive.Value(mMaxHealth);
}
