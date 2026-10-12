#include <cstring>
#include "Archetypes.h"
#include "../../Resources.h"
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
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -175);
		break;
	case ZombieType::ZOMBIE_CATAPULT:
		theZombie->AddComponent<VehicleComponent>(false, 200);
		theZombie->AddComponent<WalkerComponent>(kMin, kMax, -175);
		break;
	case ZombieType::ZOMBIE_FOOTBALL:
		theZombie->AddComponent<WalkerComponent>(0.66f, 0.68f, -175);
		break;
	case ZombieType::ZOMBIE_DANCER:
	case ZombieType::ZOMBIE_BACKUP_DANCER:
		theZombie->AddComponent<DanceTroupeComponent>();
		theZombie->AddComponent<WalkerComponent>(0.45f, 0.45f, -130);
		break;
	case ZombieType::ZOMBIE_SNORKEL:
		theZombie->AddComponent<SwimmerComponent>();
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
	default:
		break;
	}
}

/// @brief The bodies of the vanilla zombies that differ from the regular zombie (shadow, flying head, charred
/// remains)
static void AddVanillaZombieBody(Zombie *theZombie)
{
	ZombieBodyComponent aBody;
	auto aSetHead = [&aBody](int theImageId, const char *theHidden1 = "", const char *theHidden2 = "") {
		aBody.mHeadImageId = theImageId;
		strncpy(aBody.mHeadHiddenPrefixes[0], theHidden1, sizeof(aBody.mHeadHiddenPrefixes[0]) - 1);
		strncpy(aBody.mHeadHiddenPrefixes[1], theHidden2, sizeof(aBody.mHeadHiddenPrefixes[1]) - 1);
	};
	auto aSetCharred = [&aBody](ReanimationType theReanim, float theX, float theY) {
		aBody.mCharredReanim = theReanim;
		aBody.mCharredX = theX;
		aBody.mCharredY = theY;
	};

	switch (theZombie->mZombieType)
	{
	case ZombieType::ZOMBIE_FOOTBALL:
		aBody.SetShadow(20.0f, 0.0f, 16.0f);
		aBody.mShadowForwardXPerScale = 21.0f;
		aBody.mShadowBackwardXPerScale = -11.0f;
		aSetHead(ResourceId::IMAGE_ZOMBIEFOOTBALLHEAD_ID);
		break;
	case ZombieType::ZOMBIE_NEWSPAPER:
		aBody.SetShadow(29.0f, 5.0f, 4.0f);
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_NEWSPAPER_HEAD;
		break;
	case ZombieType::ZOMBIE_POLEVAULTER:
		aBody.SetShadow(36.0f, -5.0f, 11.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEPOLEVAULTERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_BOBSLED:
		aBody.SetShadow(20.0f, 13.0f, 13.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEBOBSLEDHEAD_ID);
		break;
	case ZombieType::ZOMBIE_IMP:
		aBody.SetShadow(25.0f, 13.0f, 7.0f, 0.6f);
		aSetHead(ResourceId::IMAGE_ZOMBIEIMPHEAD_ID);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_IMP, 16.0f, -10.0f);
		break;
	case ZombieType::ZOMBIE_DIGGER:
		aBody.SetShadow(17.0f, 14.0f, 5.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEDIGGERHEAD_ID);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_DIGGER, 22.0f, -10.0f);
		aBody.mCharredBackwardsX = 14.0f;
		strcpy(aBody.mCharredAnim, "anim_crumble");
		break;
	case ZombieType::ZOMBIE_SNORKEL:
		aBody.SetShadow(35.0f, -2.0f, 5.0f);
		aSetHead(ResourceId::IMAGE_REANIM_ZOMBIE_SNORKLE_HEAD_ID);
		break;
	case ZombieType::ZOMBIE_DOLPHIN_RIDER:
		aBody.SetShadow(19.0f, 15.0f, 11.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEDOLPHINRIDERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_YETI:
		aBody.SetShadow(3.0f, 20.0f, 20.0f);
		aSetHead(ResourceId::IMAGE_ZOMBIEYETIHEAD_ID);
		break;
	case ZombieType::ZOMBIE_GARGANTUAR:
	case ZombieType::ZOMBIE_REDEYE_GARGANTUAR:
		aBody.SetShadow(27.0f, 27.0f, 7.0f, 1.5f);
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_GARGANTUAR, 7.0f, -20.0f);
		break;
	case ZombieType::ZOMBIE_BALLOON:
		aBody.SetShadow(23.0f, 11.0f, 13.0f);
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_BALLOON_HEAD;
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED, 22.0f, 21.0f);
		break;
	case ZombieType::ZOMBIE_DANCER:
		aSetHead(ResourceId::IMAGE_ZOMBIEDANCERHEAD_ID, "Zombie_disco_chops", "Zombie_disco_glasses");
		aBody.mHeadRenderOffset = -1;
		break;
	case ZombieType::ZOMBIE_BACKUP_DANCER:
		aSetHead(ResourceId::IMAGE_ZOMBIEBACKUPDANCERHEAD_ID, "Zombie_disco_chops", "Zombie_backup_stash");
		break;
	case ZombieType::ZOMBIE_LADDER:
		aSetHead(ResourceId::IMAGE_ZOMBIELADDERHEAD_ID);
		break;
	case ZombieType::ZOMBIE_POGO:
		aBody.mHeadParticle = ParticleEffect::PARTICLE_ZOMBIE_POGO_HEAD;
		break;
	case ZombieType::ZOMBIE_ZAMBONI:
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_ZAMBONI, 83.0f, -26.0f);
		break;
	case ZombieType::ZOMBIE_CATAPULT:
		aSetCharred(ReanimationType::REANIM_ZOMBIE_CHARRED_CATAPULT, -14.0f, -30.0f);
		break;
	default:
		return;
	}

	theZombie->AddComponent(new ZombieBodyComponent(aBody));
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
