#include "EntityBuilder.h"
#include "Archetypes.h"
#include "Teams.h"
#include "../Board.h"

// ─── PlantBuilder ────────────────────────────────────────────────────────────

PlantBuilder::PlantBuilder(Board *theBoard, SeedType theSeedType) : EntityBuilderBase(theBoard), mSeedType(theSeedType)
{
}

PlantBuilder &PlantBuilder::At(int theGridX, int theGridY)
{
	mGridX = theGridX;
	mGridY = theGridY;
	return *this;
}

PlantBuilder &PlantBuilder::Imitater()
{
	// The Imitater morphs into a brand new plant, so chained components are not carried over to it
	mImitaterType = mSeedType;
	return *this;
}

PlantBuilder &PlantBuilder::Hypnotized()
{
	return Then([](Plant *thePlant) { Hypnosis::HypnotizePlant(thePlant); });
}

Plant *PlantBuilder::Spawn()
{
	if (mBoard == nullptr)
		return nullptr;

	SeedType aSeedType = mImitaterType != SeedType::SEED_NONE ? SeedType::SEED_IMITATER : mSeedType;
	return Finish(mBoard->AddPlant(mGridX, mGridY, aSeedType, mImitaterType));
}

// ─── ZombieBuilder ───────────────────────────────────────────────────────────

ZombieBuilder::ZombieBuilder(Board *theBoard, ZombieType theZombieType)
    : EntityBuilderBase(theBoard), mZombieType(theZombieType)
{
}

ZombieBuilder &ZombieBuilder::InRow(int theRow)
{
	mRow = theRow;
	return *this;
}

ZombieBuilder &ZombieBuilder::FromWave(int theWave)
{
	mFromWave = theWave;
	return *this;
}

ZombieBuilder &ZombieBuilder::AtX(float thePosX)
{
	return Then(
	    [thePosX](Zombie *theZombie)
	    {
		    theZombie->mPosX = thePosX;
		    theZombie->mX = (int)thePosX;
	    });
}

ZombieBuilder &ZombieBuilder::Hypnotized()
{
	return Then([](Zombie *theZombie) { theZombie->StartMindControlled(); });
}

ZombieBuilder &ZombieBuilder::Zombotany(SeedType theSeedType)
{
	With<PlantHeadComponent>(theSeedType);
	// Plants that don't shoot (Wall-nut...) just give the look
	return With(ShooterComponent::CreateLike(theSeedType));
}

Zombie *ZombieBuilder::Spawn()
{
	if (mBoard == nullptr)
		return nullptr;

	int aRow = mRow >= 0 ? mRow : mBoard->PickRowForNewZombie(mZombieType);
	return Finish(mBoard->AddZombieInRow(mZombieType, aRow, mFromWave));
}
