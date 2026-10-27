# Dev21 loaded ground bags — 27 October 2026

The isolated native inventory can drop and retrieve a complete loaded backpack
without recreating its items. Twelve bounded native runs passed. This is one
accepted development slice, not complete SQA3 parity or a public release.

## Build and deployment

- Branch: `inventory-next-harness`; engine `033e851cf204bc3a7ff1e22ee815bfdeacd7c230`.
- GitHub Actions run `40520705952` succeeded; artifact `12743655109`.
- EXE: 26862080 bytes; SHA256 `4c6804a8bb2b9574716aad45acaf377d9e4344b1adb1f8bd3918d06a54140427`.
- Tested overlay: `c8a5dc45b3be0e3cac1cba20d3916a9a3b948e2f`.
- Package: `C:/GAMMA/sqa-previews/inventory-next-20261027/Squared Away 3 Ground Bags Dev 21.zip`, 10784246 bytes; SHA256 `fa8552c0c345289910b4c0362a6dd98426994681e690a91f2c7ee4fca1ad80d5`.
- Installation: `C:/GAMMA/sqa-inventory-lab-game` only. All 13 payloads rehashed successfully after testing. Original `test.dds/scoc/scop` still match the pre-test copies.
- Squared Away Field Kit atlas remains unchanged, SHA256 `cab0598b6a001a5f28f47b7b2d3f8e173181dd4a6ba640ba43cd6c0225ed10de`; no 3D player panel.

## Native acceptance

Evidence root: `C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40520705952`.
Each stem has `.log`, `.inv`, `.world`, `-run.json` and `-verification.json`.
Native screenshots are retained; the first four captures were matched by their
run timestamps, recorded in `installed-save-audit.json`.

| Scenario / evidence stem | Input PID | Fresh reload PID | Verified result |
|---|---:|---:|---|
| `ground-release` | 34880 | 36808 | Physical ground-drop drag moves all three members to dedicated proxy19712, persistent custodian107. Actor4/1138g; world3/2140g. |
| `ground-return` | 34776 | 39504 | Physical retrieval restores exact prepared7/3278g; empty proxy is removed from online and ALife state. Source menu closes safely. |
| `ground-nofit` | 32368 | 18332 | Physical retrieval with all pocket cells filled refuses; actor10/1366g and the complete world subtree stay unchanged. |
| `ground-timeout` | 24208 | 32572 | Forced five-second provisioning timeout keeps exact prepared actor state, cleans unused proxy and allows save. |
| `ground-rollback-release` | 37764 | 15000 | Actual first native move is interrupted before client notification; all three source parents restore, unused proxy is cleaned, save_ready=1. |
| `ground-rollback-return` | 15752 | 12276 | Same partial failure during retrieval restores all three items to original proxy19712; no proxy deletion, exact actor/world records and save_ready=1. |

Every process exited0 without supervisor timeout. The final elevated process
audit found zero Anomaly/xrEngine/VerifiedDX instances. The runner refuses an
existing game, validates installed payloads, bounds each run and closes only its
own matching executable. Screenshot capture was added to the runner and exercised
by the last eight runs; it is supervision/evidence capture, not behavioral proof.

Prepared subtree: outer105 contains inner106 at4,4; inner106 contains medkit102
at1,1. The transfer preserves all three identities, conditions, masses and nested
positions. Exact reload checks compare both actor and world records.

| State | Actor export SHA256 | World export SHA256 |
|---|---|---|
| Released / failed retrieval | `eec27c55302452c1c5b165f22f83c7bb97e325584fb9dff1446852205980b23b` | `611556157f8fd926c094d3fa8c7521a7da76d72985f5192b6f6c3cab992da16b` |
| Returned / timeout / failed release | `9d30bf60382d63e8bf10930f763b7d5c799c15ac4daba4e3464a8542cead958d` | `d424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee` |
| Full-pocket refusal | `f9ee5a1a57fe7963a35b40fb54ea35ea8e4d157556a5373a489d51fbd9548356` | `611556157f8fd926c094d3fa8c7521a7da76d72985f5192b6f6c3cab992da16b` |

## Remaining limits

This dedicated `sqa3_ground_backpack` container preserves the original whole bag.
It does not implement vanilla Create stash, independent editing of ground-bag
contents, loaded corpse custody, multi-bag bulk queues, travel/offline/destruction
recovery or all77 parity requirements. Vanilla Create stash destroys/recreates
the original bag and needs separate integration. Physical R-key and occupied-slot
drag checks remain unverified. Public engines, personal FG and pristine baseline
were not changed. Dev18/Dev20 rollback packages remain available; older engines
must use compatible older disposable saves, not these new ground/world saves.
