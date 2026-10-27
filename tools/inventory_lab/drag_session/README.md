# Shared drag preview and swap proposals

`src/inventory_next/drag_session.h/.cpp` holds a gesture's stable item ID, orientation, model session and revision. It never removes the dragged item, creates an inventory placeholder or commits a drop.

- Valid placement maps to green feedback, invalid placement to red, and a valid one-target swap to orange. Actual colors belong to the renderer.
- Rotation changes the ghost and resulting move together. The footprint comes from the model, including dynamic container size.
- Preview validates the exact move or atomic pair with the shared model. An outgoing item must also fit in the vacated source position; an overlap alone does not make a valid swap.
- Swap uses one supplied boolean for all grids. The frontend should pass the same setting for actor and stash views. Native equipment displacement is deliberately separate from grid swapping.
- Swap targets require an explicit accessible-item set. Displayed/hidden world items must not become interchangeable merely because their rectangles overlap.
- A model mutation or load invalidates the gesture. Cancel and release clear its state. Release returns a proposal; the authoritative adapter must recheck access, locks and handles before committing it.
- Invalid or extreme pointer coordinates are bounded before geometry arithmetic. Multiple overlapping targets do not silently select one.

`validate.py` runs five scenario groups in both debug and optimized native builds, hidden at idle priority. It checks ghost/commit orientation, red/green/orange outcomes, shared swap toggle, inaccessible target refusal, outgoing fit, stale revisions/sessions, atomic paired transfer, cancellation and unchanged model bytes during preview.

Native input wiring, texture ghost rendering, tooltips, per-action access and actual engine ownership transfers are not supplied by this module. A passing preview is not an authorization token.
