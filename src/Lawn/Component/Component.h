#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include "EntityRef.h"

class Board;
class LawnApp;
class GameObject;
class Plant;
class Zombie;
class Projectile;

/// @brief Unique id per component class, used for fast GetComponent lookups (no RTTI needed)
using ComponentTypeId = const void *;
template <typename T> ComponentTypeId GetComponentTypeId()
{
	static const char sId = 0;
	return &sId;
}

/// @brief Reads or writes a component's fields for save files.
/// Fields are read back in the order they were written. Append new fields at the end of Serialize:
/// a save made before a field existed simply leaves that field at its default value.
class ComponentArchive
{
  public:
	ComponentArchive(std::vector<uint8_t> &theBuffer, bool theReading) : mBuffer(theBuffer), mReading(theReading)
	{
	}

	bool IsReading() const
	{
		return mReading;
	}

	template <typename T> void Value(T &theValue)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Only plain values can be archived");
		if (mReading)
		{
			if (mPos + sizeof(T) <= mBuffer.size())
			{
				memcpy(&theValue, mBuffer.data() + mPos, sizeof(T));
			}
			mPos += sizeof(T);
		}
		else
		{
			const uint8_t *aBytes = reinterpret_cast<const uint8_t *>(&theValue);
			mBuffer.insert(mBuffer.end(), aBytes, aBytes + sizeof(T));
		}
	}

	void String(std::string &theValue)
	{
		uint32_t aLength = (uint32_t)theValue.size();
		Value(aLength);
		if (mReading)
		{
			if (mPos + aLength <= mBuffer.size())
			{
				theValue.assign(reinterpret_cast<const char *>(mBuffer.data() + mPos), aLength);
			}
			mPos += aLength;
		}
		else
		{
			mBuffer.insert(mBuffer.end(), theValue.begin(), theValue.end());
		}
	}

  private:
	std::vector<uint8_t> &mBuffer;
	bool mReading;
	size_t mPos = 0;
};

/// @brief Base class of every component. Create one with `new`/AddComponent<T>() and attach it to an entity:
///
///     zombie->AddComponent(new ShooterComponent(ProjectileType::PROJECTILE_MELON, ShotArc::Lobbed));
///     plant->AddComponent<HypnotizableComponent>();
///
/// The entity's board owns the component from then on and deletes it when the entity is freed.
///
/// To make a new component:
///   1. derive from Component, add COMPONENT_TYPE(MyComponent) inside the class
///      (COMPONENT_SUBTYPE(MyComponent, BaseComponent) when deriving from another component)
///   2. override the hooks you need (OnTick, OnAttach, ...)
///   3. override Serialize if it has state worth saving
///   4. put REGISTER_COMPONENT(MyComponent) in its .cpp so saves can recreate it by name
class Component
{
  public:
	virtual ~Component() = default;

	/// @brief Stable name, written to save files (set by COMPONENT_TYPE)
	virtual const char *GetTypeName() const = 0;
	virtual ComponentTypeId GetTypeId() const = 0;
	/// @brief Is this component of that class or derived from it? (GetComponent<Base> also finds subclasses)
	virtual bool IsA(ComponentTypeId theTypeId) const = 0;

	/// @brief Save/load state. Default: nothing to save.
	virtual void Serialize(ComponentArchive &theArchive)
	{
		(void)theArchive;
	}

	/// @brief Called once after the component is attached (not again when a save is loaded)
	virtual void OnAttach()
	{
	}
	/// @brief Called when the component is removed or its entity is freed
	virtual void OnDetach()
	{
	}
	/// @brief Called every game tick while the board is playing and the owner is alive
	virtual void OnTick()
	{
	}

	// ── Gameplay events (broadcast to every component of the entity) ──

	/// @brief The owner was hypnotized and switched teams
	virtual void OnHypnotized()
	{
	}
	/// @brief Draw ImGui widgets that show and edit this component (debugger window, Components tab).
	/// Defined in ComponentInspectors.cpp; does nothing in builds without the debug tools.
	virtual void DrawInspector();

