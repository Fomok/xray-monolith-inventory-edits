# SQA3 Dev31 visual correction — additive asset v2

28 October 2026. **ART ASSETS ONLY. Ready for Engine integration; not installed or tested in game.** v1 files/IDs remain byte-identical. Native controls, data binding, source semantics and removal of collapse controls from separate windows belong to Engine.

## Two payload files

Use the files under this folder's `package/gamedata/`, preserving paths:

| File | SHA256 |
| --- | --- |
| `configs/ui/textures_descr/sqa3_visual_assets_v2.xml` | `b68f84818a476423b7a0cd24c1f8cf648bfbaa71337b00834c0be3a35bd2c51f` |
| `textures/ui/sqa3_visual_info_v2.dds` | `e02596c54c17382819bc7a91ef8ea45a36891b113f10eaafb6dc5e45bd4bfb72` |

The atlas is1024×1024, uncompressed RGBA, no mipmaps, with two-pixel edge extrusion. Texture IDs and exact crop rectangles are in the XML/`asset-manifest.json`. Runtime artwork contains **no baked name, faction, rank, emblem, portrait or money**. Contact-sheet captions/zone labels are review annotations only.

## Early/final texture contract

Two raster pixels per logical UI unit, matching v1. These dimensions are based on released XML and atlas regions, not screenshot dimensions.

| ID | UI size | Use |
| --- | --- | --- |
| `sqa3v2_info_panel` | 362×94 | Complete olive identity + money/info bar + portrait recess composition, at column-local10,10. |
| `sqa3v2_identity_region` | 264×62 | Component alternative: framed olive name/faction/rank/emblem region. |
| `sqa3v2_money_bar` | 264×28 | Component alternative: thin dark metallic money/source-info bar. |
| `sqa3v2_portrait_recess` | 92×88 | Dark portrait backing with thin warm rim. |
| `sqa3v2_portrait_frame` | 92×88 | Same outer rim with transparent image aperture; draw AFTER portrait if needed. |
| `sqa3v2_close_e` | 20×20 | Enabled textured X button. |
| `sqa3v2_close_h` | 20×20 | Hover. |
| `sqa3v2_close_t` | 20×20 | Pressed, darkened with shifted X. |
| `sqa3v2_close_d` | 20×20 | Disabled. |

Use **either** the complete composite **or** the three component backgrounds, not both. Transparent gaps in the composite let the existing dark v1 panel show through. No new whole-column background is required. Existing v1 grids, quickslot frames, arrows, panels and section bars remain unchanged.

## Placement and live controls

`layout-contract.json` records every rectangle. Group origin is column-local(10,10), size362×94; bottom104, leaving space before existing storage starts. Apply the same artwork to inventory and eligible loot columns. Dev31's current actor/partner identity controls are not arranged for this region: Engine must reposition them, not simply put the new art behind the old123×87 portrait at local197,1. See current `tools/inventory_lab/gamedata/configs/ui/sqa3_field_kit.xml`:25–51.

Relative to the new group:

- Olive identity:0,0,264,62. Name:8,14,158,18. Faction/source label:8,34,158,18.
- Rank artwork slot:176,6,38,50; emblem slot:216,6,42,50. These are display envelopes, not icon distortion instructions. Center real existing icons, preserve aspect ratio, and hide absent metadata. Do not invent a new emblem/rank or copy actor data into stash fields.
- Dark info bar:0,66,264,28. Money/info text:8,70,246,20, right aligned. Engine provides actual value/currency or genuine source information.
- Portrait frame:268,6,92,88. Portrait aperture:270,8,88,84. Draw image within the aperture, then transparent frame if necessary. Preserve source image aspect ratio with aspect-fit; do not stretch a123×87 old portrait control into the new frame.
- Rank and faction share the old public-style olive area, but are distinct live controls. Long text must be clipped/ellipsized within the text envelope without crossing rank/emblem slots. Header art does not define gameplay metadata.

