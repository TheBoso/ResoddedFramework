#include <algorithm>
#include <string>
#include <unordered_map>
#include "ComponentRegistry.h"
#include "Archetypes.h"
#include "Components.h"
#include "../Board.h"
#include "../../LawnApp.h"
#include "../System/SaveGame.h"
#include "../../Sexy.TodLib/TodDebug.h"

// ─── EntityRef ───────────────────────────────────────────────────────────────

/// @brief Find an object's DataArray ID without asserting: objects outside the array (Almanac previews) give 0
template <typename T> static uint32_t IdInDataArray(const DataArray<T> &theArray, const void *theObject)
{
	if (theObject == nullptr || theArray.mBlock == nullptr)
		return 0;

	// DataArrayItem stores the object first, so an object pointer is also a pointer to its item
	auto *anItem = static_cast<const typename DataArray<T>::DataArrayItem *>(theObject);
	if (anItem < theArray.mBlock || anItem >= theArray.mBlock + theArray.mMaxUsedCount)
		return 0;

	return (anItem->mID & DATA_ARRAY_KEY_MASK) ? anItem->mID : 0;
}

EntityRef EntityRef::Of(const GameObject *theObject)
{
	if (theObject == nullptr || theObject->mBoard == nullptr)
		return {};

	// GameObject is the first (and only) base of these classes, so the pointers are interchangeable
	Board *aBoard = theObject->mBoard;
	if (uint32_t anId = IdInDataArray(aBoard->mPlants, theObject))
		return { EntityKind::Plant, anId };
	if (uint32_t anId = IdInDataArray(aBoard->mZombies, theObject))
		return { EntityKind::Zombie, anId };
	if (uint32_t anId = IdInDataArray(aBoard->mProjectiles, theObject))
		return { EntityKind::Projectile, anId };
	return {};
}

GameObject *EntityRef::Resolve(Board *theBoard) const
{
	if (theBoard == nullptr)
		return nullptr;

	switch (mKind)
	{
	case EntityKind::Plant:
		return theBoard->mPlants.DataArrayTryToGet(mId);
	case EntityKind::Zombie:
		return theBoard->mZombies.DataArrayTryToGet(mId);
	case EntityKind::Projectile:
		return theBoard->mProjectiles.DataArrayTryToGet(mId);
	default:
		// GridItem does not derive from GameObject (yet), so it can't host components
		return nullptr;
	}
}

Plant *EntityRef::AsPlant(Board *theBoard) const
{
	return mKind == EntityKind::Plant ? static_cast<Plant *>(Resolve(theBoard)) : nullptr;
}

Zombie *EntityRef::AsZombie(Board *theBoard) const
{
	return mKind == EntityKind::Zombie ? static_cast<Zombie *>(Resolve(theBoard)) : nullptr;
}

Projectile *EntityRef::AsProjectile(Board *theBoard) const
{
	return mKind == EntityKind::Projectile ? static_cast<Projectile *>(Resolve(theBoard)) : nullptr;
}

GridItem *EntityRef::AsGridItem(Board *theBoard) const
{
	return theBoard && mKind == EntityKind::GridItem ? theBoard->mGridItems.DataArrayTryToGet(mId) : nullptr;
}

/// @brief Allocated and not flagged dead (dead objects are freed at the end of the tick)
static bool IsAlive(Board *theBoard, EntityRef theEntity)
{
	switch (theEntity.mKind)
	{
	case EntityKind::Plant:
	{
		Plant *aPlant = theEntity.AsPlant(theBoard);
		return aPlant && !aPlant->mDead;
	}
	case EntityKind::Zombie:
	{
		Zombie *aZombie = theEntity.AsZombie(theBoard);
		return aZombie && !aZombie->mDead;
	}
	case EntityKind::Projectile:
	{
		Projectile *aProjectile = theEntity.AsProjectile(theBoard);
		return aProjectile && !aProjectile->mDead;
	}
	default:
		return false;
	}
}

// ─── Component ───────────────────────────────────────────────────────────────

