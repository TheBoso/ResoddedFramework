#include <cstring>
#include "Archetypes.h"
#include "../../Resources.h"
#include "../../GameConstants.h"
#include "../Board.h"

// Plants whose fire Plant::Fire turns into a projectile. Fume/Gloom-shroom, Cob Cannon targeting and the
// Threepeater/Starfruit spreads keep their special code in Plant.cpp; the entries below are also what another
// entity copies when it "shoots like" the plant.
static const PlantShotDefinition gPlantShotDefs[] = {
	{ SeedType::SEED_PEASHOOTER,    ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_REPEATER,      ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 2, 0   },
	{ SeedType::SEED_THREEPEATER,   ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_SPLITPEA,      ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_GATLINGPEA,    ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 4, 0   },
	{ SeedType::SEED_LEFTPEATER,    ProjectileType::PROJECTILE_PEA,         ShotArc::Straight, 2, 0   },
	{ SeedType::SEED_SNOWPEA,       ProjectileType::PROJECTILE_SNOWPEA,     ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_PUFFSHROOM,    ProjectileType::PROJECTILE_PUFF,        ShotArc::Straight, 1, 290 },
	{ SeedType::SEED_SCAREDYSHROOM, ProjectileType::PROJECTILE_PUFF,        ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_SEASHROOM,     ProjectileType::PROJECTILE_PUFF,        ShotArc::Straight, 1, 290 },
	{ SeedType::SEED_CACTUS,        ProjectileType::PROJECTILE_SPIKE,       ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_CATTAIL,       ProjectileType::PROJECTILE_SPIKE,       ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_STARFRUIT,     ProjectileType::PROJECTILE_STAR,        ShotArc::Straight, 1, 0   },
	{ SeedType::SEED_CABBAGEPULT,   ProjectileType::PROJECTILE_CABBAGE,     ShotArc::Lobbed,   1, 0   },
	{ SeedType::SEED_KERNELPULT,    ProjectileType::PROJECTILE_KERNEL,      ShotArc::Lobbed,   1, 0   },
	{ SeedType::SEED_MELONPULT,     ProjectileType::PROJECTILE_MELON,       ShotArc::Lobbed,   1, 0   },
	{ SeedType::SEED_WINTERMELON,   ProjectileType::PROJECTILE_WINTERMELON, ShotArc::Lobbed,   1, 0   },
	{ SeedType::SEED_COBCANNON,     ProjectileType::PROJECTILE_COBBIG,      ShotArc::Lobbed,   1, 0   },
};

// Straight shots pass over these (used to be a hardcoded list in Projectile::FindCollisionTargetPlant)
static const SeedType gLowProfilePlants[] = {
	SeedType::SEED_PUFFSHROOM,
	SeedType::SEED_SUNSHROOM,
	SeedType::SEED_POTATOMINE,
	SeedType::SEED_SPIKEWEED,
	SeedType::SEED_SPIKEROCK,
	SeedType::SEED_LILYPAD,
};

// Projectiles fired by zombies. Only used to default old saves: every spawner assigns its own team.
static const ProjectileType gZombieTeamProjectiles[] = {
	ProjectileType::PROJECTILE_ZOMBIE_PEA,
	ProjectileType::PROJECTILE_BASKETBALL,
};

template <typename T, size_t N>
static bool Contains(const T (&theList)[N], T theValue)
{
	for (const T &anEntry : theList)
	{
		if (anEntry == theValue)
			return true;
	}
	return false;
}

/// @brief The producer a vanilla plant uses, or nullptr
static ProducerComponent *CreateVanillaProducer(SeedType theSeedType, int theRate)
{
	switch (theSeedType)
	{
	case SeedType::SEED_SUNFLOWER: {
		auto *aProducer = new ProducerComponent(CoinType::COIN_SUN, 1, theRate);
		aProducer->mBigTimeBonusCoin = CoinType::COIN_SUN;
		return aProducer;
	}
	case SeedType::SEED_TWINSUNFLOWER:
		return new ProducerComponent(CoinType::COIN_SUN, 2, theRate);
	case SeedType::SEED_SUNSHROOM:
		return new SunShroomProducerComponent(theRate);
	case SeedType::SEED_MARIGOLD: {
		auto *aProducer = new ProducerComponent(CoinType::COIN_SILVER, 1, theRate);
		aProducer->mCoinMotion = CoinMotion::COIN_MOTION_COIN;
		aProducer->mRareCoinType = CoinType::COIN_GOLD;
		aProducer->mRareChance = 10;
		aProducer->mBigTimeBonusCoin = CoinType::COIN_SILVER;
		aProducer->mStopsAfterFinalWave = true;
		return aProducer;
	}
	default:
		return nullptr;
	}
}

