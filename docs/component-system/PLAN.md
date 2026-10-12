# Component system

Goal: express game behaviour as components attached to entities instead of `mSeedType == ...` /
`mZombieType == ...` / `mMindControlled` checks, so new content is built by combining components
(e.g. a zombie with a Melon-pult head that lobs melons at your plants, or a hypnotized plant that shoots backwards).

Code lives in `src/Lawn/Component/`.

## How it works

```cpp
// Attach instances; the board owns them from then on
zombie->AddComponent(new ShooterComponent(ProjectileType::PROJECTILE_MELON, ShotArc::Lobbed, 300));
plant->AddComponent<ProducerComponent>(CoinType::COIN_SUN, 2, 2500);   // construct in place

if (ShooterComponent *aShooter = zombie->GetComponent<ShooterComponent>())
    aShooter->mInterval = 100;
zombie->RemoveComponent<HypnotizableComponent>();

// Builders: spawn + components in one expression
ZombieBuilder(board, ZombieType::ZOMBIE_NORMAL).InRow(2).Zombotany(SeedType::SEED_MELONPULT).Spawn();
PlantBuilder(board, SeedType::SEED_SUNFLOWER).At(3, 2)
    .With(ShooterComponent::CreateLike(SeedType::SEED_PEASHOOTER))
    .Hypnotized()
    .Spawn();
```

- **`Component`** (`Component.h`): abstract base class. Hooks: `OnAttach`, `OnDetach`, `OnTick`, `Serialize`,
  `DrawInspector`, and gameplay events (`OnHypnotized`, `OnHeadDropped`). Subclasses can derive from other components
  (`SunShroomProducerComponent : ProducerComponent`); `GetComponent<Base>()` finds subclasses.
- **Entity API** on `GameObject`: `AddComponent`, `GetComponent<T>`, `HasComponent<T>`, `RemoveComponent<T>`,
  `ForEachComponent`. Works for Plants, Zombies and Projectiles on a board.
- **`ComponentRegistry`** (`Board::mComponents`): owns every entity's `std::vector<std::unique_ptr<Component>>`,
  ticks them once per frame (after plants and zombies), deletes them when their entity is freed, and saves/loads them.
- **Why the vector isn't a member of `Plant`/`Zombie`:** those classes are saved by memcpy'ing their whole
  `DataArray` block. A `std::vector` member would put heap pointers into the save file, and changing the class size
  breaks every existing save. So the list is stored on the board, keyed by the entity's DataArray ID, whose
  generation key means a freed entity's components never leak onto whatever reuses its slot.
- **Saving:** one `Components.Blob` entry in the save schema. Each component is written as name + payload from
  `Serialize`; `REGISTER_COMPONENT(MyComponent)` lets the loader recreate it by name. Unknown names are skipped.
  Fields read in order and missing trailing fields keep their defaults, so components can grow by appending fields.
- **Adding a component to an archetype** changes what a saved entity should have, so bump `SAVE_VERSION`:
  loading only fills in defaults for entities with no components at all.
- **Old saves** have no blob: `ComponentRegistry::FinishLoading` gives every entity its archetype components (and
  carries the old sun countdown over).
- **Archetypes** (`Archetypes.cpp`): the default component set of each vanilla plant/zombie/projectile, applied at
  spawn in `Board::NewPlant`, `Board::AddZombieInRow` and `Board::AddProjectile`. This is the one place that still maps
  seed/zombie types to behaviour, as data tables, and it's what Phase 3 moves to data files.

### Debugging in game

Debug build → ImGui debugger → **Board → Components**:
- Spawn a Zombotany zombie from any plant (optionally hypnotized), or plant a hypnotized copy of the Quick Plant seed.
- Inspect the selected plant (Plants tab / click) or zombie (click): every component with live-editable fields,
  Remove buttons, an "Add component" picker with every registered component, and a Hypnotize button.
- New components appear automatically; override `DrawInspector` to make their fields editable
  (examples in `ComponentInspectors.cpp`).

## Components

