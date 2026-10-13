#pragma once

#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "Component.h"

/// @brief How a zombie's body is drawn, sounds and falls apart: its shadow, the head that flies off, its death
/// animation, its charred remains and where it stands once it reaches the house. Each zombie with a body different from the regular zombie has one; zombies without one use the
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
	/// @brief The same death fall time for every death animation
	void SetDeathFall(float theTime);
	/// @brief A fixed death animation speed instead of a random one
	void SetDeathAnimRate(float theRate);

	static constexpr float kRegularDeathFall = 0.77f;
	static constexpr float kRegularDeathFallAlt = 0.71f;
	static constexpr float kRegularDeathFallSuperLong = 0.788f;

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
	/// Burns where it stands (no charred remains: Bungee, Yeti)
	bool mBurnsInPlace = false;
	/// Fire only hurts it (1800 damage) instead of burning it (Dr. Zomboss)
	bool mBurnsAsDamage = false;

	/// The death animation plays at a random speed between these
	float mDeathAnimRateMin = 24.0f;
	float mDeathAnimRateMax = 30.0f;
	/// When the body hits the ground in the death animation (thud, Daisy mode); false: it never falls
	bool mHasDeathFall = true;
	float mDeathFallTime = kRegularDeathFall;
	float mDeathFallTimeAlt = kRegularDeathFallAlt;             ///< in anim_death2
	float mDeathFallTimeSuperLong = kRegularDeathFallSuperLong; ///< in anim_superlongdeath
	/// Plays its death animation even when killed in a way that leaves no body (Dr. Zomboss)
	bool mPlaysDeathAnimWhenDisintegrated = false;
	/// Goes up in a puff of smoke when mowed instead of going through the mower
	bool mPuffsWhenMowed = false;

	/// Played when it enters the lawn (NUM_FOLEY: none)
	FoleyType mAppearFoley = FoleyType::NUM_FOLEY;
	/// Its looping music, stopped with the zombie's sounds (NUM_FOLEY: none)
	FoleyType mSongFoley = FoleyType::NUM_FOLEY;

	/// Where it stands in the house after eating your brains, relative to the regular zombie
	float mHouseX = 0.0f;
	float mHouseY = 0.0f;
	/// X instead of the regular -80 on the pool lawns
	float mPoolHouseX = -80.0f;
	float mRoofHouseX = 0.0f;
	float mRoofHouseY = 0.0f;
	/// Plays its idle animation in the house
	bool mIdlesInHouse = true;

	/// Speed of the eating animation
	float mEatAnimRate = 36.0f;
	/// When in the eating animation the hands chomp (sound and the plant shaking)
	float mChewTimes[2] = { 0.14f, 0.68f };
	/// When in the walk animation the feet kick up dust (Football), -1 for none
	float mFootDustTimes[2] = { -1.0f, -1.0f };
	/// Walk animation: -1 picks one at random, 0 always anim_walk2 (Flag Zombie, Peashooter zombie)
	int32_t mWalkAnimVariant = -1;
	/// Dances instead of walking with the dance cheat (the plain zombies)
	bool mDancesInDanceMode = false;
	/// Where the body is drawn, relative to the regular zombie
	float mDrawOffsetX = 0.0f;
	float mDrawOffsetY = 0.0f;
};
