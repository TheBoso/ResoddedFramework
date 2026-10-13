#pragma once

#include "../../ConstEnums.h"
#include "../../Resources.h"
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
	ResourceId mHeadImageId = (ResourceId)-1;
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
	/// Leaves a trail of daisies when it dies in Daisy mode, offset from the regular spot
	bool mLeavesDaisies = true;
	float mDaisyX = 0.0f;
	float mDaisyY = 0.0f;
	/// Has the regular zombie head, which can pull the grossed-out face after biting garlic; without it the zombie
	/// just turns away
	bool mHasYuckyFace = true;
	/// Where the body is drawn, relative to the regular zombie
	float mDrawOffsetX = 0.0f;
	float mDrawOffsetY = 0.0f;

	/// Tracks hidden when the arm falls off (matched as prefixes or as whole track names)
	char mArmHiddenTracks[3][48] = { "Zombie_outerarm_lower", "Zombie_outerarm_hand", "" };
	bool mArmHiddenByPrefix = true;
	/// Track the falling arm starts from
	char mArmTrack[48] = "Zombie_outerarm_lower";
	/// The upper arm left behind shows this image (ResourceId, -1 to leave it as it is)
	char mArmUpperTrack[48] = "Zombie_outerarm_upper";
	ResourceId mArmUpperImageId = ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_UPPER2_ID;
	/// Image of the falling arm (ResourceId, -1 for the particle's own)
	ResourceId mArmImageId = (ResourceId)-1;

	/// Shows the ice block when frozen (no for Dr. Zomboss)
	bool mShowsIceTrap = true;
	float mIceTrapX = 0.0f;
	float mIceTrapY = 0.0f;
	float mIceTrapScale = 1.0f;

	/// Kernel-pult butter sticks to it (no for the Zamboni and Dr. Zomboss)
	bool mCanBeButtered = true;
	/// Where the butter splat sits, relative to the head ("" for the head track, anim_head1)
	char mButterHeadTrack[48] = "";
	float mButterX = 0.0f;
	float mButterY = 0.0f;
	float mButterScale = 1.0f;

	// What the zombie spawns with (Zombie::ZombieInitialize); what it does as it appears is in its behaviour's OnSpawn
	int32_t mSpawnHealth = 270;
	/// The regular zombie animation and its layers (cone, bucket, screen door, duck tube, flag...)
	bool mUsesPlainZombieReanim = true;
	char mSpawnShownPrefix[32] = "";
	char mSpawnHiddenPrefixes[3][32] = { "", "", "" };
	/// Animation it starts in on the lawn ("" for its own)
	char mSpawnFrames[32] = "";
	/// Where it can be hit, relative to its position (x, y, width, height)
	int32_t mHitRect[4] = { 36, 0, 42, 115 };
	/// Where it bites (x, y, width, height); without one, the regular zombie's
	bool mHasAttackRect = false;
	int32_t mAttackRect[4] = { 20, 0, 50, 115 };
	int32_t mSpawnWidth = 120;
	int32_t mSpawnHeight = 120;
	/// Sprite animation frames and ticks per frame (0: the regular ones)
	int32_t mAnimFrames = 0;
	int32_t mAnimTicksPerFrame = 0;
	float mScale = 1.0f;
	/// Carries something it can lose (pole, flag, ladder...)
	bool mCarriesObject = false;
	/// Can be the tongue-out variant
	bool mCanHaveTongue = true;
	/// Enters at this x (plus up to mSpawnXRandom) instead of the regular spot
	bool mHasSpawnX = false;
	float mSpawnX = 0.0f;
	int32_t mSpawnXRandom = 0;
	/// Extra x on top of where it enters (the Tall-nut zombie starts further back)
	float mSpawnXOffset = 0.0f;
	RenderLayer mRenderLayer = RenderLayer::RENDER_LAYER_ZOMBIE;
	int32_t mRenderOffset = 4;

	// Smaller rules
	/// Wears a duck tube (on every lawn, not only in the pool)
	bool mHasDuckyTube = false;
	/// Hit by whatever hits any row (Dr. Zomboss spans the whole lawn)
	bool mSpansAllRows = false;
	/// Groans now and then
	bool mGroans = true;
	/// Health lost per decay tick once it lost its head (bigger zombies: 3 whatever this says)
	int32_t mHeadlessDecay = 1;
	/// Offset of the bits flying off when it chews a Wall-nut
	float mChewEffectX = 0.0f;
	float mChewEffectY = 0.0f;
	/// Moved down this much at scale 0 (Little Trouble, Big Time: keeps the Football Zombie on the ground)
	float mSmallScaleOffsetY = 0.0f;
	/// Plays the regular eating animation (no for the Snorkel, which surfaces to eat)
	bool mPlaysEatAnim = true;
	/// Eats plants it walks into
	bool mEatsPlants = true;
	/// Climbs ladders placed on plants
	bool mClimbsLadders = true;
	/// The level's boss: on the final level only it drops the award, which flies out of it
	bool mIsBoss = false;
	/// Up/down shift of its row position (fliers and bouncers sit higher)
	float mRowYOffset = 0.0f;
	/// Loses its arm and head as it takes damage
	bool mLosesBodyParts = true;
	/// The shadow stays on the ground while it is in the air
	bool mShadowFollowsAltitude = false;
	/// Without a body animation, keeps moving instead of stepping with its sprite frames
	bool mSpriteGlides = false;
	/// Damage states: damaged below health * [0] / [1], badly damaged below health * [0] / [1]
	int32_t mDamagedBelow[2] = { 2, 3 };
	int32_t mBadlyDamagedBelow[2] = { 1, 3 };

	/// The Zombotany head: a plant animation attached to one of the zombie's tracks (REANIM_NONE: none)
	ReanimationType mPlantHeadReanim = ReanimationType::REANIM_NONE;
	char mPlantHeadAnim[32] = "anim_idle";
	char mPlantHeadTrack[32] = "anim_head1";
	/// Blanks the track's own image (a head track carrying the plant head)
	bool mPlantHeadBlanksTrack = true;
	float mPlantHeadX = 0.0f;
	float mPlantHeadY = 0.0f;
	float mPlantHeadScale = 1.0f;
};
