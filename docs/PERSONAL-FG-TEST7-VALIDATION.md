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

Validation completed for source ecf540856233427523433ffe7065ff291a80805b:

- Full engine build [39135180483](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39135180483) succeeded; artifact 12139524764, digest sha256:4b8f8d14373f8db669d0a64a5baa5aabe50f40dad1471a8329ca15aa01a91157.
- Four CPU regressions passed, including invalid/sentinel diagnostic counts,
  off/on and known/unknown scope classification. GPU depth/motion/UI/optical-map
  tests and the dual-FPS telemetry self-test exited 0.
- Existing LuaJIT settings checks passed availability, insertion, pending
  Apply/Cancel, capability status and absence of renderer-restart flags.
  UI/shader/telemetry/companion source is unchanged from Test 6.
- Isolated RTX 5080 + matched OptiScaler DLSS-NR scope integration completed
  1800 frames, with 1348 reporting two presentations, 452 reporting one,
  3148 SDK presentations total, and exit 0. Forty initial frames lacked
  foreground focus. The intentional FG-off interval accounts for 400 frames.
- With a 100-presented-FPS cap: foreground frames200..400 measured49.9994
  rendered/99.9988 DXGI presentations per second; off frames600..800 measured
  99.9914/99.9914; resumed frames1100..1500 measured49.9999/99.9998.
  This verifies the existing cap behavior, not an uncapped speed improvement.
- New context/totals lines were observed from existing SDK queries. This
  includes exact comparison of all four emitted cumulative snapshots
  (tokens240/480/720/960) against the independent full cadence CSV: passed.
  The snapshots do not sample or extrapolate the cumulative counters.
  The standalone probe correctly reports unknown engine context; actual in-game
  scoped-context association remains to be checked. Captured stdout ends
  before the full run summary; the completed CSV, explicit result file and
  exit status establish completion. No shutdown-log completeness claim.
- All 29 CI-manifest entries and six NVIDIA runtime signatures passed.
  The final delivery has a fresh manifest; executable/database/runtime payloads
  remain byte-identical to CI. Source patch applicability is verified separately
  against93483f8a0211bec6583af6179a0038dc6633dfce in the developer handoff.

Existing Test 6 remains available for rollback. Actual Test 7 gameplay, lens
image quality and long-run stability remain unverified. No public branch or
installed files were modified. No AMD FG/buffer-ring change or Preset C driver
override is included.
