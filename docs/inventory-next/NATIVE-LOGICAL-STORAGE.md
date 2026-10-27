# Native logical storage adapter

The opt-in `ActorStorage::logical` mode extends `ActorLayout` to retain an
actor-owned logical forest while native objects remain physically actor-owned.
The normal native caller still uses the Dev10 flat layout. This change is not
native pickup enforcement or a finished storage UI.

## Reconciliation contract

The caller supplies current native facts, each with physical actor parent 1.
Container definitions must be supplied explicitly from validated configuration;
the adapter never restores saved mass, condition or container capacity over live
facts. It restores only logical parent and placement. Equipment follows native
equipment state, including taking an item out of its logical bag when equipped.

Contents are restored deepest-first, before their outer bag's placement is
validated. This lets soft containers acquire their loaded footprint first.
Unequipping a loaded bag retains its descendants. A forced arrival or unequip
without a valid saved grid position remains in explicit actor recovery. Logical
mode never silently packs recovery into pockets. Missing or resized storage
leaves surviving actor-owned objects in recovery without losing their identity
or counting their weight twice. A surviving nested container retains its own
contents even when its outer storage is lost.

The persisted actor attribute `native_logical_storage=1` identifies this mode.
A flat caller rejects a restored logical forest instead of silently flattening
it. Native equipment overrides old logical membership; failed identity checks,
cycles and foreign destinations do not publish a partially changed model.

## Required before native activation

- Native ordinary pickup/loot/trade/drop must use the shared reservation and
  actual ownership acknowledgement boundary before any source detach. The
  recovery path is for forced reconciliation, never a full-grid pickup bypass.
- Dropping a loaded bag is a subtree transfer, not the missing-storage recovery
  case. Preserve the real descendants, their logical links and state. A native
  sync that observes only actor items cannot implement world subtree persistence.
- Storage and recovery panels must use logical parents. The existing single
  actor bag list cannot display overlapping coordinates from different parents.
- Reconcile native facts before action validation. Carried possession, manual
  actions/crafting and configured combat quick access need distinct queries.
  UI panel visibility is not an ownership or crafting permission. Dropped bags
  cannot enter ordinary actor crafting/use searches. See
  [the script compatibility audit](NESTED-ACTION-COMPATIBILITY.md).
- Pending descendant actions must also protect ancestors against drop; cached
  identities, state and quantities need revalidation at mutation. Native weapon
  and external script paths remain unverified for logical mode.

## Validation on 2026-10-27

`native-logical-storage-build` under the isolated preview root passes 90
behavioral tests in debug and release, plus the existing independent-process
reload and a new independent-process native logical forest reload in each build.
Tests cover bag unequip/re-equip, native mass/condition updates, nested soft bag
ordering, forced equipment, missing/shrunken storage recovery, failed cycle and
foreign moves, and refusing flat-mode reload of a logical save.

These are core adapter tests. No new game launch, native mode activation,
ordinary pickup enforcement or gameplay compatibility is claimed.
