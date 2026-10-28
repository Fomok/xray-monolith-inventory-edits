# SQA3 visual assets v1 — 28 October 2026

Ready for Engine integration; **not installed or accepted in game**. This package replaces the rejected stretched-background / metal-grid-cell treatment. It contains editable deterministic composition source, two DDS atlases, XML texture descriptors, provenance, a contact sheet and an empty layout proof. No screenshot or generated concept supplies texture pixels. No native C++, Engine draft assets, installed files, website or release was changed.

## Files to integrate

Copy these three files from this directory's `package/gamedata/` into the isolated Engine overlay, preserving relative paths, then ensure the new texture descriptor is loaded by the existing descriptor loader:

| Relative path | SHA256 |
| --- | --- |
| `configs/ui/textures_descr/sqa3_visual_assets.xml` | `0c2dd24e3bae62fc9e0d831302b283ee5ed079d251f280492a3a7d8040ddebe5` |
| `textures/ui/sqa3_visual_panels.dds` | `fcf6ac47939bb059ed3c8defe352b313f3c09c54035b411b4c33a4c7f977b266` |
| `textures/ui/sqa3_visual_controls.dds` | `5cd44561009aef6d0b06a1a702abe3b93b1d787d5a72830243138aa02b8f3bbc` |

New IDs are deliberately namespaced `sqa3v1_`; this avoids collisions with released SQA2 IDs or Engine's uncommitted `sqa3_primitives.dds` / `sqa3_native.xml` draft. Switch the relevant call sites to these IDs rather than redefining released slot textures globally. Engine owns edits to its `native-visual-assets.json` and overlay packaging. That draft and all native files were left untouched.

## Exact texture contract

All final assets use **2 raster pixels per 1024×768 logical UI unit**. Source atlas pixels and generated reference pixels are not layout coordinates. The XML and `asset-manifest.json` contain exact atlas XYWH for all IDs. DDS is uncompressed RGBA, one level, without mipmaps; panel atlas 2048×2048, controls 1024×1024. Interiors are opaque. Two-pixel edge extrusion protects linear sampling; use exact registered UV rectangles.

| ID | Logical size / use |
| --- | --- |
| `sqa3v1_equipment_panel` | 260.5×768 blank column with outer metal rim and tiled fine dark lining; no baked slots. |
| `sqa3v1_storage_panel` | 382×768, same treatment for storage and conditional nearby column. Adjust third column width to available viewport without changing height. |
| `sqa3v1_surface` | 128×64 lining tile. Do **not** stretch this across a panel. Prefer precomposed panels; tile when generating another size. |
| `sqa3v1_panel_frame` | 64×64 source frame, nine-slice suffixes below, 6-unit borders. Do not use its center as a stretched full-column background. |
| `sqa3v1_slot_frame` | 100×96, nine-slice with 3-unit borders; equipment only. |
| `sqa3v1_slot_helmet` | 75×96; helmet and backpack. |
| `sqa3v1_slot_chest` | 75×150 shared chest view. |
| `sqa3v1_slot_small` | 75×42; torch/PDA/detector, shared knife/binocular view, pistol visual placeholder. |
| `sqa3v1_slot_weapon` | 237×96, both long-weapon rows. |
| `sqa3v1_slot_quick` | 34.5×46, four quickslots plus grenade/bolt row. Draw live key labels separately; do not bake names/icons. |
| `sqa3v1_section_bar` | 354×22 olive fabric with metal rim. Nine-slice aliases available, 3-unit borders. No baked name, weight or chevron. |
| `sqa3v1_identity_bar` | 354×56 olive identity/header region. |
| `sqa3v1_grid_cell` | 32×32 reference pitch, flat dark fill with 1-unit top/left lines, no frame. Repeat per actual grid cell. Draw bottom/right outer line once. |
| `sqa3v1_grid_fill` | Uniform dark fill, freely stretchable. |
| `sqa3v1_grid_line` | 1×1 uniform line; stretch into 1-unit lines at authoritative cell boundaries. Prefer this plus fill if cell pitch differs, to keep line thickness constant. |
| `sqa3v1_arrow_left_e`, `sqa3v1_arrow_right_e` | 14×20 rectangular chest tabs; textured chevrons, no font characters. |
| `sqa3v1_arrow_up_e`, `sqa3v1_arrow_down_e` | 18×18 square collapse/expand buttons. |

