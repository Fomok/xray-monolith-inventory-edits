# Loaded corpse storage candidate

Source1b24d8ac4a35cc87d252db50de97d0c33214c80c, isolated CI40537995215.
CI40537995215 succeeded (artifact12750068057). EXE26869248bytes SHA2564ec48ea256f578574ada35118072335f0d9552d08db7681983e9f1347ab12ede. Current Dev24 overlay0761f1a2218409184936e34a78d6356ed7fea26d:13payloads verified and installed only in the isolated lab. ZIP10791312bytes SHA25696e201fa7377eaecf44fab352f6f789e4c8315653ebf993e84116e80ea4a5f7c.

## Native evidence, 2026-10-27

Evidence: C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40537995215/.
- Initial corpse-release PID14224 failed fixture preparation, not engine acceptance: client-only bolt equipment was incorrectly counted as persistent inventory. The correction opens ordinary death loot generation first, then clears/counts ALife-backed items; client-only bolt and bolt_bullet are explicitly logged and retained.
- corpse-release-retry PID33636 passed strict native verification with physical actor-to-corpse drag. Custodian live336/persistent1039 retained all three members (8499,19200,19456),2140g; actor retained four items,1138g. Native direct release of the custodian and every member logged four server refusals. Successful sqa3_corpse_release save, exit0, no timeout. Actor SHA256f80ca0cccd9b61b816c73bd7235a6747a22bbc28b073cb996f264298fd8c1109; world SHA256bf29864ff59446eb243cb28c880ec448638a9b9ec8f7fe2909e71a294f4dfc79.
- corpse-release-reload PID18312 passed exact actor/world comparison in a fresh process, exit0/no timeout.
- corpse-return PID28296 passed physical retrieval: all seven items3278g exactly match initialized baseline, world ledger empty, successful sqa3_corpse_return save, exit0/no timeout. Actor SHA25603ac30edb03ae8fab51bb61be7433b7e22b9e62ccb50f5fd89952cf05092855a; empty world SHA256d424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee.
- corpse-empty-cleanup PID15828 passed native direct ALife release after retrieval; server/client corpse absent, exact actor retained, sqa3_corpse_cleaned saved, exit0/no timeout.
- corpse-empty-cleanup-reload PID34932 passed exact carried state and absent corpse in a fresh process, exit0/no timeout.
- First corpse-offline-cycle PID37684 failed the fixture expectation that switch flags alone offline a dead NPC. CSE_ALifeCreatureAbstract::can_switch_offline explicitly requires health>0. Fixture016b30b9 instead invokes the exposed native switch_offline method without changing health; this is an explicit native lifetime test, not ordinary scheduler eligibility. Production switching rules are unchanged.
- corpse-offline-cycle-retry PID7748 passed explicit native offline/online lifetime, four real offline ALife cleanup refusals, physical retrieval and exact state (only transient ruck slot105 reset13->0). Both offline and return saves succeeded; exit0/no timeout. Final actor SHA68892c85f81c66281035c424e3787232dbc41ca77ee618cb58603147f755c3e3, empty world d424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee. Native IDs stayed stable; this is not forced-ID-remapping or actual travel acceptance.
- corpse-offline-restart PID39908 passed a fresh-process offline save, four native offline cleanup refusals, online transition and physical retrieval. Same final actor/world hashes as PID7748, exit0/no timeout.
- corpse-travel PID904 visually reached Cordon but stayed Paused. Tool Pause and window-close click did not resume/close it; explicitly stopped only verified lab PID, exit-1. No actual-travel acceptance. Buffered log omitted pause diagnostics, so candidate5999c340a2d3c878658c31ca398f920e2248b82a adds immediate FlushLog only for opt-in lab pause/key tracing. CI40545302619 is in progress, not installed. Public/personal branches unchanged.
- corpse-nofit PID2060 and fresh reloadPID22436 PASS: physical refusal with six fillers, actor10/1366g SHA2f3f387c1042f2e789a430fcc7a81503b586bd317d147511bb20db3553ff956d; complete original corpse world unchanged.
- corpse-rollback-return PID29036 and reloadPID9648 PASS: interruption after one actual native parent change restores complete corpse subtree, actor/world byte-identical to release, save_ready=1.
- corpse-rollback-release PID1672 and reloadPID37404 PASS: same partial-failure injection in deposit direction restores exact initialized7/3278g and empty world. All four rollback runs and both refusal runs saved/exit0/no timeout.
- ground-cleanup-regression PID32776 PASS on this engine: physical ground retrieval restores exact initialized7/3278g, removes empty proxy and saves/exits0. New protection does not pin an empty ground container.
- Diagnostic CI40545302619 succeeded/artifact12753842191 for5999c340a2d3c878658c31ca398f920e2248b82a; EXE26869248bytes SHA5792adcf19883157eb552d7995032a1e25bb6cd6d89a0fbe60208293ecd10c1c. Downloaded, not yet installed. Actual area travel remains blocked/unverified.