/// @brief The behaviour components of the vanilla plants (what used to be the seed switches in Plant::UpdateAbilities
/// and Plant::DoSpecial)
static void AddVanillaBehaviours(Plant *thePlant)
{
	switch (thePlant->mSeedType)
	{
	case SeedType::SEED_CHERRYBOMB:
		thePlant->AddComponent(ExplodeComponent::CherryBomb());
		break;
	case SeedType::SEED_DOOMSHROOM:
		thePlant->AddComponent<DoomFuseComponent>(100);
		thePlant->AddComponent(ExplodeComponent::DoomShroom());
		break;
	case SeedType::SEED_POTATOMINE:
		thePlant->AddComponent<PotatoMineArmingBehaviour>();
		thePlant->AddComponent(ExplodeComponent::PotatoMine());
		break;
	case SeedType::SEED_JALAPENO:
		thePlant->AddComponent<BurnRowComponent>(0);
		break;
	case SeedType::SEED_ICESHROOM:
		thePlant->AddComponent<FuseComponent>(100);
		thePlant->AddComponent<FreezeAllComponent>();
		break;
	case SeedType::SEED_SQUASH:
		thePlant->AddComponent<SquashBehaviour>();
		break;
	case SeedType::SEED_CHOMPER:
		thePlant->AddComponent<ChomperBehaviour>();
		break;
	case SeedType::SEED_BLOVER:
		thePlant->AddComponent<BloverBehaviour>();
		break;
	case SeedType::SEED_FLOWERPOT:
		thePlant->AddComponent<FlowerPotBehaviour>();
		break;
	case SeedType::SEED_LILYPAD:
		thePlant->AddComponent<LilyPadBehaviour>();
		break;
	case SeedType::SEED_IMITATER:
		thePlant->AddComponent<ImitaterBehaviour>();
		break;
	case SeedType::SEED_INSTANT_COFFEE:
		thePlant->AddComponent<CoffeeBeanBehaviour>();
		break;
	case SeedType::SEED_UMBRELLA:
		thePlant->AddComponent<UmbrellaBehaviour>();
		break;
	case SeedType::SEED_COBCANNON:
		thePlant->AddComponent<CobCannonBehaviour>();
		break;
	case SeedType::SEED_CACTUS:
		thePlant->AddComponent<CactusBehaviour>();
		break;
	case SeedType::SEED_MAGNETSHROOM:
		thePlant->AddComponent<MagnetShroomBehaviour>();
		break;
	case SeedType::SEED_GOLD_MAGNET:
		thePlant->AddComponent<GoldMagnetBehaviour>();
		break;
	case SeedType::SEED_SUNSHROOM:
		thePlant->AddComponent<SunShroomGrowthBehaviour>();
		break;
	case SeedType::SEED_GRAVEBUSTER:
		thePlant->AddComponent<GraveBusterBehaviour>();
		break;
	case SeedType::SEED_SPIKEWEED:
		thePlant->AddComponent<SpikeweedBehaviour>(20, 75, -1);
		break;
	case SeedType::SEED_SPIKEROCK:
		thePlant->AddComponent<SpikeweedBehaviour>(20, 69, 33);
		break;
	case SeedType::SEED_TANGLEKELP:
		thePlant->AddComponent<TangleKelpBehaviour>();
		break;
	case SeedType::SEED_SCAREDYSHROOM:
		thePlant->AddComponent<ScaredyShroomBehaviour>();
		break;
	default:
		break;
	}
}

void Archetypes::ApplyPlantDefaults(Plant *thePlant, bool theFromOldSave)
{
	if (!thePlant->GetEntityRef())
		return;

	thePlant->AddComponent<TeamComponent>(TEAM_PLANTS);
	thePlant->AddComponent<FacingComponent>(1);
	thePlant->AddComponent<HypnotizableComponent>(TEAM_ZOMBIES, true);

	if (Contains(gLowProfilePlants, thePlant->mSeedType))
	{
		thePlant->AddComponent<LowProfileComponent>();
	}
	AddVanillaBehaviours(thePlant);
	if (PlantShooterComponent *aShooter = PlantShooterComponent::CreateVanilla(thePlant->mSeedType))
	{
		thePlant->AddComponent(aShooter);
	}

	switch (thePlant->mSeedType)
	{
	case SeedType::SEED_HYPNOSHROOM:
		thePlant->AddComponent<HypnotizeOnEatenComponent>(true);
		thePlant->AddComponent<InedibleComponent>(true, false);
		break;
	case SeedType::SEED_CHERRYBOMB:
	case SeedType::SEED_JALAPENO:
	case SeedType::SEED_DOOMSHROOM:
		thePlant->AddComponent<InedibleComponent>(true, true);
		break;
	case SeedType::SEED_ICESHROOM:
		thePlant->AddComponent<InedibleComponent>(true, true);
		thePlant->AddComponent<ActivatesWhenEatenComponent>(true);
		break;
	case SeedType::SEED_BLOVER:
		thePlant->AddComponent<InedibleComponent>(false, true);
		thePlant->AddComponent<ActivatesWhenEatenComponent>(false);
		break;
	case SeedType::SEED_SQUASH:
		thePlant->AddComponent<InedibleComponent>(false, true);
		break;
	case SeedType::SEED_TANGLEKELP:
		thePlant->AddComponent<InedibleComponent>(false, false, true);
		break;
	case SeedType::SEED_POTATOMINE:
		thePlant->AddComponent<MineComponent>();
		break;
	case SeedType::SEED_GARLIC:
		thePlant->AddComponent<RepelOnEatenComponent>();
		break;
	case SeedType::SEED_SPIKEWEED:
		thePlant->AddComponent<SpikesComponent>(false);
		break;
	case SeedType::SEED_SPIKEROCK:
		thePlant->AddComponent<SpikesComponent>(true);
		break;
	case SeedType::SEED_WALLNUT:
		thePlant->AddComponent<WallComponent>(true);
		break;
	case SeedType::SEED_TALLNUT:
		thePlant->AddComponent<WallComponent>(true);
		thePlant->AddComponent<TallComponent>();
		break;
	case SeedType::SEED_PUMPKINSHELL:
		thePlant->AddComponent<WallComponent>(false);
		break;
	case SeedType::SEED_TORCHWOOD:
		thePlant->AddComponent<ProjectileConverterComponent>()
		    ->Convert(ProjectileType::PROJECTILE_PEA, ProjectileType::PROJECTILE_FIREBALL)
		    .Convert(ProjectileType::PROJECTILE_SNOWPEA, ProjectileType::PROJECTILE_PEA);
		break;
	default:
		break;
	}

	if (ProducerComponent *aProducer = CreateVanillaProducer(thePlant->mSeedType, thePlant->mLaunchRate))
	{
		if (theFromOldSave)
		{
			// Old saves kept the countdown in the plant itself
			aProducer->mCountdown = std::max(thePlant->mLaunchCounter, 1);
		}
		thePlant->AddComponent(aProducer);
	}
}

