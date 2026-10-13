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
template <typename T>
ComponentTypeId GetComponentTypeId()
{
	static const char sId = 0;
	return &sId;
}

/// @brief Identifies an enum type for EnumNames (see EnumNames.h); one address per type
template <typename T>
const void *EnumKey()
{
	static const char sKey = 0;
	return &sKey;
}

/// @brief The text side of a ComponentArchive: named fields, as in the definition files (see
/// ComponentDefinitions.h). Reading: each call returns false when the field is absent, leaving the value as it is.
class ComponentFields
{
  public:
	virtual ~ComponentFields() = default;

	virtual bool Integer(const char *theName, int64_t &theValue) = 0;
	/// @param theIsFloat The value is a float: written with the fewest digits that read back the same float
	virtual bool Number(const char *theName, double &theValue, bool theIsFloat) = 0;
	virtual bool Boolean(const char *theName, bool &theValue) = 0;
	virtual bool Text(const char *theName, std::string &theValue) = 0;
	/// @brief Written by name when the enum's names are registered (EnumNames), else as a number
	virtual bool Enum(const char *theName, int64_t &theValue, const void *theEnumKey) = 0;
	/// @brief Raw bytes, for values that are none of the above
	virtual bool Bytes(const char *theName, void *theData, size_t theSize) = 0;

	/// @brief A list: the elements in between are read/written with a null name, in order.
	/// Reading: theCount is set to the number of elements in the file.
	virtual bool BeginList(const char *theName, size_t &theCount) = 0;
	virtual void EndList() = 0;
	/// @brief A struct with its own Serialize, written as a nested object
	virtual bool BeginObject(const char *theName) = 0;
	virtual void EndObject() = 0;
};

/// @brief Reads or writes a component's fields, either for save files (binary) or for definition files (named
/// fields, see ComponentFields).
///
/// Binary: fields are read back in the order they were written. Append new fields at the end of Serialize: a save
/// made before a field existed simply leaves that field at its default value.
/// Named: the field names are the keys in the definition files, so keep them stable once published.
class ComponentArchive
{
  public:
	ComponentArchive(std::vector<uint8_t> &theBuffer, bool theReading)
	    : mBuffer(&theBuffer), mFields(nullptr), mReading(theReading)
	{
	}
	ComponentArchive(ComponentFields &theFields, bool theReading)
	    : mBuffer(nullptr), mFields(&theFields), mReading(theReading)
	{
	}

	bool IsReading() const
	{
		return mReading;
	}
	/// @brief Named fields (a definition file) rather than a save
	bool IsNamed() const
	{
		return mFields != nullptr;
	}

	template <typename T>
	void Value(const char *theName, T &theValue)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Only plain values can be archived");
		if (mFields)
		{
			NamedValue(theName, theValue);
			return;
		}