	/// @brief The owner took damage (after its HealthComponent was updated)
	virtual void OnDamaged(int theDamage)
	{
		(void)theDamage;
	}
	/// @brief The owner died (Plant::Die, or a zombie's body disappearing in Zombie::DieNoLoot)
	virtual void OnOwnerDied()
	{
	}
	/// @brief The owner (a zombie) lost its head
	/// @return True to skip the vanilla head-drop effect (the component handled it)
	virtual bool OnHeadDropped()
	{
		return false;
	}

	EntityRef GetOwner() const
	{
		return mOwner;
	}
	Board *GetBoard() const
	{
		return mBoard;
	}
	LawnApp *GetApp() const;

	/// @brief The owner as a GameObject / a specific kind; nullptr if it is another kind or was freed
	GameObject *GetOwnerObject() const;
	Plant *GetPlant() const;
	Zombie *GetZombie() const;
	Projectile *GetProjectile() const;

	/// @brief Another component of the same entity
	template <typename T> T *GetSibling() const;

  protected:
	/// @brief Detach this component from its entity (deleted at the end of the tick, so `this` stays valid until
	/// the caller returns)
	void RemoveSelf();

  private:
	friend class ComponentRegistry;
	EntityRef mOwner;
	Board *mBoard = nullptr;
	bool mPendingRemoval = false;
};

/// @brief Put inside a component class: gives it its save name and type id
#define COMPONENT_TYPE(ClassName)                                                                                      \
  public:                                                                                                              \
	static constexpr const char *kTypeName = #ClassName;                                                               \
	const char *GetTypeName() const override                                                                           \
	{                                                                                                                  \
		return kTypeName;                                                                                              \
	}                                                                                                                  \
	ComponentTypeId GetTypeId() const override                                                                         \
	{                                                                                                                  \
		return GetComponentTypeId<ClassName>();                                                                        \
	}                                                                                                                  \
	bool IsA(ComponentTypeId theTypeId) const override                                                                 \
	{                                                                                                                  \
		return theTypeId == GetComponentTypeId<ClassName>();                                                           \
	}

/// @brief COMPONENT_TYPE for a component that derives from another component class
#define COMPONENT_SUBTYPE(ClassName, BaseName)                                                                         \
  public:                                                                                                              \
	static constexpr const char *kTypeName = #ClassName;                                                               \
	const char *GetTypeName() const override                                                                           \
	{                                                                                                                  \
		return kTypeName;                                                                                              \
	}                                                                                                                  \
	ComponentTypeId GetTypeId() const override                                                                         \
	{                                                                                                                  \
		return GetComponentTypeId<ClassName>();                                                                        \
	}                                                                                                                  \
	bool IsA(ComponentTypeId theTypeId) const override                                                                 \
	{                                                                                                                  \
		return theTypeId == GetComponentTypeId<ClassName>() || BaseName::IsA(theTypeId);                               \
	}

/// @brief Creates components by name when a save is loaded
class ComponentFactory
{
  public:
	using CreateFn = Component *(*)();

	static void Register(const char *theTypeName, CreateFn theCreateFn);
	/// @brief nullptr for unknown names (e.g. a component from a mod that is no longer installed)
	static Component *Create(const char *theTypeName);
	/// @brief Every registered component name, sorted (debugger "Add component" list)
	static std::vector<const char *> GetRegisteredNames();
};

/// @brief Put in a component's .cpp so saves can recreate it. The class needs a default constructor.
#define REGISTER_COMPONENT(ClassName)                                                                                  \
	static const bool gRegistered_##ClassName = []                                                                     \
	{                                                                                                                  \
		ComponentFactory::Register(ClassName::kTypeName, []() -> Component * { return new ClassName(); });            \
		return true;                                                                                                   \
	}();
