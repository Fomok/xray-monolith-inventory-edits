# Field Kit header and container control acceptance

28 October 2026. Isolated Dev32 visual increment; full mod parity remains incomplete.

Engine06db74027deaeeb72e67155bd5f1138fbe7e0228, CI40626557352 successful, artifact12789794925. EXE26942464bytes, SHA256 addaaaad051950a032c8ae615a61ea3da6a77226dfc12160a42730be059a6e10. Final overlay3aebd12abd0a7b9ffa386d01c620575aba7793a6. The latter adds vanilla actor-faction texture aliases and a short reload fixture; no new engine compile required.

Final package: `C:/GAMMA/sqa-previews/inventory-next-20261028/Squared Away 3 Field Kit Dev 32.zip`,15401248bytes, SHA256 d8243fffe60bca939e5783f3cd944e9b8712f67aab571acb61ae7eec15798a9e.26 payloads verified before and after isolated lab installation. The earlier same-name intermediate package769cf6c4... is superseded.

## Implemented behavior

- UICharacterInfo optional Field Kit controls show real name, faction, rank icon, portrait and native actor money. Portrait/icon aspect ratios fit their authored envelopes. Updates run in Update, not Draw.
- Non-character sources show actual source name/kind and count/mass; no invented character faction/rank/money. Eligible corpse partners use native character information. This test covers one crate source only.
- Separate bag windows always show their contents, omit collapse controls and use a four-state textured X. Integrated sections retain collapse/expand. Closing changes presentation only.
- v2 artwork is integrated exactly from the Mod/UI handoff. Sota rank DDS/descriptor are unmodified with recorded provenance; no Sota scripts copied. Ten actor-prefixed faction aliases reuse exact vanilla ui_pda2_noice.xml UVs. The tested actor reports actor_stalker; other factions remain untested, and communities without a matching descriptor remain hidden rather than fabricated.
- No third native pistol slot; no changes to public or personal FG engines.

## Native evidence

Evidence root: `C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40626557352/`.

1. PID16992 headers: actor and stash panels rendered; separate bag had no collapse arrow and a textured X. User initially inspected; input paused, then resumed only after direct answer "Continue automated testing". Automated bag open, X close, integrated collapse/expand and reopen observed. Contents remained visible and unchanged. Save sqa3_headers, exit0, no timeout.
2. First capture exposed missing faction emblem. Native actor community uses actor_stalker while vanilla texture is stalker_icon. Overlay3aebd12a supplies aliases; no gameplay state mutation.
3. PID17420 headers-reload: final overlay loaded the saved state, actor_stalker logged, Loner emblem visible. Same stash336 and bread190g still present. Save and exit0/no timeout.
4. PID8008 headers-proof: additional short visual capture with final overlay and bag open; save and exit0/no timeout. UI snapshots showed unobstructed controls after clearing hover; automatic saved captures still include a reappearing tooltip, so do not describe those JPEGs as a polished unobstructed presentation.

Strict snapshot verifier passes all three runs:7actor items3198g, same identities, native sections, conditions, ownership and placement as Dev31. Actor export SHA a70a00cc6a639d4340c07075d12d7db122c3cc4a1bec5bbaa086b926e3c05b00; world export SHA d424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee. World export contains registered loaded subtrees, not the ordinary crate bread; its preservation is separately observed by source336/count1/mass190 and captures, not claimed from the empty world export.

Reports: headers-verification.json; headers, headers-reload and headers-proof run JSON/log/inv/world/JPEGs; final-installed-save-audit.json; final-process-event-audit.json. No FATAL/unsaveable warnings, no new Anomaly Event1000, zero games at final check. Original test.dds/scoc/scop unchanged; pristine baseline clean. Historical render/Lua crash causality remains a supported hypothesis, not proven by these short runs.

## Limits and handoff

No certification of corpse/ground headers, mixed-source switching, other factions, every screen scale, long sessions or all77 requirements. User visual acceptance remains distinct from observed implementation. Next gameplay checks are the preserved stash equipped/refusal/travel cases after visual acceptance; later phases await review of [DEVELOPMENT-PLAN.md](DEVELOPMENT-PLAN.md).

Cleanup removed only exact-hash Dev31 preview ZIP and cached b983a6df EXE,41548686bytes, recorded in superseded-cleanup.json. Dev28 rollback and historical crash evidence retained; installed MO2 mod directories untouched. Read-only inputs: isolated lab logs/saves/configuration and vanilla extracted configuration references, released Squared Away artwork, and Sota rank artwork/descriptor/script reference. All runtime installation changes stayed inside the authorized isolated lab.