/// @brief The behaviour components of the vanilla zombies (what used to be the zombie-type dispatch in
/// Zombie::UpdateActions, UpdatePlaying and Update)
static void AddVanillaZombieBehaviours(Zombie *theZombie)
{
	switch (theZombie->mZombieType)
	{
	case ZombieType::ZOMBIE_POLEVAULTER:
		theZombie->AddComponent<PolevaulterBehaviour>();
		break;
	case ZombieType::ZOMBIE_CATAPULT:
		theZombie->AddComponent<CatapultBehaviour>();
		break;
	case ZombieType::ZOMBIE_DOLPHIN_RIDER:
		theZombie->AddComponent<DolphinRiderBehaviour>();
		break;
	case ZombieType::ZOMBIE_SNORKEL:
		theZombie->AddComponent<SnorkelBehaviour>();
		break;
	case ZombieType::ZOMBIE_BALLOON:
		theZombie->AddComponent<BalloonBehaviour>();
		break;
	case ZombieType::ZOMBIE_NEWSPAPER:
		theZombie->AddComponent<NewspaperBehaviour>();
		break;
	case ZombieType::ZOMBIE_DIGGER:
		theZombie->AddComponent<DiggerBehaviour>();
		break;
	case ZombieType::ZOMBIE_JACK_IN_THE_BOX:
		theZombie->AddComponent<JackInTheBoxBehaviour>();
		break;
	case ZombieType::ZOMBIE_GARGANTUAR:
	case ZombieType::ZOMBIE_REDEYE_GARGANTUAR:
		theZombie->AddComponent<GargantuarBehaviour>();
		break;
	case ZombieType::ZOMBIE_BOBSLED:
		theZombie->AddComponent<BobsledBehaviour>();
		break;
	case ZombieType::ZOMBIE_ZAMBONI:
		theZombie->AddComponent<ZamboniBehaviour>();
		break;
	case ZombieType::ZOMBIE_LADDER:
		theZombie->AddComponent<LadderBehaviour>();
		break;
	case ZombieType::ZOMBIE_YETI:
		theZombie->AddComponent<YetiBehaviour>();
		break;
	case ZombieType::ZOMBIE_DANCER:
		theZombie->AddComponent<DancerBehaviour>();
		break;
	case ZombieType::ZOMBIE_BACKUP_DANCER:
		theZombie->AddComponent<BackupDancerBehaviour>();
		break;
	case ZombieType::ZOMBIE_IMP:
		theZombie->AddComponent<ImpBehaviour>();
		break;
	case ZombieType::ZOMBIE_PEA_HEAD:
		theZombie->AddComponent<PeaHeadBehaviour>();
		break;
	case ZombieType::ZOMBIE_JALAPENO_HEAD:
		theZombie->AddComponent<JalapenoHeadBehaviour>();
		break;
	case ZombieType::ZOMBIE_GATLING_HEAD:
		theZombie->AddComponent<GatlingHeadBehaviour>();
		break;
	case ZombieType::ZOMBIE_SQUASH_HEAD:
		theZombie->AddComponent<SquashHeadBehaviour>();
		break;
	case ZombieType::ZOMBIE_BOSS:
		theZombie->AddComponent<BossBehaviour>();
		break;
	case ZombieType::ZOMBIE_BUNGEE:
		theZombie->AddComponent<BungeeBehaviour>();
		break;
	case ZombieType::ZOMBIE_POGO:
		theZombie->AddComponent<PogoBehaviour>();
		break;
	case ZombieType::ZOMBIE_FLAG:
		theZombie->AddComponent<FlagBearerBehaviour>();
		break;
	default:
		break;
	}
}

/// @brief Traits and walking of the vanilla zombies (what used to be lists of zombie types)
static void AddVanillaZombieTraits(Zombie *theZombie)
{
	constexpr float kMin = WalkerComponent::kRegularMinSpeed;
	constexpr float kMax = WalkerComponent::kRegularMaxSpeed;
	switch (theZombie->mZombieType)
	{
	case ZombieType::ZOMBIE_GARGANTUAR:
	case ZombieType::ZOMBIE_REDEYE_GARGANTUAR:
		theZombie->AddComponent<GiantComponent>();
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -150);
		break;
	case ZombieType::ZOMBIE_POLEVAULTER:
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -150);
		break;
	case ZombieType::ZOMBIE_ZAMBONI:
		theZombie->AddComponent<VehicleComponent>(true, 200);
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -175)->mMoveStyle = MoveStyle::Drive;
		break;
	case ZombieType::ZOMBIE_CATAPULT:
		theZombie->AddComponent<VehicleComponent>(false, 200);
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -175)->mMoveStyle = MoveStyle::Glide;
		break;
	case ZombieType::ZOMBIE_FOOTBALL:
		theZombie->AddComponent<WalkerComponent>(0.66f, 0.68f, -175);
		break;
	case ZombieType::ZOMBIE_DANCER:
	case ZombieType::ZOMBIE_BACKUP_DANCER:
		theZombie->AddComponent<DanceTroupeComponent>(theZombie->mZombieType == ZombieType::ZOMBIE_DANCER);
		theZombie->AddComponent<WalkerComponent>(0.45f, 0.45f, -130);
		break;
	case ZombieType::ZOMBIE_SNORKEL:
		theZombie->AddComponent<SwimmerComponent>(true);
		theZombie->AddComponent<WalkerComponent>(0.66f, 0.68f, -130);
		break;
	case ZombieType::ZOMBIE_DOLPHIN_RIDER:
		theZombie->AddComponent<SwimmerComponent>();
		break;
	case ZombieType::ZOMBIE_POGO:
	case ZombieType::ZOMBIE_FLAG:
		theZombie->AddComponent<WalkerComponent>(0.45f, 0.45f, WalkerComponent::kRegularHouseEdgeX);
		break;
	case ZombieType::ZOMBIE_JACK_IN_THE_BOX:
		theZombie->AddComponent<WalkerComponent>(0.66f, 0.68f, WalkerComponent::kRegularHouseEdgeX);
		break;
	case ZombieType::ZOMBIE_SQUASH_HEAD:
		theZombie->AddComponent<WalkerComponent>(0.79f, 0.81f, WalkerComponent::kRegularHouseEdgeX);
		break;
	case ZombieType::ZOMBIE_YETI:
		theZombie->AddComponent<WalkerComponent>(0.4f, 0.4f, WalkerComponent::kRegularHouseEdgeX);
		break;
	case ZombieType::ZOMBIE_BUNGEE:
	case ZombieType::ZOMBIE_BOSS:
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, WalkerComponent::kRegularHouseEdgeX)->mMoveStyle =
		    MoveStyle::Stand;
		break;
	default:
		break;
	}
}

