#include <map>
#include "OnDeathComponent.h"
#include "EntityBuilder.h"
#include "Teams.h"
#include "../Board.h"
#include "../../LawnApp.h"

REGISTER_COMPONENT(OnDeathComponent)

static std::map<std::string, OnDeathComponent::Action> &ActionMap()
{
	static std::map<std::string, OnDeathComponent::Action> sActions;
	return sActions;
}

void OnDeathComponent::RegisterAction(const char *theName, Action theAction)
{
	ActionMap()[theName] = std::move(theAction);
}

std::vector<std::string> OnDeathComponent::GetRegisteredActionNames()
{
	std::vector<std::string> aNames;
	for (auto &anEntry : ActionMap())
	{
		aNames.push_back(anEntry.first);
	}
	return aNames;
}

OnDeathComponent::OnDeathComponent(Action theAction)
{
	Then(std::move(theAction));
}

OnDeathComponent::OnDeathComponent(const char *theActionName)
{
	Then(theActionName);
}

OnDeathComponent &OnDeathComponent::Then(Action theAction)
{
	mEntries.push_back({ std::string(), std::move(theAction) });
	return *this;
}

OnDeathComponent &OnDeathComponent::Then(const char *theActionName)
{
	// Looked up when the entity dies, so actions registered later (or after loading) still work
	mEntries.push_back({ theActionName, nullptr });
	return *this;
}

void OnDeathComponent::OnOwnerDied()
{
	LawnApp *anApp = GetApp();
	if (mHasRun || anApp == nullptr || anApp->mGameScene != GameScenes::SCENE_PLAYING ||
	    GetBoard()->HasLevelAwardDropped())
		return;
	mHasRun = true;

	GameObject *anOwner = GetOwnerObject();
	for (Entry &anEntry : mEntries)
	{
		Action anAction = anEntry.mAction;
		if (!anAction && !anEntry.mName.empty())
		{
			auto anIt = ActionMap().find(anEntry.mName);
			if (anIt != ActionMap().end())
			{
				anAction = anIt->second;
			}
		}
		if (anAction)
		{
			anAction(anOwner);
		}
	}
}

void OnDeathComponent::Serialize(ComponentArchive &theArchive)
{
	// Only named actions survive a save; plain lambdas are skipped
	uint32_t aCount = 0;
	if (!theArchive.IsReading())
	{
		for (Entry &anEntry : mEntries)
		{
			aCount += anEntry.mName.empty() ? 0 : 1;
		}
	}
	theArchive.Value(aCount);

	if (theArchive.IsReading())
	{
		mEntries.clear();
		for (uint32_t i = 0; i < aCount; i++)
		{
			std::string aName;
			theArchive.String(aName);
			mEntries.push_back({ aName, nullptr });
		}
	}
	else
	{
		for (Entry &anEntry : mEntries)
		{
			if (!anEntry.mName.empty())
			{
				theArchive.String(anEntry.mName);
			}
		}
	}
	theArchive.Value(mHasRun);
}

// ─── Built-in actions (examples, and pickable in the debugger) ───────────────

static const bool gRegisteredDeathActions = []
{
	// A zombie splits into an Imp, on the same team, where it died
	OnDeathComponent::RegisterAction("SpawnImp",
	                                 [](GameObject *theDead)
	                                 {
		                                 Zombie *aZombie = theDead->GetEntityRef().AsZombie(theDead->mBoard);
		                                 if (aZombie == nullptr)
			                                 return;

		                                 ZombieBuilder aBuilder(aZombie->mBoard, ZombieType::ZOMBIE_IMP);
		                                 aBuilder.InRow(aZombie->mRow).AtX(aZombie->mPosX);
		                                 if (Teams::Of(aZombie) != TEAM_ZOMBIES)
		                                 {
			                                 aBuilder.Hypnotized();
		                                 }
		                                 aBuilder.Spawn();
	                                 });

	// Anything drops a sun where it died
	OnDeathComponent::RegisterAction("DropSun",
	                                 [](GameObject *theDead)
	                                 {
		                                 theDead->mBoard->AddCoin(theDead->mX, theDead->mY, CoinType::COIN_SUN,
		                                                          CoinMotion::COIN_MOTION_FROM_PLANT);
	                                 });
	return true;
}();
