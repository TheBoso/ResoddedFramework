#pragma once

#include "../ConstEnums.h"
#include "../SexyAppFramework/Graphics.h"
#include <concepts>
#include <functional>
#include <utility>
#include "Component/Component.h"
#include "Component/HealthComponent.h"

using namespace Sexy;

class LawnApp;
class Board;

class GameObject
{
  public:
	LawnApp *mApp;
	Board *mBoard;
	int mX;
	int mY;
	int mWidth;
	int mHeight;
	bool mVisible;
	int mRow;
	int mRenderOrder;

  public:
	GameObject();
	/// @brief Frees components of objects that are not on a board (Almanac/Zombatar previews)
	~GameObject();
	/// @brief Begin the drawing a GameObject by translating the Graphics object by it's X and Y values
	/// @return True if it began to draw it, False if it was invisible
	bool BeginDraw(Graphics *g);
	/// @brief End the drawing a GameObject by translating the Graphics object by the negative of it's X and Y values
	void EndDraw(Graphics *g);
	/// @brief Frame the Graphics object relative to the GameObject (???)
	void MakeParentGraphicsFrame(Graphics *g);

	// ── Components ──
	// For Plants, Zombies and Projectiles on a board, the board owns the components (see ComponentRegistry for why
	// they are not stored in the object itself). Objects outside a board (Almanac/Zombatar previews) keep theirs
	// in a side store until they are destroyed; those are never ticked or saved.

	/// @brief This object's handle, or a null handle if it is not on a board
	EntityRef GetEntityRef() const;

	/// @brief Attach a component created with `new`, taking ownership. Replaces a component of the same class.
	/// @return The component, or nullptr (and it is deleted) if this object is not on a board
	template <typename T>
		requires std::derived_from<T, Component>
	T *AddComponent(T *theComponent)
	{
		return static_cast<T *>(AttachComponent(theComponent));
	}

	/// @brief Construct and attach a component: AddComponent<ShooterComponent>(PROJECTILE_MELON, ShotArc::Lobbed)
	template <typename T, typename... TArgs>
		requires std::derived_from<T, Component>
	T *AddComponent(TArgs &&...theArgs)
	{
		return static_cast<T *>(AttachComponent(new T(std::forward<TArgs>(theArgs)...)));
	}

	template <typename T> T *GetComponent() const
	{
		return static_cast<T *>(FindComponent(GetComponentTypeId<T>()));
	}

	template <typename T> bool HasComponent() const
	{
		return FindComponent(GetComponentTypeId<T>()) != nullptr;
	}

	template <typename T> void RemoveComponent()
	{
		DetachComponent(GetComponentTypeId<T>());
	}

	/// @brief This object's HealthComponent, created on first use. Plants and zombies always have one.
	HealthComponent &Health();

	/// @brief Status effects (see Component/StatusEffects.h)
	bool IsChilled() const;
	bool IsFrozen() const;
	bool IsButtered() const;

	/// @brief Call theFn(Component&) for every component of this object
	template <typename Fn> void ForEachComponent(Fn &&theFn) const
	{
		ForEachComponentImpl([&](Component &theComponent) { theFn(theComponent); });
	}

  private:
	Component *AttachComponent(Component *theComponent);
	Component *FindComponent(ComponentTypeId theTypeId) const;
	void DetachComponent(ComponentTypeId theTypeId);
	void ForEachComponentImpl(const std::function<void(Component &)> &theFn) const;
};
