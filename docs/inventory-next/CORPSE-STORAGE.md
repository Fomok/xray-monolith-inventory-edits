# Loaded corpse storage candidate

Source1b24d8ac4a35cc87d252db50de97d0c33214c80c, isolated CI40537995215.
Compilation and runtime acceptance are pending. Existing Dev23 remains installed.

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