void Component::RemoveSelf()
{
	if (ComponentRegistry *aRegistry = Components::RegistryFor(mBoard))
	{
		aRegistry->Remove(mOwner, GetTypeId());
	}
}

LawnApp *Component::GetApp() const
{
	return mBoard ? mBoard->mApp : nullptr;
}

GameObject *Component::GetOwnerObject() const
{
	return mOwner.Resolve(mBoard);
}

Plant *Component::GetPlant() const
{
	return mOwner.AsPlant(mBoard);
}

Zombie *Component::GetZombie() const
{
	return mOwner.AsZombie(mBoard);
}

Projectile *Component::GetProjectile() const
{
	return mOwner.AsProjectile(mBoard);
}

// ─── ComponentFactory ────────────────────────────────────────────────────────

static std::unordered_map<std::string, ComponentFactory::CreateFn> &FactoryMap()
{
	// Function-local so registration from other translation units' static initializers is safe
	static std::unordered_map<std::string, ComponentFactory::CreateFn> sMap;
	return sMap;
}

void ComponentFactory::Register(const char *theTypeName, CreateFn theCreateFn)
{
	FactoryMap()[theTypeName] = theCreateFn;
}

std::vector<const char *> ComponentFactory::GetRegisteredNames()
{
	std::vector<const char *> aNames;
	for (auto &anEntry : FactoryMap())
	{
		aNames.push_back(anEntry.first.c_str());
	}
	std::sort(aNames.begin(), aNames.end(), [](const char *a, const char *b) { return strcmp(a, b) < 0; });
	return aNames;
}

Component *ComponentFactory::Create(const char *theTypeName)
{
	auto anIt = FactoryMap().find(theTypeName);
	return anIt == FactoryMap().end() ? nullptr : anIt->second();
}

// ─── ComponentRegistry ───────────────────────────────────────────────────────

ComponentRegistry::ComponentRegistry(Board *theBoard) : mBoard(theBoard)
{
}

ComponentRegistry::~ComponentRegistry()
{
	// The board's objects are going away with it; no OnDetach, their reanimations die with the effect system
	Clear();
}

ComponentRegistry::ComponentList *ComponentRegistry::FindList(EntityRef theEntity)
{
	if (theEntity.IsNull() || theEntity.mKind >= EntityKind::Count)
		return nullptr;

	std::vector<Entry> &aEntries = mEntries[(size_t)theEntity.mKind];
	uint32_t aSlot = theEntity.SlotIndex();
	if (aSlot >= aEntries.size() || aEntries[aSlot].mOwner != theEntity)
		return nullptr;
	return &aEntries[aSlot].mComponents;
}

ComponentRegistry::Entry &ComponentRegistry::GetOrCreateEntry(EntityRef theEntity)
{
	std::vector<Entry> &aEntries = mEntries[(size_t)theEntity.mKind];
	uint32_t aSlot = theEntity.SlotIndex();
	if (aSlot >= aEntries.size())
	{
		aEntries.resize(aSlot + 1);
	}

	Entry &anEntry = aEntries[aSlot];
	if (anEntry.mOwner != theEntity)
	{
		// Leftovers of a freed object whose slot was reused before the last cleanup
		DetachAll(anEntry);
		anEntry.mComponents.clear();
		anEntry.mOwner = theEntity;
	}
	return anEntry;
}

void ComponentRegistry::DetachAll(Entry &theEntry)
{
	for (auto &aComponent : theEntry.mComponents)
	{
		if (!aComponent->mPendingRemoval)
		{
			aComponent->mPendingRemoval = true;
			aComponent->OnDetach();
		}
	}
}

Component *ComponentRegistry::Add(EntityRef theEntity, std::unique_ptr<Component> theComponent)
{
	if (!theComponent || theEntity.IsNull() || theEntity.mKind >= EntityKind::Count)
		return nullptr;

	Remove(theEntity, theComponent->GetTypeId());

	Component *aComponent = theComponent.get();
	aComponent->mOwner = theEntity;
	aComponent->mBoard = mBoard;
	aComponent->mPendingRemoval = false;
	GetOrCreateEntry(theEntity).mComponents.push_back(std::move(theComponent));
	aComponent->OnAttach();
	return aComponent;
}