		if (mReading)
		{
			if (mPos + sizeof(T) <= mBuffer->size())
			{
				memcpy(&theValue, mBuffer->data() + mPos, sizeof(T));
			}
			mPos += sizeof(T);
		}
		else
		{
			const uint8_t *aBytes = reinterpret_cast<const uint8_t *>(&theValue);
			mBuffer->insert(mBuffer->end(), aBytes, aBytes + sizeof(T));
		}
	}

	void String(const char *theName, std::string &theValue)
	{
		if (mFields)
		{
			mFields->Text(theName, theValue);
			return;
		}

		uint32_t aLength = (uint32_t)theValue.size();
		Value(theName, aLength);
		if (mReading)
		{
			if (mPos + aLength <= mBuffer->size())
			{
				theValue.assign(reinterpret_cast<const char *>(mBuffer->data() + mPos), aLength);
			}
			mPos += aLength;
		}
		else
		{
			mBuffer->insert(mBuffer->end(), theValue.begin(), theValue.end());
		}
	}

	/// @brief A list of strings (binary: a u32 count, then each string)
	void StringList(const char *theName, std::vector<std::string> &theValues)
	{
		if (mFields)
		{
			size_t aCount = theValues.size();
			if (mFields->BeginList(theName, aCount))
			{
				if (mReading)
				{
					theValues.assign(aCount, std::string());
				}
				for (std::string &aValue : theValues)
				{
					mFields->Text(nullptr, aValue);
				}
				mFields->EndList();
			}
			return;
		}

		uint32_t aCount = (uint32_t)theValues.size();
		Value(theName, aCount);
		if (mReading)
		{
			theValues.assign(aCount, std::string());
		}
		for (std::string &aValue : theValues)
		{
			String(nullptr, aValue);
		}
	}

  private:
	template <typename T>
	void NamedValue(const char *theName, T &theValue)
	{
		if constexpr (std::is_same_v<T, bool>)
		{
			mFields->Boolean(theName, theValue);
		}
		else if constexpr (std::is_enum_v<T>)
		{
			int64_t aValue = (int64_t)theValue;
			if (mFields->Enum(theName, aValue, EnumKey<T>()))
			{
				theValue = (T)aValue;
			}
		}
		else if constexpr (std::is_integral_v<T>)
		{
			int64_t aValue = (int64_t)theValue;
			if (mFields->Integer(theName, aValue))
			{
				theValue = (T)aValue;
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			double aValue = (double)theValue;
			if (mFields->Number(theName, aValue, std::is_same_v<T, float>))
			{
				theValue = (T)aValue;
			}
		}
		else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>)
		{
			std::string aText(theValue, strnlen(theValue, sizeof(T)));
			if (mFields->Text(theName, aText))
			{
				strncpy(theValue, aText.c_str(), sizeof(T) - 1);
				theValue[sizeof(T) - 1] = '\0';
			}
		}
		else if constexpr (std::is_array_v<T>)
		{
			size_t aCount = std::extent_v<T>;
			if (mFields->BeginList(theName, aCount))
			{
				for (size_t i = 0; i < std::extent_v<T> && i < aCount; i++)
				{
					NamedValue(nullptr, theValue[i]);
				}
				mFields->EndList();
			}
		}
		else if constexpr (requires(T &theStruct, ComponentArchive &theArchive) { theStruct.Serialize(theArchive); })
		{
			if (mFields->BeginObject(theName))
			{
				theValue.Serialize(*this);
				mFields->EndObject();
			}
		}
		else
		{
			mFields->Bytes(theName, &theValue, sizeof(T));
		}
	}

	std::vector<uint8_t> *mBuffer;
	ComponentFields *mFields;
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
	template <typename T>
	T *GetSibling() const;

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
#define COMPONENT_TYPE(ClassName)                            \
  public:                                                    \
	static constexpr const char *kTypeName = #ClassName;     \
	const char *GetTypeName() const override                 \
	{                                                        \
		return kTypeName;                                    \
	}                                                        \
	ComponentTypeId GetTypeId() const override               \
	{                                                        \
		return GetComponentTypeId<ClassName>();              \
	}                                                        \
	bool IsA(ComponentTypeId theTypeId) const override       \
	{                                                        \
		return theTypeId == GetComponentTypeId<ClassName>(); \
	}

/// @brief COMPONENT_TYPE for a component that derives from another component class
#define COMPONENT_SUBTYPE(ClassName, BaseName)                                           \
  public:                                                                                \
	static constexpr const char *kTypeName = #ClassName;                                 \
	const char *GetTypeName() const override                                             \
	{                                                                                    \
		return kTypeName;                                                                \
	}                                                                                    \
	ComponentTypeId GetTypeId() const override                                           \
	{                                                                                    \
		return GetComponentTypeId<ClassName>();                                          \
	}                                                                                    \
	bool IsA(ComponentTypeId theTypeId) const override                                   \
	{                                                                                    \
		return theTypeId == GetComponentTypeId<ClassName>() || BaseName::IsA(theTypeId); \
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
#define REGISTER_COMPONENT(ClassName)                                                                      \
	static const bool gRegistered_##ClassName = [] {                                                       \
		ComponentFactory::Register(ClassName::kTypeName, []() -> Component * { return new ClassName(); }); \
		return true;                                                                                       \
	}();
