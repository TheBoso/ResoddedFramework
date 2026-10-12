#pragma once

#include <cstdint>

class Board;
class GameObject;
class Plant;
class Zombie;
class Projectile;
class GridItem;

/// @brief The kind of board object an EntityRef points at.
/// Values are written to save files, so only ever append new kinds.
enum class EntityKind : uint32_t
{
	None = 0,
	Plant,
	Zombie,
	Projectile,
	GridItem,
	Count
};

/// @brief A save-safe handle to a board object: its kind plus its DataArray ID.
/// DataArray IDs carry a generation key in their upper bits, so a handle to a freed object never resolves to
/// whatever object reuses its slot later.
struct EntityRef
{
	EntityKind mKind = EntityKind::None;
	uint32_t mId = 0;

	bool IsNull() const
	{
		return mKind == EntityKind::None || mId == 0;
	}
	explicit operator bool() const
	{
		return !IsNull();
	}
	bool operator==(const EntityRef &theOther) const = default;

	/// @brief The slot index inside the owning DataArray (the lower 16 bits of the ID)
	uint32_t SlotIndex() const
	{
		return mId & 0xFFFFu;
	}

	/// @brief Build a handle from a Plant, Zombie or Projectile. Returns a null handle for objects that are not in
	/// their Board's DataArray (Almanac and Zombatar previews create them on their own).
	static EntityRef Of(const GameObject *theObject);

	/// @brief Resolve the handle, or nullptr if it points at a freed object (or a different kind)
	GameObject *Resolve(Board *theBoard) const;
	Plant *AsPlant(Board *theBoard) const;
	Zombie *AsZombie(Board *theBoard) const;
	Projectile *AsProjectile(Board *theBoard) const;
	GridItem *AsGridItem(Board *theBoard) const;
};
