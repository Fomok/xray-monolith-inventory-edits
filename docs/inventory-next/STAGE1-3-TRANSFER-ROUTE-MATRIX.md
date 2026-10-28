# SQA3 Stage 1–3 direct-transfer matrix

28 October 2026. This is the current implementation and acceptance boundary for the isolated vanilla lab. A direct move must not require an empty actor-pocket cell. The native physical owner may remain the actor for nested actor storage, or the stash for detached storage, while the logical owner records the exact container. A rejection must leave native and logical source identities unchanged.

| Source → destination | Shared route | Current evidence |
| --- | --- | --- |
| Loose stash → actor backpack, rig, pouch, carried box, actor pockets | `SqaTakeLoot` → `InventoryNextLootTake` → `GE_SQA_ACQUIRE` → `GroundPickup` exact position | New exact-cell transaction passes core test; CI/game drag and fresh reload pending. |
| Loose stash → vanilla/custom equipment | `ToSlot` or custom drop → `InventoryNextLootTake`/`InventoryNextLootCustomEquip` | Prior native custom equipment and direct equip fixtures passed. Recheck after follow-up engine. |
| Actor pockets/backpack/rig/pouch/carried box → loose stash | `SqaPutLoot` → native owned release or loaded `InventoryNextWorldRequest` | Earlier loaded bag/crate and source custody fixtures passed; each nested leaf route not individually exercised. |
| Actor storage/box ↔ other actor storage/box | `InventoryNextNativeDrop` with view owner and cell | Core cross-window preview/commit and prior native nested crate transfers passed; expanded pairs not individually exercised. |
| Actor box item → equipment | Native `ToSlot`/custom equip after logical access | Native direct equipment paths passed for representative items; box ancestry with full pockets needs a targeted run. |
| Box already in stash → actor storage/equipment | Virtual box-content source, `WorldSubtrees.without_leaf`, then native pickup acknowledgment | Core candidate test passes; CI/native build and physical drag/reload pending. Not accepted yet. |
| Actor storage/equipment/box → box already in stash | `e8a031a3` adds a detached box destination and `GE_SQA_WORLD_MEMBER` with preflighted exact cell, native move and post-ack world ledger publication | Core model checks pass; CI/native build and physical drag/reload pending. Not accepted yet. |
| Loose stash → box in stash; box in stash → loose stash or another box in stash | Same detached box destination/source and `GE_SQA_WORLD_MEMBER`; same physical custodian changes only the logical ledger | Core model checks pass; CI/native build and physical drag/reload pending. Not accepted yet. |

The model permits a rig inside a backpack when `allow_rig_items = true`; other portable containers remain blocked by default. Rig exterior minimum is 2W×3H; backpack exterior minimum is 2W×2H. Current world-member routes accept leaf items only; loaded box subtrees still require the whole-container route. No row implies physical mouse acceptance unless evidence explicitly says so. User-facing package status remains in `PHASE1-3-STATUS.md`.
