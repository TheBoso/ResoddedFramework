#include "PlantHeadComponent.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/Attachment.h"
#include "../../Sexy.TodLib/Reanimator.h"

REGISTER_COMPONENT(PlantHeadComponent)

void PlantHeadComponent::OnAttach()
{
	Zombie *aZombie = GetZombie();
	if (aZombie == nullptr || mSeedType < 0 || mSeedType >= SeedType::NUM_SEED_TYPES)
		return;

	ReanimationType aReanimType = GetPlantDefinition(mSeedType).mReanimationType;
	Reanimation *aBodyReanim = GetApp()->ReanimationTryToGet(aZombie->mBodyReanimID);
	if (aReanimType == ReanimationType::REANIM_NONE || aBodyReanim == nullptr ||
	    !aBodyReanim->TrackExists("anim_head1"))
		return;

	// Hide the zombie's own head, as the Peashooter Zombie does
	aZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_HIDDEN);
	aZombie->ReanimShowPrefix("anim_head2", RENDER_GROUP_HIDDEN);
	ReanimatorTrackInstance *aHeadTrack = aBodyReanim->GetTrackInstanceByName("anim_head1");
	aHeadTrack->mImageOverride = IMAGE_BLANK;

	Reanimation *aHeadReanim = GetApp()->AddReanimation(0.0f, 0.0f, 0, aReanimType);
	const char *anIdle = aHeadReanim->TrackExists("anim_head_idle") ? "anim_head_idle" : "anim_idle";
	if (aHeadReanim->TrackExists(anIdle))
	{
		aHeadReanim->PlayReanim(anIdle, ReanimLoopType::REANIM_LOOP, 0, 15.0f);
	}

	AttachEffect *anAttachEffect = AttachReanim(aHeadTrack->mAttachmentID, aHeadReanim, 0.0f, 0.0f);
	aBodyReanim->mFrameBasePose = 0;
	// Plants face right and zombies face left, so the head is mirrored
	TodScaleRotateTransformMatrix(anAttachEffect->mOffset, mOffsetX, mOffsetY, 0.2f, -mScale, mScale);
	mHeadReanimID = GetApp()->ReanimationGetID(aHeadReanim);
}

void PlantHeadComponent::OnDetach()
{
	if (GetApp())
	{
		GetApp()->RemoveReanimation(mHeadReanimID);
	}
	mHeadReanimID = ReanimationID::REANIMATIONID_NULL;
}

bool PlantHeadComponent::OnHeadDropped()
{
	GetApp()->RemoveReanimation(mHeadReanimID);
	mHeadReanimID = ReanimationID::REANIMATIONID_NULL;
	return true;
}

void PlantHeadComponent::Serialize(ComponentArchive &theArchive)
{
	theArchive.Value("seedType", mSeedType);
	theArchive.Value("offsetX", mOffsetX);
	theArchive.Value("offsetY", mOffsetY);
	theArchive.Value("scale", mScale);
	theArchive.Value("headReanimID", mHeadReanimID);
}
