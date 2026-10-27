# Shutdown lifetime investigation — 2026-10-27

Dev27 empty-stash PID2640 saved its fixture state but exited with access violation
0xc0000005. It is **not an accepted run**. Windows Event1000 identifies
AnomalyDX11AVX.exe RVA0xc1b5e, report5b46a342-32e0-4343-a3ae-c23696014a85.
Evidence: `C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40552310845/stash-empty-release.*`.

The exact CI40552310845 PDB (artifact12756607525, SHA256
74843c98778225e3c68d783a8018036dc9f2d3dc5630fad35c922dcaf9a484cd) places
AddOne at RVA0xc15b0 and Log at0xc2310. Disassembly at the fault is
`mov rcx,[rbx+8]`; rbx was loaded from RVA0x1839768, independently identified
by that PDB as g_pStringContainer. This is the inlined shared-string docking
path in src/xrCore/log.cpp. Symbol evidence is empty-stash-pdb-symbols.json.
No crash dump was available; a caller stack and register values are unknown.
The earlier shutdown fault at0xc178e is not symbolized against its own build,
so the two failures are not asserted to be identical.

Source confirms a lifetime bug: Startup only calls ClearActivity after the
main loop, leaving discord_core allocated and its DiscordLog -> Msg callback
registered. Core._destroy subsequently destroys the shared-string container.
The SDK wrapper's Core destructor calls internal_->destroy. Commit4e263457
therefore destroys the SDK after clearing presence, before console/logger/
engine teardown, using ordinary delete to match Core::Create's ordinary new.
It nulls the pointer, disables further updates and logs completion while
logging remains valid. This closes a concrete dangling-callback opportunity;
without a dump it remains a candidate explanation of PID2640, not a proven
caller attribution. No stash validation or exit-code requirement is relaxed.

CI40558659909 builds this change on inventory-next-harness only. Runtime
regression and empty-stash acceptance remain pending. The public engine,
personal FG branch and pristine baseline were not modified.


### Targeted shutdown verification and explicit stop — 2026-10-28

Human scope narrowed in Planning to shutdown fix/targeted verification only, then STOP for visuals. Verified no interactive game running before install. CI40558659909 SUCCESS/artifact12760608010; engine4e263457ffcf085a5c0f7616bb69bbb6024e0aba, EXE26886656 SHA8ef21dc8776d86444df176252d448fbfe2594e3d3da4ed9ec86a99602d1e107d. Overlay71e1cbd705f30a5f22c1c2421f0cd6f0b458f638; Dev28 ZIP10810199 SHA2b56d0e1fcfc020387068c36c8e409b236f8a6c7786a4b67c531c48ff404e524,16payloads verified before/after isolated install/testing. PID31064 shutdown-empty-release: physical normal dialog OK, exact same actor/world as failed PID2640, native fixture/strict verifier PASS, save+exit0/no timeout. PID19084 shutdown-empty-reload: exact reload/no callback replay, save+exit0/no timeout PASS. Actorf80ca0cccd9b61b816c73bd7235a6747a22bbc28b073cb996f264298fd8c1109; world3b20e9f83a91293550c03bb5bb9c5e1cb0cbdf78903a3b6c6aef85fd1a899229. Both log SDK destruction before renderer teardown. final-process-event-audit.json:0games/0newAnomalyEvent1000 since first run. final-installed-save-audit.json verifies16payloads and original test.dds/scoc/scop unchanged; pristinebaselineclean c57fe5b9. This is bounded regression evidence, not proof of the historical caller or universal shutdown stability. Evidence ci-40558659909/. Superseded Dev27 ZIP10811120bytes removed with exact path/hash checks; dev27-package-cleanup.json. Keep Dev25 rollback and Dev27 EXE/PDB crash evidence. No public engine changes, no broader stash cases run. Work stopped per user; visuals next.
