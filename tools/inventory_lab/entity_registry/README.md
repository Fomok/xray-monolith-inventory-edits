# Stable identity to native object binding

`src/inventory_next/entity_registry.h/.cpp` is a narrow utility for the forthcoming xrGame adapter. It maps a persistent 64-bit core item ID to a currently online 16-bit engine object slot. It contains no game hooks or raw object pointers.

Each handle contains the persistent ID, live slot, registry epoch and binding generation. Unbinding, rebinding the same item or reusing its engine slot cannot revive an old handle. Restoring a save's known IDs clears online bindings and changes epoch. Invalid restores leave the old registry unchanged. Offline IDs remain known without pretending they have live objects.

Adapter lifecycle:

1. Restore the authoritative persistent IDs after the core save validates. Do not serialize these process-local handles.
2. On actual native spawn, bind the object's persistent ID to its current `ID()`. Bindings reject ambiguous ownership or `0xffff`.
3. Capture full handles for queued work; resolve again at execution. A successful lookup is not permission, reachability, or proof of ownership—check those separately.
4. On native destruction/offline transition, unbind the exact handle. On permanent core deletion only, forget the now-offline ID. The model's high-water rule owns permanent ID retirement; the registry must not invent or recycle IDs.
5. During save/load or world reset, restore bindings from authoritative records. A missing online object is not a missing persisted item.

Maximum known IDs matches the core's 4096-node bound. Mutations are single-threaded; the atomic counter only assigns distinct process epochs. Tests include slot reuse, 1000 lifecycle cycles, failed restore rollback, cross-registry handles, destroyed versus offline entries and bounded identity counts.

This utility does not yet add persistent IDs to native server entities, change native event packets, repair ownership or implement game saving. Those remain actual engine integration work.
