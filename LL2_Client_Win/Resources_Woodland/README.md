# Woodland resources — animation revision 2

This revision changes PNG assets, JSON data and tooling only. Existing C++ source
and the user's direct `Resources_Woodland` path changes are preserved.
Original `Resources/` artwork remains untouched.

Trade UI removes the PARTNER / YOUR OFFER labels. Generated UI titles, inventory
categories, confirmation/login/trade buttons and trade guidance use Korean with
Malgun Gothic. Original image dimensions and click regions are unchanged.

## Animation changes

- Monster skill-hit VFX uses a separate six-frame radial flash/ring/ember sequence
  centered on the hit target (96x96 canvas, origin 48,48, offset 0,0).
  It has no left/right travel direction, matching the existing directionless PlayHit call.
  Built-in ImageGen source and prompt: `Source/AnimationV2/hit-impact.png` and
  `hit-impact-prompt.txt`. Rebuild with `tools/Build-WoodlandHitEffect.ps1`.
  Previous hit images and original VFX JSON are backed up under that source folder.

- Warrior walking: eight armed poses with forward/backward arm swing at 0.10 seconds each.
  A common 96x80 canvas, one scale and a fixed foot baseline replace separate fitting.
- Monsters: six distinct species with 96x96 canvases and one scale per species
  across idle, movement, hit and death. Blinking idles have no frame-to-frame zoom.
- Hits: impact recoil, recovery and neutral, preserving `MonsterHitEnd`.
- Death: collapse and fade, preserving `MonsterDieEnd`.
- Slash VFX now faces left in source images, matching the renderer's right-facing flip.
  Existing VFX canvas dimensions, origins and offsets are preserved.
- Updated art uses built-in ImageGen. Sources and exact prompts are in
  `Source/AnimationV2/walk-armed.png`, `slash-left.png`, and `armed-walk-vfx-prompts.json`.
  Previous walk/effect assets are preserved in `BeforeArmedWalkAndVFX` there.
- Monster timing remains the existing C++ value of 0.2 seconds per frame:
  hits last 0.6 seconds. JSON timing alone cannot override that existing constant.

| ID | Species |
|---|---|
| 100100 | Moss Golem |
| 100101 | Acorn Beetle |
| 100102 | Violet Forest Bat |
| 100103 | Mushroom Sentry |
| 100104 | Briar Boar |
| 100105 | Thorn Sprout |

All six are registered in client JSON. New IDs require server spawn configuration
to appear in maps; server code/data is not present or modified in this workspace.
Existing collision and projectile data are preserved. Image origins align the
new canvases with the existing collider bottom.

## Files and use

`Data/Character/...` and `Data/Monsters/...` now contain the animation frame counts
and origins, so the user's direct-resource loader can consume them normally.
Previous JSON is retained in `Source/AnimationV2/OriginalData/`. New definitions
are listed separately in `Source/AnimationV2/created-data.json`.

`Play-Woodland.cmd` runs a PowerShell preparation script, then launches the client.
The prepared `.woodland-runtime/` contains both `Resources/` and
`Resources_Woodland/` paths, supporting either the original or directly changed
loader. It does not overwrite original artwork. A normal Visual Studio launch
also uses the updated JSON when its working directory is `LL2_Client_Win`.

PowerShell scripts are offline development tools, not C++ game components:

```powershell
./tools/Prepare-WoodlandRuntime.ps1        # prepare files only
./tools/Prepare-WoodlandRuntime.ps1 -Launch
./tools/Build-WoodlandAnimations.ps1       # rebuild walking/monster PNGs and JSON
./tools/Build-WoodlandAssets.ps1           # rebuild complete pack, then animation v2
./tools/Set-WoodlandTheme.ps1 Off          # original art/data through the launcher
./tools/Set-WoodlandTheme.ps1 On
python ./tools/Verify-WoodlandAssets.py    # requires Pillow
```

The Off option applies to `Play-Woodland.cmd`, preparing an independent
`.woodland-original-runtime/` using preserved JSON and original images. It does not
change C++ path strings or the active development Data files. Previously compiled
binaries with the earlier C++ theme implementation should be rebuilt from current
source once; this animation revision itself requires no C++ changes or rebuild.
Login still requires the normal server and account.

## Checks / limitations

405 runtime PNGs and 170 animation/VFX references validated. Walking leg silhouettes
are distinct; all monster states share one scale and baseline; idle bounds vary by
at most two raster pixels; all hit frames and completion events exist. Collider and
projectile definitions match backups. C++ hashes match this revision's starting state.
No live server gameplay test was performed. `preview.png` is the earlier static
asset composition, not a current gameplay screenshot.

## Provenance

Built-in ImageGen, 2026-09-21; no CLI/API fallback. Revised masters, prompts and
frame transforms are under `Source/AnimationV2/`. Original pack prompts are in
`PROMPTS.md`. PowerShell/System.Drawing performs crop, shared scaling and placement.
The new monsters and walking art were generated for this project. Existing map art
used previous map images as layout/edit references; their rights were not independently
cleared. BGM and preserved original files retain their existing licenses.
