#include "GameObject.h"
#include "../LawnApp.h"
#include "Board.h"
#include "Component/ComponentRegistry.h"
#include "Component/StatusEffects.h"

GameObject::GameObject()
{
	mApp = gLawnApp;
	mBoard = gLawnApp->mBoard;
	mX = 0;
	mY = 0;
	mWidth = 0;
	mHeight = 0;
	mVisible = true;
	mRow = -1;
	mRenderOrder = RenderLayer::RENDER_LAYER_TOP;
}

bool GameObject::BeginDraw(Graphics *g)
{
	if (!mVisible)
		return false;

	g->Translate(mX, mY);
	return true;
}

void GameObject::EndDraw(Graphics *g)
{
	g->Translate(-mX, -mY);
}

void GameObject::MakeParentGraphicsFrame(Graphics *g)
{
	g->Translate(-mX, -mY);
}
GameObject::~GameObject()
{
	Components::ReleaseDetached(this);
}

HealthComponent &GameObject::Health()
{
	if (HealthComponent *aHealth = GetComponent<HealthComponent>())
		return *aHealth;
	return *AddComponent<HealthComponent>();
}

bool GameObject::IsChilled() const
{
	return HasComponent<ChilledStatus>();
}

bool GameObject::IsFrozen() const
{
	return HasComponent<FrozenStatus>();
}

bool GameObject::IsButtered() const
{
	return HasComponent<ButteredStatus>();
}

EntityRef GameObject::GetEntityRef() const
{
	return EntityRef::Of(this);
}

Component *GameObject::AttachComponent(Component *theComponent)
{
	std::unique_ptr<Component> aComponent(theComponent);
	ComponentRegistry *aRegistry = Components::RegistryFor(mBoard);
	EntityRef anEntity = GetEntityRef();
	if (aRegistry == nullptr || !anEntity)
		return Components::AddDetached(this, std::move(aComponent));
	return aRegistry->Add(anEntity, std::move(aComponent));
}

Component *GameObject::FindComponent(ComponentTypeId theTypeId) const
{
	ComponentRegistry *aRegistry = Components::RegistryFor(mBoard);
	EntityRef anEntity = GetEntityRef();
	if (aRegistry == nullptr || !anEntity)
		return Components::FindDetached(this, theTypeId);
	return aRegistry->Find(anEntity, theTypeId);
}

void GameObject::DetachComponent(ComponentTypeId theTypeId)
{
	ComponentRegistry *aRegistry = Components::RegistryFor(mBoard);
	EntityRef anEntity = GetEntityRef();
	if (aRegistry == nullptr || !anEntity)
	{
		Components::RemoveDetached(this, theTypeId);
		return;
	}
	aRegistry->Remove(anEntity, theTypeId);
}

void GameObject::ForEachComponentImpl(const std::function<void(Component &)> &theFn) const
{
	ComponentRegistry *aRegistry = Components::RegistryFor(mBoard);
	EntityRef anEntity = GetEntityRef();
	if (aRegistry == nullptr || !anEntity)
	{
		Components::ForEachDetached(this, theFn);
		return;
	}
	aRegistry->ForEachOf(anEntity, theFn);
}
