# Native catalog content payload contract

Engine choice for new rig/box/pouch items: native `II_ATTCH` (`CInventoryItemObject`, registered with `CSE_ALifeItem` in xrServerEntities/object_factory_register.cpp). Use vanilla `tch_junk` as the common config parent, overriding class explicitly to II_ATTCH. Keep the five existing vanilla backpack sections/classes intact. Do not copy II_CONTR or the old inventory scripts.

Required per new section: original section ID, name/short-name/description, intrinsic inv_weight, cost, kind, icons_texture and raw inv_grid_x/y/width/height plus inv_grid_scale if authored. Retain source provenance and mark external icon overrides separately. Set slot=-1 and auto_attach=false; do not reuse rig slot15 or16. Native SQA3 registration owns equipment; portable item config alone does not make an equipment slot.

Enable use_condition for rigs/pouches with original or documented common condition/immunity defaults, inheriting required stock object fields. No artifact regeneration/radiation gameplay is needed from old af_base ancestry. Preserve actual acquisition/upgrade/recipe data as audited records for Engine integration; do not fabricate dependency completeness.

Use original shared physical meshes at meshes/squared_away/rig_shared.ogf and box_shared.ogf, with exact referenced textures, and original UI atlases. Include the Mags ui/ui_magpouches.dds dependency for its three base pouches if absent from vanilla. New item names should resolve in English for the manual test; preserve released localization files where appropriate. Any new fallback labels must be clearly identified.

Provide isolated assets-package/gamedata plus an exact file/hash/provenance manifest under catalog-handoff. Config templates may remain separate from the ready-to-copy assets if engine-owned registration details are unresolved. No installed game/mod writes, no public changes. Engine will pin/import/package payloads and perform actual native spawn, equip, storage and save tests. This content preparation does not certify gameplay integration.