For ground/stash sources use the same visual family, but show only real source data. It is valid to leave the portrait/rank/emblem/money areas blank or hide irrelevant components; **do not fill them with the player's identity or5000 currency merely because the reference shows those values**. Corpse/trader metadata availability is an Engine decision, not an asset fallback. Conditional column visibility remains unchanged.

Minimal static background example, to be incorporated by Engine in the correct owner/view:

```xml
<auto_static x="10" y="10" width="362" height="94" stretch="1">
  <texture>sqa3v2_info_panel</texture>
</auto_static>
```

The snippet is column-relative, not a drop-in global actor-menu XML replacement. Confirm the descriptor is loaded and the background's texture is enabled; Dev31 previously needed explicit `TextureOn` after initialization for header backgrounds.

## Separate container window close control

Use the four `sqa3v2_close_*` IDs on the actual stateful button, with empty text. Place20×20 at(window width−24,4), so the visible/hit rectangle has4-unit top/right insets. Reserve at least24 units from the title's right edge, and exclude this hit target from title dragging. Replace the blank `sqa_fk_close` visual / square glyph; do not layer a text X on top of this asset.

The existing source at `src/xrGame/ui/UIActorMenuFieldKit.inl`:38–39 uses a disabled static close visual at264,4. Engine owns replacing that visual/control and wiring states/close behavior. **No collapse control belongs inside a separately opened carried-container window.** This asset pack includes no collapse arrow and does not change the main inventory's valid collapse controls. Closing must retain ownership/content/placement and must not trigger drag or transfer.

## Provenance

Source: `C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source/gamedata/`. `asset-manifest.json` pins source hashes and crop operations; `build_assets.py` reproduces the outputs deterministically.

- `textures/ui/sqa_fk_panels.dds`, XYWH(740,30,704,124): existing complete framed olive identity region, resized to528×124 raster pixels.
- Same atlas, XYWH(740,160,704,56): existing complete framed dark money bar, resized to528×56.
- Same atlas, XYWH(1470,50,224,150): clean dark portrait inset, resized to184×176, with authored2-raster-pixel warm rim RGBA(122,117,65,255). Transparent-frame aperture is x4..179,y4..171 inclusive. This avoids duplicating the old outer frame after resizing.
- `textures/ui/sqa_fk_controls.dds`, XYWH(600,160,48,48): released close-button base, resized40×40; antialiased crossed line artwork, shadow, brightness/state treatments added deterministically. X is raster geometry, not a font symbol.
- Released `configs/ui/ui_inventory_16.xml`:12–39 supplies public header/control context; source player width382, portrait89.5×84, rank/emblem region and money below identity. Current v2 envelope leaves10 units on either side of382-unit column. Minor component spacing changes accommodate the unified2× raster contract.

User reference inspected: `C:/Users/fomok/AppData/Local/Temp/codex-clipboard-b4a63c22-54f5-4546-9ca8-4b847100ce9f.png`. It guides appearance only and supplies no pixels. `source-top-reference.png` is an inspection crop of the released atlas, not a runtime payload.

## Validation and remaining gates

`verify_assets.py` / `validation.json`: PASS for9 unique in-bounds nonoverlapping IDs, exact DDS/PNG pixels, four distinct close states, transparent portrait aperture, expected composite dimensions, output hashes, no v1 ID collisions and unchanged v1 payload hashes. `contact-sheet.png` visually reviewed. Dev31's bounded crash/save/reload evidence was read as context only; no new native acceptance is implied.

Only this isolated v2 asset directory and appended paired feature/work-log notes changed. No C++, Engine overlay, installed game, website or public release edited; no game launched and no installed game files read. Remaining Engine gates: descriptor loading and draw order, truthful actor/source data, portrait aspect fit, text clipping, button state/input behavior, removal of separate-window collapse controls, final in-game screenshot acceptance.
