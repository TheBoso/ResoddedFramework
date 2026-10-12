#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "Component.h"

class SaveContext;

/// @brief Owns the components of every entity on a Board (Board::mComponents).
///
/// Code normally goes through the entity instead: plant->AddComponent(...), zombie->GetComponent<T>().
///
/// The component lists live here instead of inside Plant/Zombie/Projectile because those objects are saved by
/// copying their DataArray's memory byte for byte. A std::vector member would write heap pointers into the save,
/// and growing the classes would break every existing save. Each list is keyed by the entity's DataArray ID,
/// whose generation key keeps a freed entity's components from leaking onto whatever reuses its slot.
class ComponentRegistry
{
  public:
	using ComponentList = std::vector<std::unique_ptr<Component>>;

	explicit ComponentRegistry(Board *theBoard);
	~ComponentRegistry();

	/// @brief Attach a component, taking ownership. Replaces an existing component of the same class or a subclass of it.
	Component *Add(EntityRef theEntity, std::unique_ptr<Component> theComponent);
	/// @brief The first component of this class or a subclass of it, or nullptr
	Component *Find(EntityRef theEntity, ComponentTypeId theTypeId);
	/// @brief Detach and delete the component of this class
	void Remove(EntityRef theEntity, ComponentTypeId theTypeId);
	/// @brief Detach and delete every component of the entity
	void RemoveAll(EntityRef theEntity);
	/// @brief The entity's components (empty if it has none). Removed components linger until the next cleanup.
	const ComponentList &GetAll(EntityRef theEntity);

	template <typename T> T *Get(EntityRef theEntity)
	{
		return static_cast<T *>(Find(theEntity, GetComponentTypeId<T>()));
	}

	/// @brief Tick every component. Called from Board::UpdateGameObjects.
	void Tick();

	/// @brief Delete components whose entity was freed, and components removed during a tick.
	/// Called at the end of Board::ProcessDeleteQueue.
	void CollectGarbage();

	/// @brief Call theFn(Component&) on every live component of the entity (gameplay event broadcast)
	template <typename Fn> void ForEachOf(EntityRef theEntity, Fn &&theFn)
	{
		// Index loop, and the list is looked up again each time: a handler may add components to this or another
		// entity, which can reallocate both the list and the entry table
		for (size_t i = 0;; i++)
		{
			ComponentList *aList = FindList(theEntity);
			if (aList == nullptr || i >= aList->size())
				return;
			Component *aComponent = (*aList)[i].get();
			if (!aComponent->mPendingRemoval)
			{
				theFn(*aComponent);
			}
		}
	}

	/// @brief Save or load every component (one blob in the save file)
	void Sync(SaveContext &theContext);

	/// @brief After a load: drop components of entities that no longer exist and give archetype defaults to
	/// entities that have none (saves made before the component system existed)
	void FinishLoading();

  private:
	struct Entry
	{
		EntityRef mOwner;
		ComponentList mComponents;
	};

	ComponentList *FindList(EntityRef theEntity);
	Entry &GetOrCreateEntry(EntityRef theEntity);
	void DetachAll(Entry &theEntry);
	void Clear();

	Board *mBoard;
	// Per EntityKind, indexed by DataArray slot
	std::vector<Entry> mEntries[(size_t)EntityKind::Count];
	bool mTicking = false;
};

namespace Components
{
ComponentRegistry *RegistryFor(Board *theBoard);

// Components of objects that are not on a board (Almanac/Zombatar previews), keyed by the object's address.
// They are not ticked or saved, and are deleted when the object is destroyed (GameObject::~GameObject).
Component *AddDetached(const GameObject *theObject, std::unique_ptr<Component> theComponent);
Component *FindDetached(const GameObject *theObject, ComponentTypeId theTypeId);
void RemoveDetached(const GameObject *theObject, ComponentTypeId theTypeId);
void ForEachDetached(const GameObject *theObject, const std::function<void(Component &)> &theFn);
void ReleaseDetached(const GameObject *theObject);
} // namespace Components

template <typename T> T *Component::GetSibling() const
{
	ComponentRegistry *aRegistry = Components::RegistryFor(mBoard);
	return aRegistry ? aRegistry->Get<T>(mOwner) : nullptr;
}
