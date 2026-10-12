#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Component.h"

/// @brief Runs code when its entity dies (a plant is destroyed or a zombie's body disappears).
///
///     // A zombie that splits into an Imp when it dies
///     zombie->AddComponent(new OnDeathComponent([](GameObject *theDead)
///     {
///         Zombie *aZombie = static_cast<Zombie *>(theDead);
///         ZombieBuilder(aZombie->mBoard, ZombieType::ZOMBIE_IMP).InRow(aZombie->mRow).AtX(aZombie->mPosX).Spawn();
///     }));
///
/// A plain lambda can't be written to a save file, so it is lost when a saved game is loaded. To keep it, register
/// it once under a name and attach it by name:
///
///     OnDeathComponent::RegisterAction("SplitIntoImp", [](GameObject *theDead) { ... });
///     zombie->AddComponent(new OnDeathComponent("SplitIntoImp"));
///
/// The entity is still allocated (but flagged dead) while the actions run, so its position and fields are valid.
/// Actions only run during normal play, not when the level is cleaned up (level won, zombies won, board reset).
class OnDeathComponent : public Component
{
	COMPONENT_TYPE(OnDeathComponent)

	using Action = std::function<void(GameObject *theDeadEntity)>;

	OnDeathComponent() = default;
	explicit OnDeathComponent(Action theAction);
	explicit OnDeathComponent(const char *theActionName);

	/// @brief Add another action (each runs once, in the order added)
	OnDeathComponent &Then(Action theAction);
	OnDeathComponent &Then(const char *theActionName);

	/// @brief Make an action available by name (survives saving, can be added from the debugger)
	static void RegisterAction(const char *theName, Action theAction);
	static std::vector<std::string> GetRegisteredActionNames();

	void OnOwnerDied() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

  private:
	struct Entry
	{
		std::string mName; // empty: a plain lambda, not saved
		Action mAction;
	};
	std::vector<Entry> mEntries;
	bool mHasRun = false;
};
