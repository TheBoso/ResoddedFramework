#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "Components.h"

class Board;
class Plant;
class Zombie;

// Builders spawn a vanilla entity (which gets its archetype components), then attach the components and run the
// tweaks you chained on. A zombie with a Melon-pult head that lobs melons at your plants:
//
//     Zombie *aZombie = ZombieBuilder(mBoard, ZombieType::ZOMBIE_NORMAL)
//                           .InRow(2)
//                           .Zombotany(SeedType::SEED_MELONPULT)
//                           .Spawn();
//
// A hypnotized Sunflower that drops sun and also shoots peas (at your plants, since it switched teams):
//
//     PlantBuilder(mBoard, SeedType::SEED_SUNFLOWER)
//         .At(3, 2)
//         .With(ShooterComponent::CreateLike(SeedType::SEED_PEASHOOTER))
//         .Hypnotized()
//         .Spawn();

template <typename TEntity, typename TBuilder> class EntityBuilderBase
{
  public:
	/// @brief Attach this component instance (created with `new`) to the spawned entity.
	/// The builder owns it until then. A null pointer is ignored.
	TBuilder &With(Component *theComponent)
	{
		Step aStep;
		aStep.mComponent.reset(theComponent);
		mSteps.push_back(std::move(aStep));
		return Self();
	}

	/// @brief Construct a component when the entity spawns: .With<ShooterComponent>(PROJECTILE_MELON, ShotArc::Lobbed)
	template <typename T, typename... TArgs>
		requires std::derived_from<T, Component>
	TBuilder &With(TArgs... theArgs)
	{
		return With(new T(theArgs...));
	}

	/// @brief Remove one of the archetype's default components
	template <typename T> TBuilder &Without()
	{
		return Then([](TEntity *theEntity) { theEntity->template RemoveComponent<T>(); });
	}

	/// @brief Spawn on this team, as if it had always been on it (not a hypnotized conversion)
	TBuilder &OnTeam(int theTeam)
	{
		return With<TeamComponent>(theTeam);
	}

	/// @brief Run any code on the spawned entity
	TBuilder &Then(std::function<void(TEntity *)> theFn)
	{
		Step aStep;
		aStep.mFn = std::move(theFn);
		mSteps.push_back(std::move(aStep));
		return Self();
	}

  protected:
	explicit EntityBuilderBase(Board *theBoard) : mBoard(theBoard)
	{
	}

	/// @brief Apply the chained steps in order. Components not attached (spawn failed) are deleted.
	TEntity *Finish(TEntity *theEntity)
	{
		if (theEntity)
		{
			for (Step &aStep : mSteps)
			{
				if (aStep.mComponent)
				{
					theEntity->AddComponent(aStep.mComponent.release());
				}
				else if (aStep.mFn)
				{
					aStep.mFn(theEntity);
				}
			}
		}
		mSteps.clear();
		return theEntity;
	}

	Board *mBoard;

  private:
	struct Step
	{
		std::unique_ptr<Component> mComponent;
		std::function<void(TEntity *)> mFn;
	};
	std::vector<Step> mSteps;

	TBuilder &Self()
	{
		return static_cast<TBuilder &>(*this);
	}
};

class PlantBuilder : public EntityBuilderBase<Plant, PlantBuilder>
{
  public:
	PlantBuilder(Board *theBoard, SeedType theSeedType);

	PlantBuilder &At(int theGridX, int theGridY);
	PlantBuilder &Imitater();
	/// @brief Spawn already hypnotized (on its Hypnotizable team, facing the other way)
	PlantBuilder &Hypnotized();

	/// @brief Plant it through Board::AddPlant (same bookkeeping as the player planting it)
	Plant *Spawn();

  private:
	SeedType mSeedType;
	SeedType mImitaterType = SeedType::SEED_NONE;
	int mGridX = 0;
	int mGridY = 0;
};

class ZombieBuilder : public EntityBuilderBase<Zombie, ZombieBuilder>
{
  public:
	ZombieBuilder(Board *theBoard, ZombieType theZombieType);

	ZombieBuilder &InRow(int theRow);
	ZombieBuilder &FromWave(int theWave);
	/// @brief Spawn at this X position instead of off the right edge of the lawn
	ZombieBuilder &AtX(float thePosX);
	/// @brief Spawn already hypnotized
	ZombieBuilder &Hypnotized();

	/// @brief A plant head + shoots like that plant: a Zombotany zombie made from any plant
	ZombieBuilder &Zombotany(SeedType theSeedType);

	Zombie *Spawn();

  private:
	ZombieType mZombieType;
	int mRow = -1;
	int mFromWave = 0;
};
