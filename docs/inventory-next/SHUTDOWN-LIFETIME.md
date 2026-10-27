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
