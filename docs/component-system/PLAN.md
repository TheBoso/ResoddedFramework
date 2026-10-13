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
| `PlantBehaviourComponent` subclasses | Per-tick plant behaviour and activation (`UpdateBehaviour`, `DoSpecial`) | every plant with a special ability |
| `ExplodeComponent` / `BurnRowComponent` / `FreezeAllComponent` / `FuseComponent` | Instant-use effects as data | Cherry Bomb, Doom-shroom, Potato Mine, Jalapeño, Ice-shroom |
| `UnswallowableComponent` | Chompers only bite it | Gargantuar, Giga-gargantuar, Dr. Zomboss |
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
- A non-hypnotized Jack-in-the-Box used to hit hypnotized zombies in any state; it now uses the normal reach rules,
  so e.g. a hypnotized Bungee still in the air is not hit.
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
- Done: plant behaviours (`PlantBehaviours.h`). The seed-type switches in `Plant::UpdateAbilities` and
  `Plant::DoSpecial` are gone: both run the plant's `PlantBehaviourComponent`s instead (`UpdateBehaviour` /
  `DoSpecial`), at the same spot as before, so timing and the asleep/squished/Bungee rules are unchanged. A plant can
  have several behaviours.
  - Data-driven, code moved out of Plant: `ExplodeComponent` (Cherry Bomb, Doom-shroom, Potato Mine presets: radius,
    rows, burn, sound, particle, crater, achievement), `BurnRowComponent` (Jalapeño, with an optional row spread),
    `FreezeAllComponent` (Ice-shroom), `FuseComponent` (lights when awake) and `DoomFuseComponent` (its swelling
    animation).
  - The state machines of Squash, Chomper, Blover, Umbrella Leaf, Coffee Bean, Flower Pot, Lily Pad, Imitater,
    Cob Cannon, Cactus, Magnet-shroom, Gold Magnet, Sun-shroom growth, Grave Buster, Potato Mine arming, Spikeweed,
    Tangle Kelp and Scaredy-shroom moved out of Plant.cpp into their components (`VanillaPlantBehaviours.cpp`).
    They can be removed, combined or given to other plants, but they play the original plant's animation tracks.
  - `SAVE_VERSION` 6.
- Done: the behaviours' gameplay numbers are fields (editable in the debugger, saved per plant): Squash look/windup
  time and damage, Chomper bite time, chew time and bite damage, Spikeweed damage and hit timing (Spikerock is the
  same behaviour with two hits), Magnet-shroom range, rows and recharge, Gold Magnet recharge and pull chance, Grave
  Buster eat time, Tangle Kelp grab time, Scaredy-shroom fear radius and rows, Umbrella Leaf trigger delay, Coffee
  Bean wake delay. Pure animation/state bookkeeping (Flower Pot, Lily Pad, Imitater, Cob Cannon, Cactus, Sun-shroom
  growth, Potato Mine arming, Blover) keeps its constants.
- Done: the last type checks in the behaviours are traits: Spikerock's double hit is data, and Chomper asks for
  `UnswallowableComponent` (Gargantuars, Dr. Zomboss) instead of zombie types. Squash, Magnet-shroom, Scaredy-shroom,
  Star Fruit, Spikeweed and Jalapeño target by team. `SAVE_VERSION` 7.
- Done: blasts are team-aware. `Board::KillAllZombiesInRadius`, `GetZombiesInRadius` and `KillAllPlantsInRadius`
  take the attacker's team; Cherry Bomb, Doom-shroom, Potato Mine, Explode-o-nut, Cob Cannon and Jalapeno hit
  hostile zombies *and* hostile plants (so a hypnotized Cherry Bomb blows up your plants). The Jack-in-the-Box has one
  code path for both teams instead of two `mMindControlled` branches.
- Done: vanilla plant shooting is `PlantShooterComponent` (a `ShooterComponent` subclass, so
  `GetComponent<ShooterComponent>()` finds plant and zombie shooters alike). `Plant::UpdateShooter`,
  `FindTargetAndFire`, `UpdateShooting`, `Fire`, `LaunchThreepeater`, `LaunchStarFruit` and `StarFruitFire` moved
  into it, and their seed branches are data: per weapon the projectile, motion, muzzle, sounds, muzzle particle,
  shooting animation (track, speed, delay) and area damage; per plant the extra launches, fire ticks, clouds and the
  alternate-weapon chance. Threepeater, Split Pea, Starfruit, Cactus, catapults and Cob Cannon keep small dedicated
  code paths picked by a pattern field. The plant's update still drives it, so timing and animations are unchanged.
  `SAVE_VERSION` 8.
- Done: hypnotized plants are drawn mirrored (they already aimed and fired the other way). The remaining `EffectedByDamage` users are zombie/environment code (lawn mowers, Jalapeno/Squash
  zombie heads) and move with Phase 4.