Component *ComponentRegistry::Find(EntityRef theEntity, ComponentTypeId theTypeId)
{
	ComponentList *aList = FindList(theEntity);
	if (aList == nullptr)
		return nullptr;

	for (auto &aComponent : *aList)
	{
		if (!aComponent->mPendingRemoval && aComponent->IsA(theTypeId))
			return aComponent.get();
	}
	return nullptr;
}

void ComponentRegistry::Remove(EntityRef theEntity, ComponentTypeId theTypeId)
{
	// Deletion waits for CollectGarbage, so removing during a tick or an event broadcast is safe
	if (Component *aComponent = Find(theEntity, theTypeId))
	{
		aComponent->mPendingRemoval = true;
		aComponent->OnDetach();
	}
}

void ComponentRegistry::RemoveAll(EntityRef theEntity)
{
	if (FindList(theEntity))
	{
		DetachAll(mEntries[(size_t)theEntity.mKind][theEntity.SlotIndex()]);
	}
}

const ComponentRegistry::ComponentList &ComponentRegistry::GetAll(EntityRef theEntity)
{
	static const ComponentList kEmpty;
	ComponentList *aList = FindList(theEntity);
	return aList ? *aList : kEmpty;
}

void ComponentRegistry::Tick()
{
	if (mBoard->mApp->mGameScene != GameScenes::SCENE_PLAYING)
		return;

	for (auto &aEntries : mEntries)
	{
		// Index loops: OnTick may spawn entities, which can grow these vectors
		for (size_t aSlot = 0; aSlot < aEntries.size(); aSlot++)
		{
			EntityRef anOwner = aEntries[aSlot].mOwner;
			if (anOwner.IsNull() || !IsAlive(mBoard, anOwner))
				continue;

			for (size_t i = 0; i < aEntries[aSlot].mComponents.size(); i++)
			{
				Component *aComponent = aEntries[aSlot].mComponents[i].get();
				if (!aComponent->mPendingRemoval)
				{
					aComponent->OnTick();
				}
			}
		}
	}
}

void ComponentRegistry::CollectGarbage()
{
	for (auto &aEntries : mEntries)
	{
		for (Entry &anEntry : aEntries)
		{
			if (anEntry.mOwner.IsNull())
				continue;

			if (!anEntry.mOwner.Resolve(mBoard))
			{
				DetachAll(anEntry);
				anEntry.mComponents.clear();
				anEntry.mOwner = {};
				continue;
			}

			std::erase_if(anEntry.mComponents, [](const std::unique_ptr<Component> &theComponent)
			              { return theComponent->mPendingRemoval; });
		}
	}
}

void ComponentRegistry::Clear()
{
	for (auto &aEntries : mEntries)
	{
		aEntries.clear();
	}
}

// Save layout (one blob, so a component can grow without touching the save schema):
//   u32 version, u32 entity count
//   per entity: EntityRef, u32 component count
//     per component: u16 name length, name, u32 payload size, payload (Component::Serialize)
static constexpr uint32_t kComponentSaveVersion = 1;
static const char *kComponentBlobKey = "Components.Blob";

template <typename T> static void Put(std::vector<uint8_t> &theBuffer, const T &theValue)
{
	const uint8_t *aBytes = reinterpret_cast<const uint8_t *>(&theValue);
	theBuffer.insert(theBuffer.end(), aBytes, aBytes + sizeof(T));
}

template <typename T> static bool Take(const std::vector<uint8_t> &theBuffer, size_t &thePos, T &theValue)
{
	if (thePos + sizeof(T) > theBuffer.size())
		return false;
	memcpy(&theValue, theBuffer.data() + thePos, sizeof(T));
	thePos += sizeof(T);
	return true;
}

