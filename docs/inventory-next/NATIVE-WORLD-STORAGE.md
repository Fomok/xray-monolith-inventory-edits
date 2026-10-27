# Loaded world storage: dependency milestone

The installed Dev18 build still safely refuses loaded-container release. This
milestone does not enable loaded-bag transfers or certify their gameplay behavior.

## Why this comes first

`ActorLayout` only reconciles actor-owned native facts and rejects world-owned
records on restore. Releasing a loaded bag through the ordinary single-item event
would leave its contents with the actor or lose their logical parentage during the
next reconciliation. The existing transfer coordinator checked descendant handles,
but accepted only the shell's parent acknowledgement. That cannot prove a complete
multi-object ownership transfer.

## Implemented foundation

- `TransferSession::Dispatch::subtree` describes every item and its expected native
  source/destination. Explicit flat custody can represent an actor or world
  inventory holding all objects while the logical bag tree remains nested.
- A scalar acknowledgement cannot commit a loaded subtree. Complete observations
  must match every identity and expected parent. Partial, duplicate, stale or
  mismatched observations require reconciliation and continue blocking saves.
  A rejection unlocks only after every item is confirmed back at its source.
- `WorldSubtrees` stores detached bags separately from the actor projection. Each
  record names a persistent custodian (zero for ground), preserves internal
  placement/state/identities, and validates unique IDs across records. The outer
  bag's world staging position is normalized; it is not an actor-grid reservation.
- Bounded, canonical, checksummed persistence restores transactionally. Native
  refresh requires complete matching identities/sections and uses current native
  mass/condition/state, retaining only saved logical parentage and placement.
- Development engine save/reset hooks use chunk `0x33515753`. Older development
  saves without the chunk start with an empty ledger. This integration still needs
  the isolated Windows engine build and runtime regression tests.

## Verified offline

117 behavioral tests pass in debug and release, including complete nested
actor/world roundtrips, partial outcomes, rejection, handle reuse, corrupt saves,
duplicate identities, wrong custodians, current-fact refresh and invalid resize.
Independent processes reload the ordinary model, actor forest and detached world
ledger. Evidence: `native-subtree-ack-build` under the dated preview workspace.
Unchanged core objects were reused from `native-ground-pickup-build`; the changed
transfer unit and test translation unit were rebuilt with warnings as errors.

## Remaining native dependency

The engine must stage a complete native transaction, validate physical custody
and identity for every descendant, update all server parents with rollback before
client notification, then wait for every client acknowledgement. Actor layout and
world ledger publication must share one save gate. Acquisition must refresh native
facts and preflight the complete loaded footprint before removing a world record.
Ground custody needs a safe native representation for hidden descendants; dropping
each child as an unrelated ground item is not an implementation of a loaded bag.
Offline/remap/destruction and container disappearance need explicit reconciliation.

No world-transfer UI success may be shown until those steps have native evidence.
Keep Dev18 rollback, the production installation and public release unchanged.
