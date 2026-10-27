# Read-only inventory panel controller

`src/inventory_next/presentation.h/.cpp` provides native, renderer-independent view state. It reads the same model as the background lab. It does not implement an alternate inventory or move items.

- Ordered panels keyed by stable container ID; opening twice does not duplicate a panel.
- Independent collapse, row scrolling and viewport sizes; partial tiles intersecting the viewport remain available for renderer clipping.
- Grid projections omit equipped items. Equipment arrows select a stable member ID within the current actor's group; removing that member chooses the first remaining ID.
- An explicit observation set authorizes world containers **and individual visible children**. Permission on an outer container does not reveal hidden children or nested bags. The game adapter must supply discovery, distance, reveal and access checks; this controller does not perform them.
- Stale model revisions yield no panel/item projection until synchronized. Session changes clear panel and selection bindings. Losing permission or dropping an unobserved bag closes its panel.
- UI interactions cannot mutate ownership, quantities, mass or saved model bytes. View state is currently session-local, not persisted as player settings.

Call `synchronize(model, actor_id, visible_world_objects)` after each authoritative model/access/reveal update and before rendering. Access changes can occur without an inventory revision. Callers must deliver them explicitly. `selected()` is a cached view selection, not authority for performing an item action; revalidate any action through the model and access gate. Never use this display-only permission set to bypass game admission rules.

Run `python tools/inventory_lab/presentation/validate.py --compiler <g++.exe> --output <isolated-directory>` for debug and optimized builds. Both bounded child processes run hidden at idle priority. The seven scenario groups check disclosure, stale state, lifecycle, viewport intersections, selection fallback and unchanged inventory bytes.

This is not an in-game UI or a claim of gameplay parity. Native widgets, input routing, rendering clips, nearby discovery, action admission and save adapters remain separate integration work.
