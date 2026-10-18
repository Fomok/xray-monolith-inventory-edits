# Personal Test 7: scope-aware evidence

User authorized justified improvements after reviewing Brodrigz release 0.6/tag
1.0.2, pinned bdefda0d7bb5c505dbad0fa15887de9a2eae12dc. This is the same
reference already credited in our integration, not a newer patch. Its scoped
capture logging and cumulative counters motivate this diagnostic improvement.
Credits remain Brodrigz, OGSR team and xrSimpAI; existing licenses are unchanged.

Test 6's game session showed 41 enabled periodic SDK samples with two presented
frames, but its logs did not associate scoped state with each sample. Test 7
adds a current-frame context snapshot and lifetime counters using the existing
SDK state query. No extra SDK queries, resource reads, synchronization, frame
allocations, eligibility decisions or pacing changes are introduced.

`[FG CONTEXT]` shares the token with periodic `[FG DIAG]` lines: known,
engineFrame, scoped, pipOption, scopeInputs, foreground, opticEpoch,
scopeSession, enabled, actualPresented and status. Unknown contexts (including
standalone/loading frames) are explicit and cleared at each BeginFrame.
`scopeInputs=1` requires a successful current scoped input capture, rather than
just an aiming flag. The foreground field reflects engine activity at capture,
not an independent OS focus measurement at scan-out.

`[FG TOTALS]` reports enabled query counts, valid/invalid results, the sum of
actual SDK presentations, queries with more than one presentation, and the
corresponding subset with known current scope inputs. Unwritten SDK sentinel
values and failed status/results cannot inflate presentation totals. Counters
are emitted periodically and at shutdown; they count runtime queries, not
physical display scan-out. Brief scope intervals can contribute even between
periodic log samples. They do not establish visual quality inside the lens.

FG/PiP Video switches, Apply/Cancel/persistence, cap semantics, optical-map
reprojection, scope/UI safety, the OptiScaler NR/menu companion and its 64-byte
dual-FPS ABI are unchanged. Existing Test 6 matching UI/shader files and
companion remain required. No new user option is necessary.

The upstream buffer ring is a possible future performance experiment, not a
proven improvement for this combination. Its AMD FG/FSR4 path is a separate
feature project; its generic main-view scope tagging does not replace our
Bodycam mapping. Its factor-of-two cap scaling differs from our measured
cap behavior and is not adopted. Both ship Streamline 2.14.16 and DLSS-G
310.9.16; Preset C activation is a driver override and remains unverified.

Validation: pending the Test 7 build and runtime checks. Existing Test 6 remains
available for rollback. No public branch or installed files are modified.
