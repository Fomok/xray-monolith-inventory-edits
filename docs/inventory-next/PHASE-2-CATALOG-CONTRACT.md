# SQA3 catalog handoff for Phases 2 and 3

28 October 2026. Engine requests audited data from Mod/UI, without duplicating its extraction. Implementation owner: Engine. This is a data contract, not a claim of integration.

Provide UTF-8 JSON with schema `sqa3.catalog.source.v1`, source release/hash, and an `items` array. Each record needs:

- `section`, `family` (backpack, rig, pouch, box or ordinary item), source path/section and effective inheritance/DLTX provenance.
- `exterior`: original logical width/height, rotation permission; for backpacks/rigs, proposed empty/half/full sizes with provenance or explicit authored proposal. Every other family remains fixed.
- `interior`: width/height, row-major `cells` (0 blocked, positive compartment ID), `compartment_names`, and any compartment-specific acceptance. Backpacks use this same shape contract as rigs; do not flatten masks into capacity counts.
- `mounts`: stable authored key, original anchor/extent coordinates, permitted pouch families, condition threshold and enabled state. Include the original rig layout and pouch extension geometry exactly; no automatic rearrangement.
- category/acceptance tags, stack limits by domain, mass/condition/state rules, equipment compatibility and dependencies. Keep unknown values explicit rather than inventing defaults.
- `assets`: actual icon/model/texture/config references, source provenance and required dependencies. Separate reusable artwork/data from scripts that must be replaced.
- acquisition: trader/new-game/spawn section IDs and recipe/upgrade/dismantle references, including dependencies unavailable in vanilla. Phase3 testing needs a documented easy way to obtain each representative item.

Also provide an audited `rig_layouts` table preserving released geometry, a pouch-to-mount compatibility table, and unresolved definitions. Supply at least one representative backpack with compartments, fixed box, original rig, matching pouch and damaged rig before the entire catalog if practical.

## Current user policy

Default policy forbids putting any container inside another container. Express that as configurable acceptance data, not a class hardcode. Existing nested saves must remain loadable and recoverable without destruction or extra ordinary-pickup capacity. Future data may permit selected nesting without engine redesign.

This restriction applies to a portable container's storage interior. Actor base pockets/root and world stash roots can accept container items. Equipped bags, custom equipment slots and mounted pouches are equipment relationships and remain permitted. Test those distinctions explicitly; do not reject a mounted pouch as storage nesting.

Only backpacks and rigs may change exterior footprint. Pouches and every other container remain fixed. Preserve rig cell/mount geometry. The native renderer centers the whole effective layout bounding box and recenters after pouch attachment/removal; it must not change logical coordinates to center artwork.

Equipment registration must use stable namespaced keys and persistent IDs outside the complete reserved vanilla slot range, including unused slots. Engine owns the runtime slot registry and ownership/save/event implementation; catalog supplies compatibility tags and desired registration keys. Do not assign arbitrary native slot integers or repurpose unused vanilla slots. Pistol frame remains visual only.

## Delivery

Put audited JSON, provenance and a concise handoff under an isolated preview directory; give Planning its exact path. Engine will validate/import into versioned SQA3 definitions and implement native adapters. No installed/public changes are required for this source handoff. ZoneBench authoring integration remains a later phase; establish exportable field semantics now.