| Component | What it does | Used by (archetypes) |
|---|---|---|
| `HealthComponent` | Body health; `entity->Health()` | every plant and zombie |
| `HelmComponent` / `ShieldComponent` / `FlyingComponent` | Armour layers with their own health, hit outermost first | Conehead, Buckethead, Football, Digger, Bobsled, Screen Door, Newspaper, Ladder, Balloon, Wall-nut/Tall-nut zombies |
| `TeamComponent` | Team id (0 plants, 1 zombies, -1 neutral, others custom) + native team | every plant, zombie, projectile |
| `FacingComponent` | Facing direction + native direction; mirrored plants flip their attack area and shots | plants, zombies |
| `HypnotizableComponent` | Can be hypnotized; which team it joins; whether it turns around | plants, zombies |
| `HypnotizeOnEatenComponent` | Hypnotizes whoever bites it | Hypno-shroom |
| `LowProfileComponent` | Straight projectiles fly over it | Puff/Sun-shroom, Potato Mine, Spikeweed, Spikerock, Lily Pad |
| `ChilledStatus` / `FrozenStatus` / `ButteredStatus` | Timed status effects (`StatusEffectComponent` subclasses); slow or stop the owner | applied by snow peas, Ice-shroom, Kernel-pult butter |
| `SpikesComponent`, `TallComponent`, `WallComponent`, `RepelOnEatenComponent`, `InedibleComponent`, `ActivatesWhenEatenComponent`, `MineComponent` | Plant traits zombies react to | Spikeweed/Spikerock, Tall-nut, Wall-nut/Tall-nut/Pumpkin, Garlic, instant plants/Tangle Kelp, Blover/Ice-shroom, Potato Mine |
| `ProjectileConverterComponent` | Converts projectiles passing through (pea -> fireball) | Torchwood |
| `OnDeathComponent` | Runs lambdas / named actions when the entity dies | custom content |
| `ProducerComponent` | Drops sun/coins on a timer (rate, count, rare coin, Big Time bonus, final-wave cutoff) | Sunflower, Twin Sunflower, Marigold |
| `SunShroomProducerComponent` | Producer subclass: small sun while small, none while growing | Sun-shroom |
| `ShooterComponent` | Generic projectile shooter (straight or lobbed) that targets the nearest hostile plant **or** zombie in its row; `CreateLike(seed)` copies a plant's shot | Zombotany builds, custom content |
| `PlantHeadComponent` | Replaces a zombie's head with any plant's animation | Zombotany builds |

### Team rules now in place

- Projectiles carry their shooter's team and hit **hostile** plants and zombies, whatever their type. This replaces
  the `PROJECTILE_ZOMBIE_PEA`/`PROJECTILE_BASKETBALL` special cases and the `DAMAGES_MINDCONTROLLED` flag for
  projectiles, plant targeting and splash damage (`Teams::CanAttack`, `Zombie::IsTargetableBy`).
- Zombies only chew hostile plants and only fight hostile zombies.
- Hypnotized plants: switch team, get the mind-control tint, mirror their attack area, and straight shooters fire the
  other way. Their shots hit plants, and other plants shoot at them, lobbed ones included.

### Vanilla behaviour changes in this phase

Small differences that fall out of using one team rule everywhere:
- Gatling-head zombie peas can hit hypnotized zombies (only Peashooter-head peas could). Peashooter-head peas now
  respect the target's state (e.g. they can't hit a hypnotized Pole Vaulter mid-vault).
- Catapult Zombie basketballs can hit hypnotized zombies.
- A zombie pea hitting a plant shows its splat 5px higher (shared impact code), and one hitting a hypnotized zombie
  now shows a splat at all.
- Sun is produced in the component tick right after zombies update, instead of inside the plant's own update.
- Chill, ice and butter wear off in the component tick, so a zombie stays affected for at most one extra game tick
  (10 ms) compared to before.

## Phases

**Phase 1 (done):** component core, save/load + old-save backfill, builders, debugger inspector, team/facing/
hypnosis, team-based projectile collision, generic shooter, plant heads, producers replacing
`Plant::UpdateProductionPlant`.

**Phase 2: health and status effects (done).**
- Done: `HealthComponent` (body + zombie helm/shield/flying layers, each with a max). The ten health fields are gone
  from `Plant`/`Zombie`; code uses `entity->Health()` (created on first use, so every plant/zombie has one, including
  Almanac/Zombatar previews, whose components live in an off-board store freed by `~GameObject`). Plant damage goes
  through `Combat::DamagePlant`, zombie damage through `Zombie::TakeDamage`, and both broadcast
  `Component::OnDamaged`. Health is editable live in the debugger's Components tab.
- Done: `SAVE_VERSION` 2 is now written and checked. Older saves are refused with a message and the level restarts,
  instead of being misread (the Plant/Zombie memory layout changed).
- Done: armour as components. `HelmComponent` (cone, bucket, football helmet, hard hat, Wall-nut/Tall-nut head,
  bobsled), `ShieldComponent` (screen door, newspaper, ladder) and `FlyingComponent` (balloon) derive from
  `ArmorComponent` (own health, a layer order, virtual `TakeDamage`/`OnBroken`/`OnDamageStageChanged`).
  `Zombie::TakeDamage` runs damage through every armour component, outermost layer first, then the body.
  The per-type looks (track, damaged images, drop particle) are data tables in `ArmorComponents.cpp`.
  `Zombie::mHelmType`/`mShieldType` are gone (`GetHelmType()`/`GetShieldType()`), and `HealthComponent` is body
  only. Any zombie can get armour: `zombie->AddComponent<HelmComponent>(HelmType::HELMTYPE_PAIL, 1100)`.