/// @brief Copy a track name into a fixed-size name field
template <size_t N>
static void SetName(char (&theDest)[N], const char *theName)
{
	strncpy(theDest, theName, N - 1);
	theDest[N - 1] = '\0';
}

/// @brief The bodies of the vanilla zombies that differ from the regular zombie (shadow, flying head, death
/// animation, charred remains, sounds, place in the house, walking and eating animation, lost arm, ice and butter, yucky face, daisies, what it spawns with)
static void AddVanillaZombieBody(Zombie *theZombie)
{
	ZombieBodyComponent aBody;
	auto aSetHead = [&aBody](int theImageId, const char *theHidden1 = "", const char *theHidden2 = "") {
		aBody.mHeadImageId = theImageId;
		strncpy(aBody.mHeadHiddenPrefixes[0], theHidden1, sizeof(aBody.mHeadHiddenPrefixes[0]) - 1);
		strncpy(aBody.mHeadHiddenPrefixes[1], theHidden2, sizeof(aBody.mHeadHiddenPrefixes[1]) - 1);
	};
	auto aSetArm = [&aBody](const char *theTrack, const char *theUpperTrack, int theUpperImageId, int theImageId) {
		SetName(aBody.mArmTrack, theTrack);
		SetName(aBody.mArmUpperTrack, theUpperTrack);
		aBody.mArmUpperImageId = theUpperImageId;
		aBody.mArmImageId = theImageId;
	};
	auto aHideArmTracks = [&aBody](const char *theTrack1, const char *theTrack2, const char *theTrack3 = "") {
		aBody.mArmHiddenByPrefix = false;
		SetName(aBody.mArmHiddenTracks[0], theTrack1);
		SetName(aBody.mArmHiddenTracks[1], theTrack2);
		SetName(aBody.mArmHiddenTracks[2], theTrack3);
	};
	auto aSetIceTrap = [&aBody](float theX, float theY, float theScale = 1.0f) {
		aBody.mIceTrapX = theX;
		aBody.mIceTrapY = theY;
		aBody.mIceTrapScale = theScale;
	};
	auto aSetButter = [&aBody](float theX, float theY, float theScale = 1.0f) {
		aBody.mButterX = theX;
		aBody.mButterY = theY;
		aBody.mButterScale = theScale;
	};
	auto aSetRect = [](int32_t(&theRect)[4], int theX, int theY, int theWidth, int theHeight) {
		theRect[0] = theX;
		theRect[1] = theY;
		theRect[2] = theWidth;
		theRect[3] = theHeight;
	};
	auto aSetAttackRect = [&aBody, &aSetRect](int theX, int theY, int theWidth, int theHeight) {
		aBody.mHasAttackRect = true;
		aSetRect(aBody.mAttackRect, theX, theY, theWidth, theHeight);
	};
	auto aSetSpawnX = [&aBody](float theX, int theRandom) {
		aBody.mHasSpawnX = true;
		aBody.mSpawnX = theX;
		aBody.mSpawnXRandom = theRandom;
	};
	auto aHidePrefixes = [&aBody](const char *thePrefix1, const char *thePrefix2 = "", const char *thePrefix3 = "") {
		SetName(aBody.mSpawnHiddenPrefixes[0], thePrefix1);
		SetName(aBody.mSpawnHiddenPrefixes[1], thePrefix2);
		SetName(aBody.mSpawnHiddenPrefixes[2], thePrefix3);
	};
	auto aSetPlantHead = [&aBody](ReanimationType theReanim, const char *theAnim, const char *theTrack, float theX,
	                              float theY, float theScale) {
		aBody.mPlantHeadReanim = theReanim;
		SetName(aBody.mPlantHeadAnim, theAnim);
		SetName(aBody.mPlantHeadTrack, theTrack);
		aBody.mPlantHeadBlanksTrack = strcmp(theTrack, "anim_head1") == 0;
		aBody.mPlantHeadX = theX;
		aBody.mPlantHeadY = theY;
		aBody.mPlantHeadScale = theScale;
	};
	auto aSetCharred = [&aBody](ReanimationType theReanim, float theX, float theY) {
		aBody.mCharredReanim = theReanim;
		aBody.mCharredX = theX;
		aBody.mCharredY = theY;
	};

	switch (theZombie->mZombieType)
	{
	case ZombieType::ZOMBIE_FOOTBALL:
		aBody.mSmallScaleOffsetY = 20.0f;
		aBody.mUsesPlainZombieReanim = false;
		aSetRect(aBody.mHitRect, 50, 0, 57, 115);
		aHidePrefixes("anim_hair");
		aBody.mAnimTicksPerFrame = 6;
		aBody.mCanHaveTongue = false;
		aBody.mDaisyX = 160.0f;
		aBody.mHasYuckyFace = false;
		SetName(aBody.mArmHiddenTracks[0], "Zombie_football_leftarm_lower");
		SetName(aBody.mArmHiddenTracks[1], "Zombie_football_leftarm_hand");
		aSetArm("Zombie_football_leftarm_hand", "Zombie_football_leftarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_FOOTBALL_LEFTARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_FOOTBALL_LEFTARM_HAND_ID);
		aBody.mFootDustTimes[0] = 0.03f;
		aBody.mFootDustTimes[1] = 0.61f;
		aBody.mDrawOffsetY = -16.0f;
		aBody.SetDeathFall(0.52f);
		aBody.SetDeathAnimRate(24.0f);
		aBody.mPoolHouseX = -10.0f;
		aBody.mRoofHouseX = -14.0f;
		aBody.SetShadow(20.0f, 0.0f, 16.0f);
		aBody.mShadowForwardXPerScale = 21.0f;
		aBody.mShadowBackwardXPerScale = -11.0f;
		aSetHead(ResourceId::IMAGE_ZOMBIEFOOTBALLHEAD_ID);
		break;
	case ZombieType::ZOMBIE_NEWSPAPER:
		aBody.mUsesPlainZombieReanim = false;
		aSetAttackRect(20, 0, 50, 115);
		aBody.mCanHaveTongue = false;
		aHideArmTracks("Zombie_paper_hands", "Zombie_paper_leftarm_lower");
		aSetArm("Zombie_paper_leftarm_lower", "Zombie_paper_leftarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_PAPER_LEFTARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_PAPER_LEFTARM_LOWER_ID);
		aBody.mChewTimes[0] = 0.42f;
		aBody.mChewTimes[1] = 0.42f;
		aBody.SetDeathFall(0.63f);
		aBody.SetShadow(29.0f, 5.0f, 4.0f);
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_NEWSPAPER_HEAD;
		break;
	case ZombieType::ZOMBIE_POLEVAULTER:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 500;
		aBody.mAnimTicksPerFrame = 6;
		aBody.mCarriesObject = true;
		aBody.mCanHaveTongue = false;
		aSetSpawnX(WIDE_BOARD_WIDTH + 70, 10);
		aHideArmTracks("Zombie_polevaulter_outerarm_lower", "Zombie_outerarm_hand");
		aSetArm("Zombie_polevaulter_outerarm_lower", "Zombie_polevaulter_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_POLEVAULTER_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_HAND_ID);
		aBody.mEatAnimRate = 20.0f;
		aBody.mChewTimes[0] = 0.38f;
		aBody.mChewTimes[1] = 0.8f;
		aBody.mDrawOffsetX = -6.0f;
		aBody.mDrawOffsetY = -11.0f;
		aBody.SetDeathFall(0.68f);
		aBody.mIdlesInHouse = false;
		aBody.SetShadow(36.0f, -5.0f, 11.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEPOLEVAULTERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_BOBSLED:
		aBody.mSpriteGlides = true;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mRenderOffset = 3;
		aBody.mCanHaveTongue = false;
		aBody.mLeavesDaisies = false;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_dolphinrider_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_BOBSLED_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_BOBSLED_OUTERARM_HAND_ID);
		aBody.mChewTimes[0] = 0.33f;
		aBody.mChewTimes[1] = 0.83f;
		aBody.mDrawOffsetY = -12.0f;
		aBody.SetDeathFall(0.81f);
		aBody.SetShadow(20.0f, 13.0f, 13.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEBOBSLEDHEAD_ID);
		break;
	case ZombieType::ZOMBIE_IMP:
		aBody.mChewEffectX = 24.0f;
		aBody.mChewEffectY = 40.0f;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_imp_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_IMP_ARM1_BONE_ID, ResourceId::IMAGE_REANIM_ZOMBIE_IMP_ARM2_ID);
		aBody.mEatAnimRate = 20.0f;
		aBody.mChewTimes[0] = 0.33f;
		aBody.mChewTimes[1] = 0.79f;
		aBody.mHasDeathFall = false;
		aBody.mPuffsWhenMowed = true;
		aBody.SetShadow(25.0f, 13.0f, 7.0f, 0.6f);
		aSetHead(ResourceId::IMAGE_ZOMBIEIMPHEAD_ID);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_IMP, 16.0f, -10.0f);
		break;
	case ZombieType::ZOMBIE_DIGGER:
		aBody.mClimbsLadders = false;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mCanHaveTongue = false;
		aBody.mCarriesObject = true;
		aSetRect(aBody.mHitRect, 50, 0, 28, 115);
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_digger_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_DIGGER_OUTERARM_UPPER2_ID, ResourceId::IMAGE_ZOMBIEDIGGERARM_ID);
		aSetIceTrap(-27.0f, 0.0f);
		aBody.mEatAnimRate = 20.0f;
		aBody.SetDeathFall(0.85f);
		aBody.SetDeathAnimRate(18.0f);
		aBody.mPuffsWhenMowed = true;
		aBody.mSongFoley = FoleyType::FOLEY_DIGGER;
		aBody.SetShadow(17.0f, 14.0f, 5.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEDIGGERHEAD_ID);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_DIGGER, 22.0f, -10.0f);
		aBody.mCharredBackwardsX = 14.0f;
		strcpy(aBody.mCharredAnim, "anim_crumble");
		break;
	case ZombieType::ZOMBIE_SNORKEL:
		aBody.mPlaysEatAnim = false;
		aBody.mUsesPlainZombieReanim = false;
		aSetRect(aBody.mHitRect, 12, 0, 62, 115);
		aSetAttackRect(-5, 0, 55, 115);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_snorkle_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_SNORKLE_OUTERARM_UPPER2_ID, -1);
		aBody.mEatAnimRate = 20.0f;
		aBody.mHasDeathFall = false;
		aBody.SetDeathAnimRate(14.0f);
		aBody.SetShadow(35.0f, -2.0f, 5.0f);
		aSetHead(ResourceId::IMAGE_REANIM_ZOMBIE_SNORKLE_HEAD_ID);
		break;
	case ZombieType::ZOMBIE_DOLPHIN_RIDER:
		aBody.mSpriteGlides = true;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 500;
		aBody.mAnimTicksPerFrame = 6;
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_dolphinrider_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_DOLPHINRIDER_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_HAND_ID);
		aBody.mHasDeathFall = false;
		aBody.mPuffsWhenMowed = true;
		aBody.mAppearFoley = FoleyType::FOLEY_DOLPHIN_APPEARS;
		aBody.SetShadow(19.0f, 15.0f, 11.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEDOLPHINRIDERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_YETI:
		aBody.mHeadlessDecay = 10;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 1350;
		aBody.mCarriesObject = true;
		aSetAttackRect(20, 0, 50, 115);
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_hand", "Zombie_yeti_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_YETI_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_YETI_OUTERARM_HAND_ID);
		aBody.mEatAnimRate = 20.0f;
		aBody.mDrawOffsetY = -20.0f;
		aBody.SetDeathFall(0.68f);
		aBody.SetDeathAnimRate(14.0f);
		aBody.mPuffsWhenMowed = true;
		aBody.mBurnsInPlace = true;
		aBody.SetShadow(3.0f, 20.0f, 20.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEYETIHEAD_ID);
		break;
	case ZombieType::ZOMBIE_GARGANTUAR:
	case ZombieType::ZOMBIE_REDEYE_GARGANTUAR:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnWidth = 180;
		aBody.mSpawnHeight = 180;
		aBody.mSpawnHealth = theZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR ? 6000 : 3000;
		aBody.mAnimFrames = 24;
		aBody.mAnimTicksPerFrame = 8;
		aSetSpawnX(WIDE_BOARD_WIDTH + 45, 10);
		aSetRect(aBody.mHitRect, -17, -38, 125, 154);
		aSetAttackRect(-30, -38, 89, 154);
		aBody.mCanHaveTongue = false;
		aBody.mRenderOffset = 8;
		aBody.mCarriesObject = true;
		aBody.mHasYuckyFace = false;
		aSetIceTrap(-20.0f, -7.0f, 1.6f);
		aSetButter(-5.0f, -15.0f, 1.2f);
		aBody.mDrawOffsetY = -8.0f;
		aBody.SetDeathFall(0.86f);
		aBody.SetDeathAnimRate(14.0f);
		aBody.mHouseY = 30.0f;
		aBody.mRoofHouseY = 5.0f;
		aBody.SetShadow(27.0f, 27.0f, 7.0f, 1.5f);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_GARGANTUAR, 7.0f, -20.0f);
		break;
	case ZombieType::ZOMBIE_BALLOON:
		aBody.mChewEffectY = 47.0f;
		aBody.mRowYOffset = -30.0f;
		aBody.mSpriteGlides = true;
		aBody.mUsesPlainZombieReanim = false;
		aSetRect(aBody.mHitRect, 36, 30, 42, 115);
		aSetAttackRect(20, 30, 50, 115);
		aBody.mCanHaveTongue = false;
		aBody.mDaisyX = 110.0f;
		aBody.mDaisyY = 30.0f;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_BALLOON_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_HAND_ID);
		aSetIceTrap(-9.0f, 27.0f);
		aBody.mEatAnimRate = 20.0f;
		aBody.mDrawOffsetY = 17.0f;
		aBody.SetDeathFall(0.68f);
		aBody.mAppearFoley = FoleyType::FOLEY_BALLOONINFLATE;
		aBody.SetShadow(23.0f, 11.0f, 13.0f);
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_BALLOON_HEAD;
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED, 22.0f, 21.0f);
		break;
	case ZombieType::ZOMBIE_DANCER:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mScale = 0.8f;
		aBody.mSpawnHealth = 500;
		aBody.mCanHaveTongue = false;
		aBody.mDaisyX = 160.0f;
		aHideArmTracks("Zombie_disco_outerarm_lower", "Zombie_disco_outerhand_point", "Zombie_disco_outerhand");
		aSetArm("Zombie_disco_outerarm_lower", "Zombie_disco_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_DISCO_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_DISCO_OUTERARM_HAND_ID);
		aBody.SetDeathFall(0.83f);
		aSetHead(ResourceId::IMAGE_ZOMBIEDANCERHEAD_ID, "Zombie_disco_chops", "Zombie_disco_glasses");
		aBody.mHeadRenderOffset = -1;
		break;
	case ZombieType::ZOMBIE_BACKUP_DANCER:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mScale = 0.8f;
		aBody.mCanHaveTongue = false;
		aBody.mDaisyX = 160.0f;
		aHideArmTracks("Zombie_disco_outerarm_lower", "Zombie_disco_outerhand");
		aSetArm("Zombie_disco_outerarm_lower", "", -1, ResourceId::IMAGE_REANIM_ZOMBIE_BACKUP_INNERARM_HAND_ID);
		aBody.SetDeathFall(0.83f);
		aSetHead(ResourceId::IMAGE_ZOMBIEBACKUPDANCERHEAD_ID, "Zombie_disco_chops", "Zombie_backup_stash");
		break;
	case ZombieType::ZOMBIE_LADDER:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 500;
		aSetAttackRect(10, 0, 50, 115);
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_hand", "Zombie_ladder_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_LADDER_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_HAND_ID);
		aBody.mChewTimes[0] = 0.42f;
		aBody.mChewTimes[1] = 0.42f;
		aBody.SetDeathFall(0.62f);
		aSetHead(ResourceId::IMAGE_ZOMBIELADDERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_POGO:
		aBody.mRowYOffset = -16.0f;
		aBody.mSpriteGlides = true;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mCanHaveTongue = false;
		aBody.mCarriesObject = true;
		aBody.mSpawnHealth = 500;
		aSetAttackRect(10, 0, 30, 115);
		aBody.mDaisyY = 120.0f;
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_outerarm_lower", "Zombie_outerarm_upper", ResourceId::IMAGE_REANIM_ZOMBIE_POGO_OUTERARM_UPPER2_ID, ResourceId::IMAGE_REANIM_ZOMBIE_OUTERARM_HAND_ID);
		aSetIceTrap(-10.0f, 20.0f);
		aSetButter(0.0f, -5.0f);
		aBody.mDrawOffsetY = 16.0f;
		aBody.SetDeathFall(0.84f);
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_POGO_HEAD;
		break;
	case ZombieType::ZOMBIE_ZAMBONI:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 1350;
		aBody.mAnimFrames = 2;
		aBody.mAnimTicksPerFrame = 8;
		aSetSpawnX(WIDE_BOARD_WIDTH, 10);
		aBody.mRenderOffset = 8;
		aSetRect(aBody.mHitRect, 0, -13, 153, 140);
		aSetAttackRect(10, -13, 133, 140);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aBody.mCanBeButtered = false;
		aBody.mDrawOffsetX = 68.0f;
		aBody.mDrawOffsetY = -23.0f;
		aBody.mHasDeathFall = false;
		aBody.mAppearFoley = FoleyType::FOLEY_ZAMBONI;
		aBody.mHouseY = 15.0f;
		aBody.mRoofHouseX = -28.0f;
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_ZAMBONI, 83.0f, -26.0f);
		break;
	case ZombieType::ZOMBIE_CATAPULT:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 850;
		aSetSpawnX(WIDE_BOARD_WIDTH + 25, 10);
		aSetRect(aBody.mHitRect, 0, -13, 153, 140);
		aSetAttackRect(10, -13, 133, 140);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetIceTrap(32.0f, 0.0f);
		SetName(aBody.mButterHeadTrack, "Zombie_catapult_driver_head");
		aBody.mDrawOffsetX = -25.0f;
		aBody.mDrawOffsetY = -18.0f;
		aBody.mHasDeathFall = false;
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_CATAPULT, -14.0f, -30.0f);
		break;
	case ZombieType::ZOMBIE_JACK_IN_THE_BOX:
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 500;
		aBody.mAnimTicksPerFrame = 6;
		aSetAttackRect(20, 0, 50, 115);
		aBody.mHasYuckyFace = false;
		aSetArm("Zombie_jackbox_outerarm_lower", "Zombie_jackbox_outerarm_lower", ResourceId::IMAGE_REANIM_ZOMBIE_JACKBOX_OUTERARM_LOWER2_ID, ResourceId::IMAGE_ZOMBIEJACKBOXARM_ID);
		aBody.mEatAnimRate = 20.0f;
		aBody.mChewTimes[0] = 0.53f;
		aBody.mChewTimes[1] = 0.53f;
		aBody.SetDeathFall(0.64f);
		aBody.mSongFoley = FoleyType::FOLEY_JACKINTHEBOX;
		break;
	case ZombieType::ZOMBIE_BUNGEE:
		aBody.mEatsPlants = false;
		aBody.mLosesBodyParts = false;
		aBody.mShadowFollowsAltitude = true;
		aBody.mUsesPlainZombieReanim = false;
		aBody.mSpawnHealth = 450;
		aBody.mAnimFrames = 4;
		aBody.mRenderLayer = RenderLayer::RENDER_LAYER_GRAVE_STONE;
		aBody.mRenderOffset = 7;
		aSetRect(aBody.mHitRect, -20, 22, 110, 94);
		aSetAttackRect(0, 0, 0, 0);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetIceTrap(-45.0f, -23.0f, 1.2f);
		aBody.mHasDeathFall = false;
		aBody.mPuffsWhenMowed = true;
		aBody.mBurnsInPlace = true;
		break;
	case ZombieType::ZOMBIE_BOSS:
		aBody.mSpansAllRows = true;
		aBody.mGroans = false;
		aBody.mEatsPlants = false;
		aBody.mIsBoss = true;
		aBody.mLosesBodyParts = false;
		aBody.mDamagedBelow[0] = 4;
		aBody.mDamagedBelow[1] = 5;
		aBody.mBadlyDamagedBelow[0] = 1;
		aBody.mBadlyDamagedBelow[1] = 2;
		aBody.mUsesPlainZombieReanim = false;
		aSetSpawnX(0.0f, 0);
		aSetRect(aBody.mHitRect, 700, 80, 90, 430);
		aSetAttackRect(0, 0, 0, 0);
		aBody.mRenderLayer = RenderLayer::RENDER_LAYER_TOP;
		aBody.mHasYuckyFace = false;
		aBody.mShowsIceTrap = false;
		aBody.mCanBeButtered = false;
		aBody.mHasDeathFall = false;
		aBody.SetDeathAnimRate(18.0f);
		aBody.mPlaysDeathAnimWhenDisintegrated = true;
		aBody.mBurnsAsDamage = true;
		break;
	case ZombieType::ZOMBIE_NORMAL:
		aBody.mDancesInDanceMode = true;
		break;
	case ZombieType::ZOMBIE_TRAFFIC_CONE:
		aBody.mDancesInDanceMode = true;
		SetName(aBody.mSpawnShownPrefix, "anim_cone");
		aHidePrefixes("anim_hair");
		break;
	case ZombieType::ZOMBIE_PAIL:
		aBody.mDancesInDanceMode = true;
		SetName(aBody.mSpawnShownPrefix, "anim_bucket");
		aHidePrefixes("anim_hair");
		break;
	case ZombieType::ZOMBIE_FLAG:
		aBody.mCarriesObject = true;
		aSetSpawnX(WIDE_BOARD_WIDTH, 0);
		aBody.mWalkAnimVariant = 0;
		break;
	case ZombieType::ZOMBIE_PEA_HEAD:
		aHidePrefixes("anim_hair", "anim_head2");
		SetName(aBody.mSpawnFrames, "anim_walk2");
		aSetPlantHead(ReanimationType::REANIM_PEASHOOTER, "anim_head_idle", "anim_head1", 65.0f, -5.0f, 1.0f);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aBody.mWalkAnimVariant = 0;
		break;
	case ZombieType::ZOMBIE_SQUASH_HEAD:
		aHidePrefixes("anim_hair", "anim_head2");
		SetName(aBody.mSpawnFrames, "anim_walk2");
		aSetPlantHead(ReanimationType::REANIM_SQUASH, "anim_idle", "anim_head1", 55.0f, -15.0f, 0.75f);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetButter(6.0f, -9.0f);
		break;
	case ZombieType::ZOMBIE_WALLNUT_HEAD:
		aHidePrefixes("anim_hair", "anim_head", "Zombie_tie");
		aSetPlantHead(ReanimationType::REANIM_WALLNUT, "anim_idle", "Zombie_body", 50.0f, 0.0f, 0.8f);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		aSetButter(-6.0f, -1.0f);
		break;
	case ZombieType::ZOMBIE_TALLNUT_HEAD:
		aHidePrefixes("anim_hair", "anim_head", "Zombie_tie");
		aSetPlantHead(ReanimationType::REANIM_TALLNUT, "anim_idle", "Zombie_body", 37.0f, 0.0f, 0.8f);
		aBody.mCanHaveTongue = false;
		aBody.mSpawnXOffset = 30.0f;
		aBody.mHasYuckyFace = false;
		aSetButter(-24.0f, -39.0f);
		break;
	case ZombieType::ZOMBIE_JALAPENO_HEAD:
		aHidePrefixes("anim_hair", "anim_head", "Zombie_tie");
		aSetPlantHead(ReanimationType::REANIM_JALAPENO, "anim_idle", "Zombie_body", 55.0f, -5.0f, 1.0f);
		aBody.mCanHaveTongue = false;
		aBody.mSpawnHealth = 500;
		aBody.mHasYuckyFace = false;
		break;
	case ZombieType::ZOMBIE_GATLING_HEAD:
		aHidePrefixes("anim_hair", "anim_head2");
		SetName(aBody.mSpawnFrames, "anim_walk2");
		aSetPlantHead(ReanimationType::REANIM_GATLINGPEA, "anim_head_idle", "anim_head1", 65.0f, -5.0f, 1.0f);
		aBody.mCanHaveTongue = false;
		aBody.mHasYuckyFace = false;
		break;
	case ZombieType::ZOMBIE_DUCKY_TUBE:
		aBody.mHasDuckyTube = true;
		break;
	default:
		return;
	}

	theZombie->AddComponent(new ZombieBodyComponent(aBody));
}

