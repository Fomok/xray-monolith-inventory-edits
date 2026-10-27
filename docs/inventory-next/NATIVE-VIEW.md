# Explicit native read-only inventory view

Implemented in `src/xrGame/ui/InventoryNextView.h/.cpp`, using existing CUIWindow,
CUIStatic, CUIButton and CUIScrollView. No browser, alternate packing algorithm,
actor-menu replacement, script hook or automatic startup registration is added.
This is a real native control implementation awaiting xrGame compilation and
isolated runtime verification, not a claim that a playable GUI already runs.

## Binding and ownership

`Bind(const Model&, const Presentation&, Id actor, const Visuals&)` and `Refresh`
copy the supplied model into `unique_ptr<const Model>` and copy presentation and
visual metadata. No engine entity, Node, ItemView or caller-owned Model pointer is
retained. Source objects may be destroyed after the call returns. Methods must run
on the UI thread; the caller must not mutate the source while it is being copied.

Bind validates the supplied actor root through `Presentation::open` on the copied
state. A session/revision/actor mismatch fails closed via Unbind. The actor storage
panel is opened explicitly; other supplied panels keep their state. All grid items
come from `Presentation::visible_items`, preserving its world access restrictions.
Equipment rows come from actual snapshot members directly equipped on that actor;
selection/cycling uses Presentation. No ownership command is dispatched.

Children use engine auto-delete ownership. The only control pointers retained are
the owned scroll view and a button/action map. Invalidate immediately hides/disables
the old subtree and clears actions. Rebuilding/deleting children occurs at the next
Update, never inside the sender button's click stack. Unbind discards model and
metadata immediately. Destruction relies on normal CUIWindow child ownership.
Invalidation releases topmost mouse captures belonging to this subtree, canceling
active scrollbar gestures without releasing another dialog's capture.

The view cannot detect changes in an external object it intentionally does not
retain. The owner MUST invalidate on model mutation/load/reset and access/visibility
changes, then Refresh with a newly synchronized Presentation. Session/revision
accessors identify the displayed snapshot but are not subscriptions. Every Refresh
rebuilds even at the same revision, because access permissions and labels may change.
If the owner cannot supply an authorized snapshot, call Unbind.

## Integration for Engines

Only the two new UI files and this note were added by this task. Engine chat owns
project files, probe/bootstrap, input routing and UI lifetime. Add these entries to
`src/xrGame/xrGame.vcxproj` and matching UI filters:

```xml
<ClCompile Include="ui\InventoryNextView.cpp" />
<ClInclude Include="ui\InventoryNextView.h" />
```

The implementation starts with `stdafx.h` and uses the project's existing PCH.
The current project declares C++17. Ensure `src/inventory_next/model.cpp` and
`presentation.cpp` are linked into xrGame once; shared sources should use the
engine chat's existing per-file PCH settings (normally NotUsing). Their include
from this header is `../../inventory_next/presentation.h`, relative to xrGame/ui.
Do not compile a second copy if they are already supplied by a linked native target.

Illustrative owner-side calls, inside an explicit isolated read-only probe:

```cpp
auto view = xr_new<CInventoryNextView>();
view->SetAutoDelete(true);
owner->AttachChild(view);
view->Initialize(Fvector2().set(0, 0), Fvector2().set(700, 600));
presentation.synchronize(model, actorId, visibleWorldIds);
view->Bind(model, presentation, actorId, visuals);

// Before replacing or exposing a changed model/access snapshot:
presentation = view->PresentationState(); // preserve local scroll/collapse/selection
view->Invalidate();
presentation.synchronize(model, actorId, newlyVisibleWorldIds);
view->Refresh(model, presentation, actorId, refreshedVisuals);

// On owner shutdown or loss of permission, before normal parent destruction:
view->Unbind();
```

Do not synchronize the local state with guessed world permissions. The observer
must supply explicit authorized IDs. A top-level corpse permission does not imply
permission for all nested items. Initialize before binding is recommended. Resize
through Initialize again so the native scroll control updates its geometry.

## Rendering and controls

- Native outer scrollbar handles the list of panels. Per-panel up/down buttons
  call Presentation row scrolling; collapse toggles Presentation state.
- Equipment previous/next buttons change selected member only. The `[+]` button
  on a visible container/equipped container opens its authorized panel.
- Item positions and rotated footprints come directly from ItemView; only display
  cell scale changes to fit panel width. No repacking or movement is performed.
- Item/viewport scissor windows use the same PushScissor/PopScissor contract as
  native CUIScrollView. Partially visible tall items are clipped at row boundaries.
- Input clipping is separate from drawing: clip windows and the concrete buttons
  reject mouse events outside the intersection of their own and all ancestor
  rectangles, using half-open right/bottom bounds. Leaf checks also cover capture
  routing that bypasses parents. Rejected events cancel only the local subtree's
  captures, hover and pressed-button state. Button Update clears clipped hover and
  pressed state after scrolling even without cursor motion; no click is synthesized.
