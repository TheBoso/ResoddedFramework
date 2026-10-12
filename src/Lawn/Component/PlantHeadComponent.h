#pragma once

#include "../../ConstEnums.h"
#include "Component.h"

/// @brief Replaces a zombie's head with any plant's animation: the Zombotany look, for every plant.
/// Only zombies with a regular head ("anim_head1" track) can wear one.
///
///     zombie->AddComponent<PlantHeadComponent>(SeedType::SEED_MELONPULT);
class PlantHeadComponent : public Component
{
	COMPONENT_TYPE(PlantHeadComponent)

	PlantHeadComponent() = default;
	explicit PlantHeadComponent(SeedType theSeedType, float theOffsetX = 65.0f, float theOffsetY = -5.0f,
	                            float theScale = 1.0f)
	    : mSeedType(theSeedType), mOffsetX(theOffsetX), mOffsetY(theOffsetY), mScale(theScale)
	{
	}

	/// @brief Creates the head animation and hides the zombie's own head
	void OnAttach() override;
	/// @brief Removes the head animation
	void OnDetach() override;
	/// @brief The plant head disappears instead of the zombie's head flying off
	bool OnHeadDropped() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	SeedType mSeedType = SeedType::SEED_NONE;
	float mOffsetX = 65.0f;
	float mOffsetY = -5.0f;
	float mScale = 1.0f;
	ReanimationID mHeadReanimID = ReanimationID::REANIMATIONID_NULL;
};
