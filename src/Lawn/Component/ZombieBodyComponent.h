#pragma once

#include "../../ConstEnums.h"
#include "Component.h"

/// @brief How a zombie's body is drawn and falls apart: its shadow, the head that flies off and its charred
/// remains. Each zombie with a body different from the regular zombie has one; zombies without one use the
/// regular values. Pure data: what *happens* when a zombie loses its head (Pogo stick breaking, flag falling...)
/// lives in its behaviour's ZombieBehaviourComponent::OnLoseHead.
class ZombieBodyComponent : public Component
{
	COMPONENT_TYPE(ZombieBodyComponent)

	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief Shadow offsets for walking left (forward) and right (backwards); the PerScale parts are multiplied by
	/// the zombie's scale (Little Trouble, Big Time). mHasShadowOffsets false: the regular zombie's shadow.
	void SetShadow(float theForwardX, float theBackwardX, float theY, float theScale = 1.0f);

	bool mHasShadowOffsets = false;
	float mShadowForwardX = 0.0f;
	float mShadowForwardXPerScale = 0.0f;
	float mShadowBackwardX = 0.0f;
	float mShadowBackwardXPerScale = 0.0f;
	float mShadowY = 0.0f;
	float mShadowScale = 1.0f;

	/// The particle the head flies off as (PARTICLE_NONE: the regular head, or its mowed/in-pool variants)
	ParticleEffect mHeadParticle = ParticleEffect::PARTICLE_NONE;
	/// ResourceId of the head image the particle shows, -1 for the particle's own image
	int32_t mHeadImageId = -1;
	/// Tracks hidden along with the head (a Dancer's sideburns...)
	char mHeadHiddenPrefixes[2][32] = { "", "" };
	/// Draw order of the flying head relative to the zombie (+1 in front, -1 behind)
	int32_t mHeadRenderOffset = 1;

	ReanimationType mCharredReanim = ReanimationType::REANIM_ZOMBIE_CHARRED;
	float mCharredX = 22.0f;
	float mCharredY = -10.0f;
	/// Extra X when the zombie walks backwards (the Digger's charred remains face the other way)
	float mCharredBackwardsX = 0.0f;
	/// Animation the charred remains play ("" for their default)
	char mCharredAnim[32] = "";
};
