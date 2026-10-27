# Native placement development slice

Only `-inventory-lab -dbg` enables this adapter. It projects actual actor-owned items into a development16x64 grid; it does not enforce final pickup capacity, equip items, or change physical ownership. Do not promote it as complete SQA3.

`inventory_next_probe export` reconciles native item facts and exports to lab appdata. `inventory_next_probe view` opens the native window. `inventory_next_move <core ID> <x> <y> <0|1 rotated>` validates through the same model as view controls. Escape closes. Layout persists in ALife chunk0x33514c59 alongside the identity ledger. Saved facts are never authoritative over current native mass/condition/size. Failed moves leave placement intact.

## Isolated runtime fixture

Use only the disposable lab. Preserve original test save, verify no game process before launching, use one windowed process and terminate that owned PID on timeout. `keypress_on_start off` avoids the startup input gate. Base arguments: `-dbg -inventory-lab -nosound -start server(test/single/alife/load)`.

The autorun script is inert without `-sqa3-fixture`. Add `-sqa3-fixture-spawn` to create bandage/medkit through actual ALife spawn; add `-sqa3-fixture-edit` to move100 to4,3 and attempt overlapping101 and negative bounds. Saves to sqa3_layout and quits. Reload that save with `-sqa3-fixture-reload` (without spawn/edit); export, screenshot, save sqa3_layout_reload and quit. Compare native identities/mass and exported positions across processes. Do not launch concurrently. Automated domain commands do not establish physical mouse input success.

## Visual assets

The checked-in sqa3_native.xml contains nine slices from the existing Squared Away Field Kit controls atlas, with center aliased to native _back. Supply the unmodified atlas recorded in tools/inventory_lab/native-visual-assets.json. Do not copy legacy SQA scripts. Item atlas and rectangle come from each native item using vanilla icon rules. Current rotated icon fallback is text.

## Validation

76 core behavior tests in debug/release and independent restart pass. Native acc4e204 build/runtime verification pending. Prior4e9a11c0 runtime verified PDA identity save/reload and real bandage/medkit spawn,638g total, successful save/quit. This does not establish current layout chunk or visual behavior.