void ComponentRegistry::Sync(SaveContext &theContext)
{
	std::vector<uint8_t> aBlob;

	if (!theContext.mReading)
	{
		std::vector<const Entry *> aLive;
		for (auto &aEntries : mEntries)
		{
			for (const Entry &anEntry : aEntries)
			{
				if (!anEntry.mOwner.IsNull() && anEntry.mOwner.Resolve(mBoard))
				{
					aLive.push_back(&anEntry);
				}
			}
		}

		Put(aBlob, kComponentSaveVersion);
		Put(aBlob, (uint32_t)aLive.size());
		for (const Entry *anEntry : aLive)
		{
			Put(aBlob, anEntry->mOwner);
			uint32_t aCount = 0;
			for (auto &aComponent : anEntry->mComponents)
			{
				aCount += aComponent->mPendingRemoval ? 0 : 1;
			}
			Put(aBlob, aCount);

			for (auto &aComponent : anEntry->mComponents)
			{
				if (aComponent->mPendingRemoval)
					continue;

				const char *aName = aComponent->GetTypeName();
				uint16_t aNameLength = (uint16_t)strlen(aName);
				Put(aBlob, aNameLength);
				aBlob.insert(aBlob.end(), aName, aName + aNameLength);

				std::vector<uint8_t> aPayload;
				ComponentArchive anArchive(aPayload, false);
				aComponent->Serialize(anArchive);
				Put(aBlob, (uint32_t)aPayload.size());
				aBlob.insert(aBlob.end(), aPayload.begin(), aPayload.end());
			}
		}

		theContext.SyncBytes(aBlob.data(), aBlob.size(), kComponentBlobKey);
		return;
	}

	Clear();
	auto anEntryIt = theContext.mSchemeEntries.find(kComponentBlobKey);
	if (anEntryIt == theContext.mSchemeEntries.end())
		return; // a save from before the component system: FinishLoading applies the archetypes

	aBlob.resize(anEntryIt->second.mSize);
	theContext.SyncBytes(aBlob.data(), aBlob.size(), kComponentBlobKey);

	size_t aPos = 0;
	uint32_t aVersion = 0, anEntityCount = 0;
	if (!Take(aBlob, aPos, aVersion) || aVersion != kComponentSaveVersion || !Take(aBlob, aPos, anEntityCount))
	{
		TodTraceAndLog("Components: unreadable component data in save, using defaults");
		return;
	}

	for (uint32_t e = 0; e < anEntityCount; e++)
	{
		EntityRef anOwner;
		uint32_t aCount = 0;
		if (!Take(aBlob, aPos, anOwner) || !Take(aBlob, aPos, aCount) || anOwner.mKind >= EntityKind::Count)
			return;

		for (uint32_t c = 0; c < aCount; c++)
		{
			uint16_t aNameLength = 0;
			if (!Take(aBlob, aPos, aNameLength) || aPos + aNameLength > aBlob.size())
				return;
			std::string aName(reinterpret_cast<const char *>(aBlob.data() + aPos), aNameLength);
			aPos += aNameLength;

			uint32_t aPayloadSize = 0;
			if (!Take(aBlob, aPos, aPayloadSize) || aPos + aPayloadSize > aBlob.size())
				return;
			std::vector<uint8_t> aPayload(aBlob.begin() + aPos, aBlob.begin() + aPos + aPayloadSize);
			aPos += aPayloadSize;

			std::unique_ptr<Component> aComponent(ComponentFactory::Create(aName.c_str()));
			if (!aComponent)
			{
				TodTraceAndLog("Components: unknown component '%s' in save, skipped", aName.c_str());
				continue;
			}

			aComponent->mOwner = anOwner;
			aComponent->mBoard = mBoard;
			ComponentArchive anArchive(aPayload, true);
			aComponent->Serialize(anArchive);
			// No OnAttach: the component is resuming, its effects (reanimations...) were saved with the board
			GetOrCreateEntry(anOwner).mComponents.push_back(std::move(aComponent));
		}
	}
}

