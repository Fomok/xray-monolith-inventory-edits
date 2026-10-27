# Background inventory snapshots

This diagnostic renderer draws a saved **shared C++ inventory model** directly to
a BMP file. It creates a memory GDI surface, not a window. It never captures the
desktop, activates an application, reads input, launches the game, or changes the
model. It uses idle process priority and exits after one image.

This is not Anomaly rendering and not the final SQA3 player interface. It can expose
model placement problems, but does not establish game renderer/input compatibility.
It links the same model implementation; all footprint, placement and weight values
come from that model. The image shows one actor-root grid, one selected container,
and one observed world-root grid. Equipped items are listed without consuming grid
cells. Stack quantities are shown on tiles. Items have diagnostic ID labels rather
than real art/names. Other roots and containers can be selected explicitly.

Build using the isolated compiler already prepared for the lab:

```powershell
./tools/inventory_lab/viewer/build.ps1 -Compiler C:/GAMMA/sqa-previews/inventory-next-20261027/toolchain/w64devkit/bin/g++.exe -OutputDirectory C:/GAMMA/sqa-previews/inventory-next-20261027/viewer-build
```

Run with `Start-Process -WindowStyle Hidden -Wait -PassThru` and arguments:

```text
--snapshot "path/to/model.inv" --output "path/to/new-image.bmp" --container 10
```

Optional `--actor ID` and `--world ID` choose a particular root. Omitting them selects
the first corresponding root. The selected container defaults to the first item
container. Existing image files are refused, not overwritten. Invalid snapshots
fail with a nonzero exit code; an output-adjacent `.error.txt` explains load/render
failures when its directory is writable. Missing/invalid CLI arguments return 2.
The input save must match the current core schema. Error messages do not imply a
successful image. For reproducible checks record input/output hashes and exit code.

There is no interactive drag interface, equipment arrow widget, spatial world query,
texture reload, scrolling or game screenshot capture here. The viewer displays only
objects already in a test model. Full grids are scaled to fit; long item-label lists
are explicitly marked as abbreviated. Native UI work remains a separate milestone.

Current schema4 supports core-authored compartment/blocked-cell shading through
Model::cell_region. The renderer has no separate placement validator. Rebuild after
model schema changes and use freshly generated snapshots.
