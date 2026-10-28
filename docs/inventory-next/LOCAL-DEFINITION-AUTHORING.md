# Local SQA3 storage definitions, schema1

The command-line authoring tool validates definitions before replacing a destination. Use a development folder, package it, then install only a matching verified lab package. Restart the game to load changed definitions. These commands do not change a save or update ZoneBench.

From the repository root, validate the pinned defaults:

```powershell
python tools/inventory_content/native_definitions.py validate tools/inventory_lab/gamedata/configs/sqa3_catalog.json
```

Export a native catalog to an editable JSON preset, then import the edited preset to a separate development LTX:

```powershell
python tools/inventory_content/native_definitions.py export tools/inventory_lab/gamedata/configs/sqa3_catalog.ltx --output C:/GAMMA/sqa-previews/my-storage-preset.json
python tools/inventory_content/native_definitions.py import C:/GAMMA/sqa-previews/my-storage-preset.json --output C:/GAMMA/sqa-previews/my-storage-catalog.ltx
```

Reset a development LTX using the pinned defaults as the source:

```powershell
python tools/inventory_content/native_definitions.py reset tools/inventory_lab/gamedata/configs/sqa3_catalog.json --output C:/GAMMA/sqa-previews/my-storage-catalog.ltx
```

The source is never overwritten. Validation errors exit2 and identify the section/field; an existing output remains unchanged. Successful replacement uses a temporary sibling file, flush and atomic replace. Keep your exported preset if you want to undo a reset.

JSON uses `schema: sqa3.storage.preset.v1` and a `definitions` map keyed by native item section. Each definition contains family, width/height, row-major cells, empty/half/full `[width,height]`, growing, allow_container_items, exterior_independent, max_contents_grams and stack_limit. Optional tear_thresholds apply only to rigs. Optional acceptance has all four pipe-separated lists: accept_kinds, accept_sections, reject_sections, accept_flags. Classifier flags are ammo, weapon, magazine and rig. An explicit rejection wins over a section allowance; the portable nesting prohibition wins over category permission. Actor/world roots and equipment relationships use their separate ownership rules.

Cells use0 for blocked space and positive compartment IDs; one item cannot straddle compartments. Dimensions are1..64. Growth is allowed only for backpack/rig families and must be monotonic. Fixed families use equal stages. The explicit independent-exterior policy allows a small exterior icon/footprint to represent a larger authored interior. Source thresholds are descending condition basis points; largest rig compartments tear first, later authored ID breaks ties. Pouch grants remain separate and use their mount thresholds.

The tool migrates the earlier `sqa3.native.catalog.v1` offline catalog to this preset schema. Unknown versions and definition fields are rejected. This is authoring-format migration, not SQA2 save migration. Native LTX declares `[sqa3_catalog_metadata] schema_version = 1`; unsupported versions are refused by the adapter. Item classes, artwork, custom equipment registration and acquisition are separate package inputs; authoring a storage definition does not register a new native item by itself.

Validation evidence: `tools/inventory_content/test_native_definitions.py` covers all49 definitions through native LTX roundtrip, invalid output preservation, unsupported version/family/growth/classifier rejection and malformed row dimensions. Native catalog acceptance is tracked separately in STAGE1-3-COMPLETION-CHECKLIST.md.

Five backpack defaults use explicit rectangles in `tools/inventory_content/native-backpack-layouts.json`: main/side/lower/base compartments with blocked lower corners. Their JSON provenance distinguishes newly authored SQA3 development geometry from the22 recovered rig masks. Re-running the importer preserves this distinction.

### Backpack capacity audit
The new development layouts preserve original usable-cell counts: itm_backpack 64 to 64; equ_small_pack 80 to 80; equ_small_military_pack 96 to 96; equ_military_pack 104 to 104; equ_tourist_pack 120 to 120. A four-cell base pocket compensates for four blocked lower-corner cells; overall presentation gains one row. No usable-cell capacity reduction is intended. Compartment boundaries deliberately prevent one item spanning separate pockets, so some shapes may need rearrangement/recovery after a definition change. Growth thresholds remain the already disclosed provisional development balancing. native-backpack-layouts.json records original dimensions and both usable counts; import rejects unequal totals. All22 original rig masks remain unchanged.
