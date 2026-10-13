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