- Still seed-checked in Zombie.cpp, by design for now: Bungee grab rules, sleep-icon offsets, I, Zombie and tutorial
  rules, and Tangle Kelp/Squash tracking their own target.

**Phase 4: zombie behaviours (in progress).**
- Done: the zombie-type dispatch is behaviour components (`ZombieBehaviours.h`). `ZombieBehaviourComponent` has three
  hooks that run exactly where the old dispatch was: `UpdateActions` (in `Zombie::UpdateActions`, only while the
  zombie can act), `UpdatePlaying` (every tick on the board, even frozen: Dr. Zomboss) and `UpdateAlways` (every
  tick, also for Almanac previews: Bungee, Pogo). The 23 behaviour functions (Pole Vaulter, Catapult, Dolphin Rider,
  Snorkel, Balloon, Newspaper, Digger, Jack-in-the-Box, Gargantuars, Bobsled, Zamboni, Ladder, Yeti, Dancer, Backup
  Dancer, Imp, the four Zombotany heads, Dr. Zomboss, Bungee, Pogo) moved from Zombie.cpp into their components
  (`VanillaZombieBehaviours.cpp`). The zombie archetype is now applied at the end of `Zombie::ZombieInitialize`, so
  zombies outside a level get their components too. `SAVE_VERSION` 9.
- Done: zombie traits (`ZombieTraits.h`) replace the recurring zombie-type groups in the shared zombie code:
  `GiantComponent` (Gargantuars: crush spikes, keep their limbs, heavy death/burn/shadow visuals),
  `VehicleComponent` (Zamboni, Catapult: drive over plants, spikes destroy them, fire-proof, break down below
  `mBreakdownHealth`; `mCrushesSpikes` for the Zamboni), `DanceTroupeComponent` (Dancer + Backup Dancers move, slow
  down and go quiet together), `SwimmerComponent` (Snorkel, Dolphin Rider enter the pool their own way) and
  `WalkerComponent` (speed range and how far past the lawn the zombie must walk; Football, Yeti, Dancers, Pogo, Flag,
  Jack-in-the-Box, Squash head, the big zombies). Query them with `Zombie::IsGiant()`, `IsVehicle()`,
  `IsInDanceTroupe()`, `IsSwimmer()`, `CrushesSpikes()`. The archetype is applied early in `ZombieInitialize` so the
  first speed pick already sees the walker. Zombie.cpp went from 232 to 184 zombie-type checks. `SAVE_VERSION` 10.
- Done: behaviour hooks, first batch. `ZombieBehaviourComponent::OnLoseHead` runs in `Zombie::DropHead` where the
  per-type code was (Pole Vaulter drops its pole, Balloon its hat, Pogo breaks its stick, and the new
  `FlagBearerBehaviour` drops the flag). The look of each zombie's body is data on `ZombieBodyComponent`: shadow
  placement (`DrawShadow`), the flying head's particle, image, hidden tracks and draw order (`DropHead`), and the
  charred remains' reanim, offset and animation (`ApplyBurn`). Zombies without one use the regular zombie's values,
  so a custom zombie gets a different shadow or head by attaching a body instead of editing Zombie.cpp. The
  Gargantuar shadow/charred look now comes from the body, not `GiantComponent` (a zombie made giant at runtime keeps
  its own look). Zombie.cpp went from 166 to 132 `mZombieType ==` checks. `SAVE_VERSION` 11.
- Done: behaviour hooks, second batch. `ZombieBehaviourComponent` gained `OnBodyDamaged` (Zamboni, Catapult,
  Gargantuar and Dr. Zomboss show their own damaged look instead of losing an arm/head), `OnHitByMower` (Dr. Zomboss
  ignores mowers, vehicles blow up), `OnMowed` (drop the pole/flag/head), `OnBurned`, `HasDeathAnim` (Dolphin Rider
  out of the pool), `OnDying`, `UpdateDying` (Dr. Zomboss' finale, vehicles blowing up) and `OnRemoved` (Bobsled
  team, Bungee and Dr. Zomboss take others with them). The bool hooks are asked through `AnyBehaviour` in
  Zombie.cpp. `ZombieBodyComponent` gained the death animation speed and fall time, puffing when mowed, burning in
  place / fire only hurting, the appear and song sounds, and where the zombie stands in the house; Jack-in-the-Box,
  Bungee and Dr. Zomboss got bodies. Gargantuar body cracks moved from `IsGiant()` to `GargantuarBehaviour`.
  Zombie.cpp went from 132 to 91 checks. `SAVE_VERSION` 12.