/// @brief The armour the vanilla zombies spawn with (the Bobsled leader's helmet comes from its behaviour)
static void AddVanillaZombieArmor(Zombie *theZombie)
{
	switch (theZombie->mZombieType)
	{
	case ZombieType::ZOMBIE_TRAFFIC_CONE:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_TRAFFIC_CONE, 370);
		break;
	case ZombieType::ZOMBIE_PAIL:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_PAIL, 1100);
		break;
	case ZombieType::ZOMBIE_FOOTBALL:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_FOOTBALL, 1400);
		break;
	case ZombieType::ZOMBIE_DIGGER:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_DIGGER, 100);
		break;
	case ZombieType::ZOMBIE_WALLNUT_HEAD:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_WALLNUT, 1100);
		break;
	case ZombieType::ZOMBIE_TALLNUT_HEAD:
		theZombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_TALLNUT, 2200);
		break;
	case ZombieType::ZOMBIE_DOOR:
		theZombie->AddComponent<ShieldComponent>(ShieldType::SHIELDTYPE_DOOR, 1100);
		break;
	case ZombieType::ZOMBIE_LADDER:
		theZombie->AddComponent<ShieldComponent>(ShieldType::SHIELDTYPE_LADDER, 500);
		break;
	case ZombieType::ZOMBIE_NEWSPAPER:
		theZombie->AddComponent<ShieldComponent>(ShieldType::SHIELDTYPE_NEWSPAPER, 150);
		break;
	case ZombieType::ZOMBIE_BALLOON:
		theZombie->AddComponent<FlyingComponent>(20);
		break;
	default:
		break;
	}
}