Each arrow direction has `_e` enabled, `_h` hover, `_t` pressed and `_d` disabled. Wire actual state textures and keep button text empty. `sqa3v1_panel_frame`, `sqa3v1_slot_frame` and `sqa3v1_section_bar` expose `_lt`, `_t`, `_rt`, `_l`, `_back`, `_r`, `_lb`, `_b`, `_rb` aliases for native frame-window use. Here `_t` means top slice, distinct from arrow pressed-state IDs. When using nine-slices, explicitly set the intended logical border thickness; do not assume raw atlas pixel width equals UI units.

## Layout reference, not a native control replacement

`layout-reference.json` gives coordinates used in `layout-proof.png`, on a 1024×768 logical canvas. Its empty example grids do not declare capacity, rig presence, eligible loot or gameplay state. Engine must use actual ownership/capacity/source results.

- Equipment column x=0, width=260.5. Helmet x=12,y=9; backpack x=12,y=117. Chest x=93,y=9; shared knife/binocular visual x=93,y=171. Detector x=174,y=117; pistol **visual only** x=174,y=171. The shared visual is not permission to merge native equipment ownership or remove binocular access.
- Chest tabs x=86 and161,y=74, size14×20. Their centers are exactly on chest borders x=93 and168 and vertical midpoint y=84. Do not inset them into the chest image.
- Both weapon rows x=12, y=223/325, size237×96. F1–F4 frames at x=12/52.5/93/133.5, y=433; grenade/bolt alongside at174/214.5. No separate binocular row between guns and quickslots.
- Released source has quick item cells33×44 at `ui_inventory_16.xml`:332–375; these frame dimensions are derived from the existing34.5×46 grenade/bolt frame family. Fit native icon controls within the recess with aspect preserved. Keep key labels live at its top edge; the proof's bottom labels are annotations only.
- Storage x=260.5,width382; nearby x=642.5,width381.5 in the proof. Whole nearby panel is conditional. Engine owns its selector/source labels and visibility; the repeated empty sections in the proof are material examples, not a requested two-source display policy.
- Header inset14; width354; reserve the remaining14 units before the next column for rim/scrollbar. Center titles over the bar, place collapse square3 units from its left edge, right-align real weight with6-unit right inset. Clip title between collapse and reserved weight width. Neither text nor bar may extend under the scrollbar. If actual scrollbar geometry requires more room, shorten the bar while retaining fixed borders.
- Center each storage grid in the usable content viewport, not across any old equipment rail. No equipment rail exists in these assets. Use real content weights; the proof deliberately does not invent values. Recovery visibility and transfer rules remain Engine behavior.

## Provenance / deterministic editing

`build_assets.py` uses released `sqa_fk_panels.dds` and `sqa_fk_controls.dds` from `C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source/gamedata/textures/ui/`. Exact source hashes are in `asset-manifest.json`.

Source XYWH crops: lining(8,1700,256,128), olive(280,1700,256,128), slot recess(40,26,200,192) from panels; outer frame(8,8,128,128) from controls. Panel centers tile lining at native raster density; source frame corners stay fixed and only edge strips resize. Section centers tile olive; slot recesses use source nine-slice composition. Grid is authored two-color geometry. Chevrons are deterministic antialiased polygon artwork composited over source material, with shadow and separate state treatments; no font dependency in runtime textures. Segoe UI is used only in offline review captions.

Reference images inspected: user screenshot `C:/Users/fomok/AppData/Local/Temp/codex-clipboard-4f382fb5-3015-4075-b6c4-27c3af5fa07b.png`; released appearance `C:/GAMMA/zonebench-svelte/web/assets/gameplay/inventory.png`; accepted generated appearance `C:/Users/fomok/.codex/generated_images/01a0ef99-0c2c-7612-a635-58fdbb8fd01b/exec-5e4734f3-7648-422d-8990-a521aca6de6e.png`. These supply visual comparison only, never texture pixels or dimensions.

Rebuild with the bundled Python (Pillow): run `build_assets.py`, then `verify_assets.py`. `validation.json`: PASS,31 base assets/58 descriptor IDs; DDS versus PNG pixel equality, unique/in-bounds IDs, opaque interiors, nonoverlapping base rectangles, four distinct button states, two-color unframed grid, payload hashes. Contact sheet and layout proof visually inspected. No native run/build/install occurred in this asset task. Engine still must verify descriptor loading, scale/aspect, hover/press/disabled mapping, actual scroll clipping and shared-view controls in game.