- Done: walking and animation. `WalkerComponent::mMoveStyle` says how a zombie moves: `Walk` (feet follow the
  walk animation), `Glide` (Catapult: own speed, animation at its own pace), `Drive` (Zamboni: own speed even when
  chilled), `Stand` (Bungee, Dr. Zomboss never walk). `ZombieBodyComponent` gained the eating animation speed, when
  the hands chomp, foot dust (Football), the walk animation variant (Flag, Peashooter zombie), dancing with the
  dance cheat (the plain zombies, which now get a body) and the body's draw offset. New behaviour hooks `PlaceBody`
  (Dolphin Rider, Snorkel, Bungee place and clip their own body) and `DrawBody` (Bobsled, Bungee, Dancer draw their
  own way). Zombie.cpp went from 91 to 66 `mZombieType ==` checks (and 120 to 110 `case` labels). `SAVE_VERSION` 13.
- Done: lost arms, ice and butter. `ZombieBodyComponent` gained the lost arm (tracks hidden, track the arm falls
  from, upper-arm image, falling arm image), the ice block (shown or not, offset, scale) and the butter splat (can be
  buttered, head track, offset, scale); the Wall-nut, Tall-nut and Squash zombies got bodies. New hooks `OnLoseArm`
  (Pogo stick and the flag show damage), `OnButtered` / `OnButterRemoved` (Pogo lands, Balloon propeller stops, the
  Peashooter and Gatling heads resume shooting). `BodyOf(zombie)` in Zombie.cpp returns the body or the regular
  one. A Dancer now hides its pointing hand's track with its arm even when the arm leaves no particle (it never
  swims, and a disintegrated zombie leaves no body, so this doesn't show). Zombie.cpp went from 66 to 58
  `mZombieType ==` checks and 110 to 62 `case` labels. `SAVE_VERSION` 14.
- Done: targeting and the yucky face. New hooks `CanBeTargeted` (a Bungee only at the bottom of its drop,
  Dr. Zomboss only with his head down, the Bobsled team only its leader) and `CanBeTargetedOffScreen` (the Bobsled
  team). `SwimmerComponent::mSwimsUnderwater` (Snorkel) drives `Zombie::IsSubmerged()`, used for targeting and
  where Tangle Kelp grabs. `ZombieBodyComponent::mHasYuckyFace` says the zombie has the regular head that can pull
  the garlic face (true by default, like the regular zombie; the zombies with their own head turn it off, and the
  Jalapeno and Gatling zombies got bodies for it). Zombie.cpp is down to 50 `mZombieType ==` checks.
  `SAVE_VERSION` 15.
- Done: spawning and Daisy mode. The 33-case switch in `Zombie::ZombieInitialize` is gone. The armour the
  zombies spawn with comes from the archetype (`AddVanillaZombieArmor`; only the Bobsled leader's helmet stays in
  its behaviour, since it depends on the team). What a zombie spawns with is body data: health, the plain zombie
  animation and its shown/hidden layers, hit and attack rects, size, sprite frames, scale, carrying an object, the
  tongue variant, where it enters, its render layer and the Zombotany head (plant animation, track, offset,
  scale). What it does as it appears (starting phase and animation, Jack-in-the-Box fuse, Gargantuar pole, Bobsled
  seats, the Flag Zombie's flag and Zombatar...) is the new `OnSpawn(zombie, ZombieSpawn&)` hook; `ZombieSpawn`
  carries the parent zombie, the render layer/offset and a cancel flag (a Bungee with nothing to grab). The
  shield is attached after the behaviours spawn. Daisy mode reads `mLeavesDaisies` and the daisy offset from the
  body. Random numbers are drawn in the same order as before. Zombie.cpp is down to 45 `mZombieType ==` checks and
  29 `case` labels. `SAVE_VERSION` 16.
- Still type-checked: the legacy sprite path of `UpdateZombieWalking` (zombies without a body animation), the
  sprite drawing in `DrawZombie`, and `SetupReanimLayers` (shared with the Almanac).
- Next: the remaining checks are mostly single types tied to that zombie's own animation and state (Zamboni ice
  trail, Catapult basketball count, Snorkel/Dolphin pool animations, Dr. Zomboss parts). They move into hooks on the
  zombie's behaviour component (e.g. `OnDeathAnim`, `GetDrawPosition`) so the shared code asks the behaviour instead
  of the type. The vanilla
  Zombotany heads keep their own behaviours for now: turning them into plant head + generic shooter would change how
  they play (the generic shooter waits for a target; the vanilla Peashooter Zombie fires on a timer).

**Phase 5: data-driven archetypes.** Archetypes load from definition files (or the Unity ScriptableObjects on
`feature/unity-bridge`): a plant/zombie definition is a list of component names and field values, which is exactly
what `ComponentFactory` + `Serialize` already handle.

## Adding a component

1. `class MyComponent : public Component { COMPONENT_TYPE(MyComponent) ... };` (or `COMPONENT_SUBTYPE` to extend one)
2. Override the hooks you need; read the owner with `GetPlant()`/`GetZombie()`, siblings with `GetSibling<T>()`.
3. `Serialize` the fields worth saving (append-only).
4. `REGISTER_COMPONENT(MyComponent)` in the .cpp; optionally override `DrawInspector`.
5. Attach it in an archetype, a builder, or at runtime from the debugger.