void ComponentRegistry::FinishLoading()
{
	// Drop entries whose owner did not survive the load
	for (auto &aEntries : mEntries)
	{
		for (Entry &anEntry : aEntries)
		{
			if (!anEntry.mOwner.IsNull() && !anEntry.mOwner.Resolve(mBoard))
			{
				anEntry.mComponents.clear();
				anEntry.mOwner = {};
			}
		}
	}

	// Every archetype has a TeamComponent, so its absence means "never initialized"
	Plant *aPlant = nullptr;
	while (mBoard->mPlants.IterateNext(aPlant))
	{
		if (!aPlant->HasComponent<TeamComponent>())
		{
			Archetypes::ApplyPlantDefaults(aPlant, true);
		}
		if (!aPlant->HasComponent<HealthComponent>())
		{
			// Health only lives in the component, so there is nothing to recover it from
			TodTraceAndLog("Components: a loaded plant (seed %d) has no health", (int)aPlant->mSeedType);
		}
	}

	Zombie *aZombie = nullptr;
	while (mBoard->mZombies.IterateNext(aZombie))
	{
		if (!aZombie->HasComponent<TeamComponent>())
		{
			Archetypes::ApplyZombieDefaults(aZombie);
		}
		if (!aZombie->HasComponent<HealthComponent>())
		{
			TodTraceAndLog("Components: a loaded zombie (type %d) has no health", (int)aZombie->mZombieType);
		}
	}

	Projectile *aProjectile = nullptr;
	while (mBoard->mProjectiles.IterateNext(aProjectile))
	{
		if (!aProjectile->HasComponent<TeamComponent>())
		{
			Archetypes::ApplyProjectileDefaults(aProjectile);
		}
	}
}

ComponentRegistry *Components::RegistryFor(Board *theBoard)
{
	return theBoard ? theBoard->mComponents : nullptr;
}

// ─── Components of objects outside a board ───────────────────────────────────

static std::unordered_map<const GameObject *, ComponentRegistry::ComponentList> &DetachedStore()
{
	// Never destroyed: GameObjects with static lifetime may be destroyed after it at exit
	static auto *sStore = new std::unordered_map<const GameObject *, ComponentRegistry::ComponentList>();
	return *sStore;
}

Component *Components::AddDetached(const GameObject *theObject, std::unique_ptr<Component> theComponent)
{
	if (!theComponent)
		return nullptr;

	RemoveDetached(theObject, theComponent->GetTypeId());
	Component *aComponent = theComponent.get();
	DetachedStore()[theObject].push_back(std::move(theComponent));
	aComponent->OnAttach();
	return aComponent;
}

Component *Components::FindDetached(const GameObject *theObject, ComponentTypeId theTypeId)
{
	auto anIt = DetachedStore().find(theObject);
	if (anIt == DetachedStore().end())
		return nullptr;

	for (auto &aComponent : anIt->second)
	{
		if (aComponent->IsA(theTypeId))
			return aComponent.get();
	}
	return nullptr;
}

void Components::RemoveDetached(const GameObject *theObject, ComponentTypeId theTypeId)
{
	auto anIt = DetachedStore().find(theObject);
	if (anIt == DetachedStore().end())
		return;

	std::erase_if(anIt->second,
	              [theTypeId](const std::unique_ptr<Component> &theComponent)
	              {
		              if (!theComponent->IsA(theTypeId))
			              return false;
		              theComponent->OnDetach();
		              return true;
	              });
}

void Components::ForEachDetached(const GameObject *theObject, const std::function<void(Component &)> &theFn)
{
	for (size_t i = 0;; i++)
	{
		auto anIt = DetachedStore().find(theObject);
		if (anIt == DetachedStore().end() || i >= anIt->second.size())
			return;
		theFn(*anIt->second[i]);
	}
}

void Components::ReleaseDetached(const GameObject *theObject)
{
	auto &aStore = DetachedStore();
	if (aStore.empty())
		return;

	auto anIt = aStore.find(theObject);
	if (anIt != aStore.end())
	{
		ComponentRegistry::ComponentList aComponents = std::move(anIt->second);
		aStore.erase(anIt);
		for (auto &aComponent : aComponents)
		{
			aComponent->OnDetach();
		}
	}
}