void Archetypes::ApplyZombieDefaults(Zombie *theZombie)
{
	// Zombies outside a level (Almanac, Zombatar previews) get components too: Pogo and Bungee previews animate
	// through their behaviours

	// mMindControlled is already set for zombies that spawn hypnotized and for zombies from old saves
	bool isHypnotized = theZombie->mMindControlled;
	TeamComponent *aTeam = theZombie->AddComponent<TeamComponent>(TEAM_ZOMBIES);
	aTeam->mTeam = isHypnotized ? TEAM_PLANTS : TEAM_ZOMBIES;
	FacingComponent *aFacing = theZombie->AddComponent<FacingComponent>(-1);
	aFacing->mDirection = isHypnotized ? 1 : -1;
	theZombie->AddComponent<HypnotizableComponent>(TEAM_PLANTS, true);

	if (theZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR ||
	    theZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR || theZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
	{
		theZombie->AddComponent<UnswallowableComponent>();
	}

	AddVanillaZombieBehaviours(theZombie);
	AddVanillaZombieTraits(theZombie);
	AddVanillaZombieBody(theZombie);
	AddVanillaZombieArmor(theZombie);
}

void Archetypes::ApplyProjectileDefaults(Projectile *theProjectile)
{
	if (!theProjectile->GetEntityRef())
		return;

	theProjectile->AddComponent<TeamComponent>(DefaultProjectileTeam(theProjectile->mProjectileType));
}

int Archetypes::DefaultProjectileTeam(ProjectileType theProjectileType)
{
	return Contains(gZombieTeamProjectiles, theProjectileType) ? TEAM_ZOMBIES : TEAM_PLANTS;
}

const PlantShotDefinition &Archetypes::GetPlantShot(SeedType theSeedType)
{
	static const PlantShotDefinition kNone = { SeedType::SEED_NONE, kNoProjectile, ShotArc::Straight, 0, 0 };
	for (const PlantShotDefinition &aShot : gPlantShotDefs)
	{
		if (aShot.mSeedType == theSeedType)
			return aShot;
	}
	return kNone;
}
