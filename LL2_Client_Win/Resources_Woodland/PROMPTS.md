# Image generation record

Mode: built-in ImageGen, 2026-09-21. No CLI/API fallback was used.
Selected master files are retained under `Source/`; maps are retained at their
runtime paths under `Background/forest/`. All cropping and UI construction is
reproducible with `tools/Build-WoodlandAssets.ps1`.

## Main atlas (new artwork, no input image)

> Use case: stylized-concept. Create a production 2D side-scrolling fantasy RPG sprite atlas, original artwork unrelated to existing game IP. Transparent background, no text, no labels, no grid lines. Exactly 6 columns and 4 rows of equal cells on a 1536x1024 canvas. Each sprite entirely inside its cell with generous transparent margins. Cohesive polished hand-painted pixel-inspired game art, crisp silhouettes, teal forest and warm amber accents. Rows 1-2: SAME small human swordsman in teal hooded cape, brown boots, cream tunic, short silver sword, facing LEFT, consistent body size. Row 1 six successive walk-cycle poses. Row 2 cells: idle, jump, crouch, sword windup, sword slash, sword follow-through. Row 3: three original forest monsters with two poses each: moss-covered round stone creature idle and step, amber acorn beetle idle and step, purple forest bat wings up and wings down. Row 4: red potion bottle, blue potion bottle, gold coin pouch, pale crystal cluster, curved golden slash effect, glowing teal stone portal. Render as individual usable sprites with genuine transparent alpha. No floor shadows outside cells, no scenery, no checkerboard.

Selected revision (`Source/woodland-atlas.png`), with the preceding generated atlas as edit target:

> Edit this sprite atlas: remove ALL background completely, leaving genuinely transparent alpha around each sprite. Preserve every sprite, all 24 cells, exact layout and art. No colored background, no gradient, no checkerboard. Each sprite must be isolated. Do not change sprites or cell positions.

The image viewer may display colors stored in fully transparent pixels. Alpha was
checked directly before slicing; no color-key background removal was used.

## Map 1

Edit target: `Resources/Background/forest/forest_ground_1.png` from this project.

> Use case: style-transfer. Edit this actual side-scrolling game map into polished original woodland fantasy game art. CRITICAL preserve 1536x1024 map geometry EXACTLY: every walkable platform top edge, gap, ladder position and ladder endpoints must stay at the identical pixel coordinates. Do not add or remove platforms or ladders. Reskin grass to rich moss and tiny amber flowers, earth to beautifully shaded dark ancient stones intertwined with roots, trees to elegant miniature forest trees. Fill empty white spaces with a low-contrast misty teal forest backdrop, distant tree silhouettes, soft sunbeams, atmospheric depth. Playable platforms sharply distinct from distant background. Warm amber highlights, teal shadows. Hand painted crisp 2D game illustration matching a small teal-hooded swordsman and moss golems. No characters, no UI, no letters, no text. Exact same map layout, 1536x1024.

## Maps 2 and 3

Edit targets: the corresponding `forest_ground_2.png` and `forest_ground_3.png`
from the original project. The exact prompt template below was used once per map;
`{backdrop}` was replaced with the indicated text.

> Use case: style-transfer. Reskin this actual 1536x1024 side-scrolling game level into polished original woodland fantasy artwork. Preserve EXACT positions, sizes, top edges and gaps of EVERY platform, ladder and bridge. Keep all geometry identical for existing collision coordinates. Replace plain grass with moss, tiny warm amber flowers, earth with ancient dark stones and intertwined roots, trees with elegant detailed woodland trees. Replace white backdrop with atmospheric low-contrast {backdrop}. Foreground platforms must read clearly against backdrop. Crisp hand-painted 2D game illustration, teal shadows and amber highlights. No characters, no UI, no text. Do not add/remove platforms. Keep 1536x1024 canvas.

- Map 2 backdrop: `sunlit turquoise forest with distant ruins and mist`
- Map 3 backdrop: `dusky teal ancient forest with distant waterfalls and fireflies`

## Additional character actions

Reference image: selected generated main atlas, used to preserve character identity.
Output: `Source/woodland-actions.png`.

> Use case: identity-preserve. Supporting reference is the existing teal hooded swordsman sprite atlas. Create a NEW supplemental sprite sheet of ONLY that same human swordsman, same proportions and outfit. 1536x1024 transparent canvas, exactly 3 columns x 2 rows, six separate sprites, centered within 512x512 cells with ample padding. Row 1: three sequential REAR VIEW climbing poses, arms raised gripping invisible ladder rungs, alternating bent knees, sword sheathed, face NOT visible, teal cape visible from back. Row 2: three sequential LEFT-facing crouching sword thrust frames: preparation, full extension to LEFT, recovery. Genuine transparent alpha background. No gradients, scenery, grid, labels or text. Consistent size within cells. Same original character; no other subjects.

## Item atlas (new artwork, no input image)

Output: `Source/woodland-items.png`.

> Use case: stylized-concept. Original 2D woodland fantasy RPG inventory icon atlas on genuine transparent alpha, 1536x1024 canvas, exactly FIVE equal columns and TWO equal rows, ten evenly spaced individual icons, generous empty padding. Hand painted crisp silhouettes readable at 32x32, dark fine outlines, warm amber highlights, matching teal forest fantasy. Row 1 left to right: red healing potion cork bottle, orange healing potion cork bottle, blue mana potion cork bottle, shiny green apple with leaf, milky white elixir cork bottle. Row 2 left to right: blue spiral snail shell, golden brown mushroom cap, red spiral snail shell, green spiral snail shell, rough copper ore nugget with metallic veins. No lettering, no UI frames, no checkerboard, no backdrop, no shadows outside icons. Each subject strictly isolated in its own cell. All original designs, not from any existing franchise.
