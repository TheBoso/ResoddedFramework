#pragma once

#include "Component.h"

// Traits plants react to. Each replaces a list of zombie types checked in plant code.

/// @brief Too big to be swallowed whole: a Chomper only bites it (Gargantuar, Giga-gargantuar, Dr. Zomboss)
class UnswallowableComponent : public Component
{
	COMPONENT_TYPE(UnswallowableComponent)
};