- The fallback draws text, usable-cell dots and blocked-cell crosses. It is not
  final visual styling; adjacent region borders are not yet drawn distinctly.
- Visual metadata is keyed by real model IDs. Label precedence is supplied label,
  supplied section, nonempty `attributes["native_section"]`, nonempty
  `attributes["section"]`, then the actual numeric ID. `native_section` is the exact
  key written by InventoryNextProbe; explicit Visuals always keep precedence.
  No section/type is guessed from names or created for display.
- An optional `texture_id` must be an already validated registered native UI
  texture descriptor. CUIStatic::InitTexture/SetStretchTexture are established
  APIs. Empty references use text. Rotated items currently use text fallback;
  raw atlas conversion, visual rotation and safe missing-texture validation are
  adapter work. Do not pass arbitrary/unverified filenames as texture IDs.

## Evidence and validation boundary

API inspection: UIWindow child ownership/DetachAll and message routing;
UIScrollView InitScrollView/AddWindow/Clear/GetDesiredChildWidth/SetScrollPos and
Draw scissor; CUIButton BUTTON_CLICKED target delivery; CUIStatic TextItemControl,
InitTexture and stretch; CUILines font/ellipsis/immediate rendering; shared
Presentation access checks, row scrolling, group cycling and visible_items;
Model::snapshot already returns rotated footprints.

Static checks validate local include paths, UI API declarations, explicit const
snapshot ownership, and absence of model execute/move/spawn or engine ownership
mutation calls in the adapter. Source whitespace is checked separately. These are
source checks, not a substitute for C++ compilation. No MSVC build, native UI test
or game launch was performed by this task. The prior portable model contract does
not compile these xrGame UI classes and is not evidence of their renderer behavior.

Nine static contract checks passed: shared include resolution, immutable snapshot
ownership, no model mutation calls, no retained Node/ItemView pointers, controller
visibility lookup, deferred rebuild, and button/scroll/ellipsis API declarations.
The include correction is `../../inventory_next/presentation.h` (two parent steps,
not three). The inspected shared presentation header SHA-256 is
`4c4f7dc3a1b62238fd0bec1f4d883e7af1e313c66d2d92f482403424e60abc96`.

Engine validation gates: compile xrGame with its normal SDK/PCH/dependencies; bind
an actual isolated probe; check clicks, outer scroll and row scroll; clip a rotated
or partly hidden item; collapse/reopen; change equipment selection; revoke world
access at unchanged revision; replace session; destroy source after Bind; Unbind
during pending UI input; resize; and destroy the parent under a visible scrollbar.
Assert the model serialization/revision is unchanged by all view input.

The follow-up clipping/label fix was checked against native OnMouseAction (hovered
children can receive out-of-bounds events), Update (leaf-only hover calculation),
ReleaseMouseCapture (subtree removal), CUIButton state/reset behavior and the probe's
native_section assignment. Static inspection confirms both clip and leaf-button
guards, pressed-state cancellation, per-update stale-hover cancellation and label
fallback ordering. Native acceptance cases still required: click the hidden portion
of a partly clipped [+]; press inside then release outside; scroll a pressed button
out of view without moving the cursor; reenter and release without a new press;
verify outer-scroll clipping too; and confirm another dialog's capture survives.

## Remaining behavior

The view is read-only: no drag/drop, equip/unequip, context actions, keyboard focus
navigation, tooltip inspection, resizing handles or mutation preview is wired.
Mounted-pouch children are not automatically listed because visible_items excludes
Mounted location; the owner can explicitly open their authorized panels in supplied
Presentation. No world discovery, item-icon resolver, final localized labels or
native game observer is implemented here. Performance of many large open panels
requires native profiling; controls rebuild on explicit invalidation, not each frame.
The next integration step is the engine-owned actor probe binding and MSVC build,
followed by native read-only input/lifetime testing before mutation controls.


## Engine integration increment

Linked in xrGame.vcxproj. Explicit `inventory_next_probe view` under -inventory-lab -dbg builds the actor snapshot with saved core identities and opens a native dialog. Existing dialogs must first be closed. Escape closes it; execute again to refresh. CUIGameCustom owns/deletes it at unload/destruction and hides it on native inventory actions. It is explicitly a frozen snapshot, not a synchronized live ownership view. Probe layout is16columns and64rows, restricted to widths<=16; oversize input fails clearly instead of truncating. No game launch or native input test.

All engine-facing CInventoryNextView callbacks now have noexcept exception containment; failure clears controls/actions/snapshot and releases its input capture without changing the authoritative model. This source alone opts into the same narrow exception boundary as the other SQA3 adapters. Private render helpers may throw only into those boundaries. Pending native compile determines API/link correctness, not runtime lifetime safety.
