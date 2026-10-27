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

## Native crate transaction candidate

The foundation engine `eba95c79833053bf7e6f6158883da66ce0d46fc9` compiled
successfully in CI40508770954. It was not installed over the verified Dev18 lab.

A subsequent opt-in `-sqa3-world-storage` candidate adds actor/crate transfers.
`LoadedStorageSession` stages actor and world candidates together, preflights the
complete loaded footprint on acquisition, and only exposes both results after
full native acknowledgement.121 debug/release checks and all three independent
restart forms pass. Ground custody remains deliberately unsupported.

The native candidate uses one server event to validate crate access, persistent
custody and all live facts, silently detach/attach every member, and roll back
before broadcasting if any member fails. Publication reserves incoming objects
or excludes departing objects until all client parents match. Saves remain gated.
Crate lists hide logical descendants while their weight still counts once; ordinary
single-item release cannot bypass custody. Actual native compilation, physical
release/reacquisition, partial-failure and fresh game reload tests are pending.


## Nested native fixture prepared - 27 October 2026

Candidate engine4fdfb9936b3a11d960b6ecf75dd3201f25c070a6 is building in CI40511593122. Dev18 engine0be12d07 with temporary fixture overlay21d0e23e prepared disposable save `sqa3_world_prepared` in PID17028 (exit0): outer bag105 contains bag106 at4,4, which contains medkit102 at1,1. All other original records remain exact;7items3278g. Export SHA25647d58479817ca0d3064c5e6774a768ae7735056311f64d97f80591a7491ffd0f. Evidence: ci-40502480472/world-prepared.log/.inv/-verification.json. This verifies preparation only; Dev18 has no world ledger and the report explicitly records world_persistence_observed=false. Original Dev18 rollback ZIP is unchanged.
