# Engine to Mod/UI handoff - 28 October 2026

Asset editing paused after user requested specialist split. Drafts are NOT accepted.
Do not commit Engine's C++ or package changes from the asset task.

## Asset files already modified (uncommitted)
- tools/inventory_lab/gamedata/configs/ui/textures_descr/sqa3_native.xml: outer panel nine-slice changed from small equipment rim to released inventory outer rim; added four arrow IDs.
- tools/inventory_lab/native-visual-assets.json: provisional primitives entry with repo_relative pinning.
- tools/inventory_lab/gamedata/textures/ui/sqa3_primitives.dds: provisional deterministic 256x32 RGBA atlas. Mod/UI owns replacement.
No generator file was added. Initial atlas was generated inline; replace with your reproducible asset tool.

## Current native integration contract (provisional, coordinate changes here)
- sqa3_arrow_left/right/up/down: full button textures, currently24x24 crops at x32/64/96/128,y0. Chest draws left/right10x18, storage collapse20x20. No text glyphs.
- Grid shader currently planned ui/sqa3_primitives; cell at0,0,32x32 in256x32 atlas. Prefer a stable named sqa3_grid_cell descriptor: Engine will resolve the descriptor shader/UV instead of hardcoding atlas dimensions if your final layout differs.
- sqa_fk_panel: CUIFrameWindow nine-slice outer container/equipment rim. Its back is tiled at source pixel size; do not use tiny fabric swatch.
- sqa_fk_inventory_panel: exact full released panel for main inventory and conditional loot backgrounds.
- sqa_fk_slot: equipment/quickslot frame nine-slice, reserved for equipment only.
- sqa_fk_header: olive center and metallic header rim. Native header width excludes scrollbar plus6px padding.

## Engine-owned integration files already changed
- sqa3_field_kit.xml: removed redundant labels/stretched surfaces; inventory/loot use complete public panel. Slot5 moved onto knife location93,171; quickslot list12,433 with33x44 cells at40px pitch; bolt174,433 and grenade214.5,433. Engine handles actual list visibility/shared visual semantics. Please propose any coordinate changes in your handoff rather than concurrently editing this XML.
- package_native_lab.py reads extra repo_relative assets from pinned git overlay, leaving public source atlases exact-hash verified.

Engine will wait for final assets before package/visual acceptance. Current installed Dev29 is rejected visual smoke; latest b41a9605 recovery fix is in CI. No game running after user interrupted Computer Use. Do not install or launch from the asset task.

Native grid contract confirmed: CUITextureMaster resolves **sqa3_grid_cell** shader/region and active texture dimensions. Please add that descriptor ID to final XML; no fixed atlas size or crop required. Four arrow IDs remain as above.


## Superseding final integration

Mod/UI final handoff visual-assets-v1 is integrated at e05ff97d, using sqa3v1_
IDs and separate sqa3_visual_assets.xml. Prototype primitives DDS was removed;
prototype descriptor modifications reverted before integration. Exact new DDS
and descriptor are copied from Mod/UI's verified package. Runtime buttons use
CUI3tButton with all four state textures; panel and equipment artwork use measured
precomposed assets. The earlier provisional texture contract above is historical.