## Contract

Opt-in -sqa3-corpse-storage also requires logical/world storage. Loaded bag
transfers reuse the complete native subtree transaction and save gate. A corpse
must be dead, have a native inventory owner, allow deadbody taking, remain within
three metres, and not be marked for destruction. Live NPC trade is unchanged.

While a world record exists, native gameplay cleanup is deferred for its
custodian, members and their actual server ancestors. Preflight covers online
GE_DESTROY, direct ALife release, client DestroyObject before its removed latch,
and offline corpse redundancy. It checks the entire ancestor chain before the
server's recursive child loop; refusing only a child could otherwise spin that
loop. Missing identity/ancestry fails closed. Level teardown bypasses protection
when bReady is false so disconnect and travel retain native behavior.

After complete retrieval removes the world record, ordinary cleanup is allowed.
This deliberately retains the corpse while it holds recorded loaded storage;
moving its contents to a separate ground bag on expiry is not implemented.
Arbitrary script ownership changes and all destruction paths are not yet covered.
The candidate is not a complete corpse lifecycle guarantee.

## Required native evidence

- Physical actor-to-corpse drag; exact three-member state and saved world record.
- Online destruction requests for custodian and each member refuse; no partial loss.
- Fresh-process loaded save reload.
- Physical retrieval and exact reload, full-pocket refusal and native partial rollback.
- Real offline/online cycle, including direct offline ALife release attempts.
- Empty corpse cleanup succeeds after retrieval, preserving actor records.
- Existing ground cleanup and normal travel remain functional with the new guard.

Fixture extends sqa3_native_autorun.script with -sqa3-world-corpse and
-sqa3-world-destroy-attempt; only a disposable spawned NPC's generated inventory
is cleared during preparation. Offline cycle uses -sqa3-cycle-corpse and
-sqa3-cycle-destroy-attempt. Empty cleanup has its own explicit fixture.

## Independent pause diagnosis

The Dev23 crate travel run remains unverified after its Paused screen resisted
tool input. This does not block independent corpse implementation. The later
human instruction to continue independent work was verified in Planning.
-sqa3-pause-trace records native pause reasons and received keyboard presses;
it does not force unpause or claim physical keyboard acceptance. The next CI also
retains matching PDBs for three days, downloaded only if crash diagnosis needs
them. Earlier initialization shutdown access violation remains unresolved.

Fixture correction: cleanup requests use alife():release directly. Vanilla _g.script alife_release can return false without native release for a standalone NPC; for squad members it calls squad:remove_npc first. Native veto acceptance must not be mistaken for validation of that Lua bookkeeping path. Squad-managed cleanup compatibility remains open.

Superseded Dev23 Travel Safety and Offline Reload Fixture ZIPs were hash-checked and removed (21573774bytes). Travel Gate Test Dev23 rollback and all evidence retained; cleanup manifest sqa-previews/inventory-next-20261027/dev23-superseded-package-cleanup.json.
