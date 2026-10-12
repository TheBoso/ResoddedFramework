#include "Archetypes.h"
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
	SeedType::SEED_PUFFSHROOM, SeedType::SEED_SUNSHROOM, SeedType::SEED_POTATOMINE,
	SeedType::SEED_SPIKEWEED,  SeedType::SEED_SPIKEROCK, SeedType::SEED_LILYPAD,
};

// Projectiles fired by zombies. Only used to default old saves: every spawner assigns its own team.
static const ProjectileType gZombieTeamProjectiles[] = {
	ProjectileType::PROJECTILE_ZOMBIE_PEA,
	ProjectileType::PROJECTILE_BASKETBALL,
};

template <typename T, size_t N> static bool Contains(const T (&theList)[N], T theValue)
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
	case SeedType::SEED_SUNFLOWER:
	{
		auto *aProducer = new ProducerComponent(CoinType::COIN_SUN, 1, theRate);
		aProducer->mBigTimeBonusCoin = CoinType::COIN_SUN;
		return aProducer;
	}
	case SeedType::SEED_TWINSUNFLOWER:
		return new ProducerComponent(CoinType::COIN_SUN, 2, theRate);
	case SeedType::SEED_SUNSHROOM:
		return new SunShroomProducerComponent(theRate);
	case SeedType::SEED_MARIGOLD:
	{
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

void Archetypes::ApplyZombieDefaults(Zombie *theZombie)
{
	if (!theZombie->GetEntityRef())
		return;

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
