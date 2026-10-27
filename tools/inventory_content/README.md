# SQA3 content catalog v1

This standalone Python standard-library tool extracts the authoritative full SQA2
release and optional Stash Grid Test 2 package into a versioned, provenance-bearing
JSON interchange catalog. It does not execute Lua, load the game, install content,
copy Knox code or modify the native core. No runtime monkeypatch is carried over.

The companion rule compiler now normalizes authored geometry, pouch grants,
stack precedence and acceptance families. See [RULES-HANDOFF.md](RULES-HANDOFF.md)
for exact native mappings, unresolved cases and required model/adapter extensions.

From the worktree root:

```powershell
python -B tools/inventory_content/validate_catalog.py --output-dir C:/GAMMA/sqa-previews/inventory-next-20261027/content-validation
python -B tools/inventory_content/extract_catalog.py --output C:/GAMMA/sqa-previews/inventory-next-20261027/content-validation/catalog-v1.json
```

The extractor also accepts explicit `--source` and `--stash` roots. Validation is
deliberately pinned to the known release defaults; it is not a validator for an
arbitrary replacement release. Missing config roots fail. Extraction errors return
nonzero, with the diagnostic catalog still written for inspection.

## Source contract

- Release: `C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source`.
- Stash: `C:/GAMMA/sqa-previews/stash-grid-marker-test/package`.
- Every file under each layer's `gamedata` has a byte size and SHA-256. All LTX/XML
  documents retain source text, including comments and unsupported syntax. Legacy
  non-UTF8 source text uses a reversible Latin-1 byte mapping, explicitly marked
  as unknown locale; XML semantic parsing respects the XML byte encoding declaration.
- Each optional directory is independent. The tool never silently overlays SOTA,
  ZONA or the stash package on the release. In particular, old stash scripts must
  not replace hotfix base scripts merely because they appear in this catalog.
- Provenance uses profile-relative file IDs and one-based LTX lines. XML values
  have file/element references. Paths in `sources` locate the input roots.

## JSON schema and native adapter handoff

`schema = sqa.inventory-content`, `schema_version = 1.0.0`. Breaking changes require
a new major version. Unknown keys should be retained by downstream tooling.

| Field | Meaning and adapter use |
|---|---|
| `files`, `input_manifest_sha256` | Exact input identity, including runtime scripts/assets; scripts are hashed, not interpreted. |
| `documents` | Source text and ordered LTX sections/entries. Stable record ID is file ID plus header line. Repeated keys, section patches, inheritance lists and unknown records are retained. |
| `profiles[].definitions` | Sorted system-section projection after lexical mod_system filename order. Includes non-item system sections such as inventory; do not spawn every record. |
| `own_fields`, `fields`, `history` | Local assignments, partially inherited projection, and complete contributing section IDs. Every projected field retains its authoring line. |
| `resolution`, `unresolved` | `local_complete` only means no known unresolved local graph dependency. `partial` must not become a spawn-ready item without host resolution. |
| `semantics` | Raw authored exterior grid fields, text IDs, icon atlas, amp rig/pouch/box properties, repair and equipment fields. No numeric defaults or class guesses. |
| `tables` | Separate acceptance, stack, backpack, grants, repair, recipe, trader and loot config domains. Never merge same-named numeric sections across files globally. |
| `recipes` | Authored output section, toolkit number, book reference and ingredient/count pairs, with original record retained. References explicitly say local-declared or external-definition-required. |
| `backpack_sizes` | Authored rows first, columns second; exact cell product. Legacy scalar count has null columns. UI reflow is separate. |
| `localizations`, `texture_regions` | Text per language/profile and atlas rectangles; duplicates are retained and diagnosed rather than overwritten. |
| `dependencies`, `diagnostics` | External bases/override targets, unresolved host table semantics, text/assets and includes; errors distinguish malformed data from expected missing host content. |

Plain system declarations are validated for empty/non-ASCII/over-63-byte IDs and
duplicates. Multiple plain definitions are errors, not last-wins success. Ordinary
field assignment history is retained in documents, with the last assignment used
in the partial projection. Parent fields merge in listed order, then local fields.
Unknown section/key operators and override inheritance edits force partial status;
their original syntax remains available. Inheritance cycles force partial status.
This is deliberately not a replacement DLTX interpreter: an engine loader must
resolve actual include order, host sections and DLTX mutations before final import.

## Required native integration work

1. Supply exact host/game/mod definitions and importer order for `af_base`, adopted
   boxes, existing pouches/backpacks, repair lists, recipe books/ingredients,
   traders and loot. Resolve IDs using the host domain, not arbitrary token matches.
2. Select installer layers explicitly. Integrate native classes, object factories,
   slots and existing asset/localization lookup. Missing assets may be inherited
   or supplied by another layer; absence here is not proof of a broken game asset.
3. Convert raw rig layouts, pouch grants and box shaped slots to stable native
   compartment IDs. Do not infer native slot numbers from old UI cell indices.
4. Implement stack rule precedence, settings multipliers, acceptance predicates,
   occupied-pouch refusal, condition gates and repair/craft/trade/loot transactions
   against the shared core. Table rows are data, not executable policy.
5. Encode SQA3 nesting policy deliberately: soft bags/rigs may nest as permitted;
   rigid-case acceptance remains restricted. The catalog does not impose SQA2's
   runtime blanket container-in-container rejection or invent growth curves.
6. Bind per-instance state (condition, magazine contents, upgrades, parts), native
   entity lifetime and save semantics independently from item definitions.
7. Validate in an isolated native session. Parser tests are not gameplay evidence.

## Verification and scope

`test_catalog.py` covers inheritance/override provenance, cycles/external parents,
duplicate/missing/overlong IDs, duplicate keys, unsupported operations, known rig,
tall pouch, long-ID artifact bag, pistol case, backpack orientation, acceptance,
stack ceiling, recipes, localization/profile separation, every file hash, JSON
round-trip and byte-identical independent rebuilds. `validate_catalog.py` writes
the catalog, full test log, counts, hashes and `game_validation: not_run`.

The coordinator owns paired vault updates and the engine commit. Only this tool
directory was added to the worktree; evidence is outside it in the requested
preview folder. No core edit, commit, installed-file read/write or game launch is
needed for this work. See the generated validation report for actual results.
