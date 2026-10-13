# Definition files

Every zombie, plant and projectile type spawns with the components its definition file lists. The files live in
`assets/definitions/` in the repo, which the build copies to `definitions/` in the game folder:

```
definitions/
  zombies/football.json
  plants/peashooter.json
  projectiles/pea.json
```

The file name doesn't matter; the type comes from the key inside the file.

## Format

```jsonc
{
  "zombie": "ZOMBIE_FOOTBALL",        // or "plant": "SEED_PEASHOOTER", "projectile": "PROJECTILE_PEA"
  "components": [
    { "type": "TeamComponent", "team": 1, "nativeTeam": 1 },
    { "type": "WalkerComponent", "minSpeed": 0.66, "maxSpeed": 0.68, "houseEdgeX": -175 },
    { "type": "HelmComponent", "helmType": "HELMTYPE_FOOTBALL", "health": 1400, "maxHealth": 1400 },
    { "type": "ZombieBodyComponent", "hitRect": [50, 0, 57, 115], "headImageId": "IMAGE_ZOMBIEFOOTBALLHEAD_ID" }
  ]
}
```

- `"type"` is the component's class name. Every component registered with `REGISTER_COMPONENT` can be used.
- The other keys are the component's fields, named as in its `Serialize`. For example,
  `theArchive.Value("minSpeed", mMinSpeed)` is `"minSpeed"`. A field you leave out keeps the component's default.
- Enum values are names: `"HELMTYPE_FOOTBALL"`, `"PARTICLE_ZOMBIE_HEAD"`, `"IMAGE_ZOMBIEFOOTBALLHEAD_ID"`, `"Glide"`.
  Numbers work too.
- Fixed-size arrays are lists, and structs (a plant shooter's weapons) are nested objects.
- `//` and `/* */` comments are allowed.

The components are attached in the order they're listed, and the type gets exactly those components. A type
without a file falls back to the built-in tables in `Archetypes.cpp`.

## New zombies, plants and projectiles

A file that names a type the game doesn't have, plus the game type it is `"basedOn"`, adds a new type. No code
changes are needed. Examples are in `docs/component-system/examples/`: copy them into `assets/definitions/` to try
them.

```jsonc
{
  "zombie": "ZOMBIE_BLACK_FOOTBALL",      // the new type's name (also its key in the string table)
  "basedOn": "ZOMBIE_FOOTBALL",           // a game type
  "id": 40,                               // optional, see below
  "info": { "name": "Black Football Zombie", "description": "...", "pickWeight": 500 },
  "components": [ ... ]                   // optional: without it, the base's components
}
```

**How a new type plays.** A new type is a variant of its base. In play its zombie, plant or projectile has the base's
type (`mZombieType` / `mSeedType` / `mProjectileType`), so every rule still written per type in the code treats it like the base: where it
can be planted, how it enters the pool, special packets and so on. Its own type (`mDefinitionType`) picks:
- its components (this file);
- its animation;
- its name and Almanac texts;
- its cost and recharge (plants), its damage and image (projectiles);
- for zombies, when it shows up in levels.

**`info` for a zombie.** Every field is optional and defaults to the base's.

| Field | Meaning |
|---|---|
| `name` | Name shown in the Almanac and on hover |
| `description`, `descriptionHeader` | Almanac texts |
| `reanim` | A game reanimation (`"REANIM_ZOMBIE_FOOTBALL"`) or a `.reanim` file (`"reanim/Zombie_blackfootball.reanim"`) |
| `value` | Wave points it costs |
| `startingLevel` | First adventure level it can appear on |
| `firstAllowedWave` | First wave it can appear in |
| `pickWeight` | How often it is picked for waves. **0 unless set**: a new zombie stays out of levels until this opts it in |
| `allowedLevels` | Adventure levels (1-50) it may appear on |

**`info` for a plant.** Every field is optional and defaults to the base's.

| Field | Meaning |
|---|---|
| `name`, `tooltip`, `description`, `descriptionHeader` | Seed packet and Almanac texts |
| `reanim` | As for zombies |
| `cost` | Sun cost |
| `refreshTime` | Recharge, in ticks (750 = 7.5 s) |
| `launchRate` | How often its producer or shooter acts, when its components use the plant's rate |
| `subClass` | `SUBCLASS_NORMAL` or `SUBCLASS_SHOOTER` |

**`info` for a projectile.** Every field is optional and defaults to the base's.

| Field | Meaning |
|---|---|
| `damage` | Damage per hit (splash damage follows from it, as for its base) |
| `image` | Its picture: a game image (`"IMAGE_PROJECTILEPEA_ID"`) or an image id from `properties/resources.xml`. The image's group has to be loaded when it is first drawn |
| `imageRow` | Which row of the image to draw (0 when it has its own `image`) |
| `scale` | Size it is drawn at |

A new projectile flies, hits and splashes like its base: a lobbed base arcs, a fireball base burns. Plants and
zombies fire it by naming it in their components (a `PlantShooterComponent` weapon's `"projectile"`, a
`ShooterComponent`'s `"projectileType"`), and a `ProjectileConverterComponent` (Torchwood) can turn projectiles
into it or turn it into something else. It converts like its base unless the converter lists it by name. See
`examples/projectiles/big_pea.json` and `examples/plants/big_peashooter.json`.

**Where they show up.**
- New zombies stay out of levels until their `info` sets a `pickWeight`. With one, they join the waves of the
  levels their base appears on (or `allowedLevels`), from `startingLevel` and `firstAllowedWave` (the base's
  unless set).
- New plants are unlocked when their base is, and appear on extra pages of the seed chooser. The "More Plants"
  button sits above the Almanac button.
- Both appear on extra pages of the Almanac, behind the "More Plants" / "More Zombies" button between Index and
  Close.
- Both are listed in the debugger's spawn menus.

**IDs and saves.** New types get IDs after the game's own: zombies from 35 to 99, plants from 75 to 138,
projectiles from 14 to 77. A save remembers types by
ID. Without `"id"`, a file gets the next free ID in file-name order, so adding or removing other new types can
shift it. Give each new type a fixed `"id"` once saves matter.

**Custom animations.** `"reanim": "reanim/My_zombie.reanim"` loads a reanimation file that isn't one of the game's
(up to 64 of them). Any reanimation field of a component can name such a file too. The images it uses must be
listed in `properties/resources.xml`, like the game's own.

**Limits for now.**
- `basedOn` must be a game type, not another new type.
- Zombies that bring others along spawn their base's companions: a new Dancer's backup dancers and a new Bobsled
  team's followers are the vanilla ones.
- Crazy Dave's picks, the Imitater and the random seed button only use the game's plants.

## Seeing problems

Unknown component types, unknown fields (typos), wrong value kinds and unknown enum names are reported:
- on stderr when the files load;
- in the debugger (F1 → Board → Definitions), which also has a **Reload definitions** button. Edits apply to what
  spawns after reloading.

## Tools

Both run without starting the game, and log to `definitions_tool.log` next to the folder:

```
LawnProject.exe -exportdefinitions=<folder>   # write the built-in tables as definition files
LawnProject.exe -checkdefinitions=<folder>    # check every file builds exactly what the built-in tables build
```

The vanilla files in `assets/definitions` were made with the export, and pass the check. Run the check after
changing a vanilla component's defaults or its `Serialize`, so the files and the tables stay in step.

## When the code changes

- **New field in a component:** add it to `Serialize` with a name, at the end (saves read fields in order).
- **New enum values:** rerun `python tools/generate_enum_names.py` so the names are known.
- **Renamed field key:** existing definition files using the old key will report it as unknown.