- Done: `Combat::Damage(entity, amount, flags)` damages any entity: zombies through their armour, plants through
  `Combat::DamagePlant`, anything else with a `HealthComponent` directly.
- Done: status effects as components that count down in `OnTick` and clean up in `OnExpired`:
  `ChilledStatus`, `FrozenStatus` (ice trap) and `ButteredStatus`, all deriving from `StatusEffectComponent`.
  They replace `Zombie::mChilledCounter`/`mIceTrapCounter`/`mButteredCounter`; query with `IsChilled()`/`IsFrozen()`/
  `IsButtered()` on any entity, apply with `StatusEffects::Extend<T>(entity, ticks)` / `Set<T>`. They work on plants
  too: chilled plants act at half speed, frozen/buttered ones pause, and hostile snow peas, winter melons and butter
  apply them. `SAVE_VERSION` is 3.
- Not a status: vanilla "burn" (Jalapeño, Cherry Bomb) is an instant kill/1800 damage with a charred death animation,
  not a timed effect, so it stays in `Zombie::ApplyBurn`. A damage-over-time `BurningStatus` would be new content.
- Done: splash damage hits hostile plants too (full damage on the target, a third on the top plant of each
  square nearby), and lobbed plants aim at hostile plants when there is no zombie to hit. `SAVE_VERSION` is 4.

**Phase 3: plant behaviours (in progress).**
- Done: the seed-type checks *other* code made about plants are now trait components (`PlantTraits.h`):
  `SpikesComponent` (Spikeweed/Spikerock: `Plant::IsSpiky`, vehicles, crushing), `TallComponent` (Tall-nut stops
  jumpers), `WallComponent` (Wall-nut/Tall-nut/Pumpkin: chew sound and marks, ladders, I, Zombie double bites),
  `RepelOnEatenComponent` (Garlic), `InedibleComponent` (instant plants can't be chewed/driven over while awake;
  Tangle Kelp is ignored), `ActivatesWhenEatenComponent` (Blover), `MineComponent` (Potato Mine) and
  `ProjectileConverterComponent` (Torchwood; its update moved into the component's `OnTick`, so any plant can
  light peas). `Plant::IsAbleToAct()` is the shared "awake, not squished/grabbed/frozen" check for behaviour
  components.
- Done: `OnDeathComponent` runs lambdas when its entity dies (plant destroyed or shovelled, zombie body removed),
  with named actions (`OnDeathComponent::RegisterAction`) that survive saving and can be added from the debugger.
  Built-in examples: `SpawnImp`, `DropSun`.
- Next: the plant state machines still dispatched by seed type in `Plant::UpdateAbilities` (Chomper, Squash, Potato
  Mine arming, Cherry Bomb/Jalapeño/Doom-shroom explosions, Magnet-shroom, Umbrella Leaf, Cob Cannon...) become
  behaviour components, then vanilla plant shooting moves onto `ShooterComponent` (needs per-plant muzzle offsets and
  the Threepeater/Starfruit spreads as shot patterns). Placement traits (aquatic, nocturnal, flying, upgrades) are
  needed before a plant exists (seed packets), so they belong in the plant definitions (Phase 5), not components.
- Still seed-checked in Zombie.cpp, by design for now: Bungee grab rules, sleep-icon offsets, I, Zombie and tutorial
  rules, and Tangle Kelp/Squash tracking their own target.

**Phase 4: zombie behaviours.** Same for `Zombie::Update*`: `WalkerComponent` (speed, direction from Facing),
`EaterComponent`, `VaulterComponent`, `ThrowerComponent` (Imp/basketball), `SummonerComponent` (Dancer), then the
Zombotany types become pure archetypes (plant head + shooter/explode), deleting `UpdateZombiePeaHead`/`GatlingHead`/
`JalapenoHead`/`SquashHead`.

**Phase 5: data-driven archetypes.** Archetypes load from definition files (or the Unity ScriptableObjects on
`feature/unity-bridge`): a plant/zombie definition is a list of component names and field values, which is exactly
what `ComponentFactory` + `Serialize` already handle.

## Adding a component

1. `class MyComponent : public Component { COMPONENT_TYPE(MyComponent) ... };` (or `COMPONENT_SUBTYPE` to extend one)
2. Override the hooks you need; read the owner with `GetPlant()`/`GetZombie()`, siblings with `GetSibling<T>()`.
3. `Serialize` the fields worth saving (append-only).
4. `REGISTER_COMPONENT(MyComponent)` in the .cpp; optionally override `DrawInspector`.
5. Attach it in an archetype, a builder, or at runtime from the debugger.
