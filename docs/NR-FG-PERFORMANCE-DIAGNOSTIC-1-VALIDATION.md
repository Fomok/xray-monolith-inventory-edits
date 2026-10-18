# NR/FG Performance Diagnostic 1 — validation (19 October 2026)

Built source `ccc9cfa6bab5246eec8d678c47f788ec7314a9b4` on isolated branch
`diagnostic-nr-fg-timing-20261019`. [CI run39173606523](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39173606523) succeeded: DX11-AVX only, bridge/input probe compilation and telemetry self-test. No public release/tag/download or website changes.

Engine artifact12154076409:77797793bytes; downloaded ZIP SHA256
187b9a70fc8eb4f920a96fa8e40ce6c2ee492e92ac09569afb99ff863e6fb035 matches GitHub digest.
EXE31139840bytes; SHA256 `2754ed91aeef6d47bf03b4894eb9802be3c514b097515d5b0f5d682fb8a5abbc`. Verified x64PE and diagnostic marker.
Player ZIP **Squared Away NR FG Performance Diagnostic 1.zip**: 11307449bytes;
SHA256 `fa79507936b526da6d3ce1641fe0943635d99e5d9ca68c6979812e70354359c2`. ZIP CRC/readback and every payload verified. Only installed replacement is bin/AnomalyDX11AVX.exe; other entries are instructions, license/notices and manifest. PDB retained separately in local CI artifact, not player ZIP.

Local RTX5080 standalone validation used isolated copies of the published runtime,
unchanged companion C0EB533DDB19516B5F0A932BDA78D7D8B6B05AC6BEA64B4E33AC599A75E69D0A and suppliedINI/models. No installed files read or written during this build task.
- Input/scope/depth/motion/UI/context-state probe passed.
- Automatic normal-startup smoke passed without old environment variables.
- Lifecycle passed960presentations,7resizes,8readbacks and NR cleanup.
- Interactive1800presentation probe collected all4GPUtiming stages, invalid0/skipped0, finalreadback/cleanup passed. Exit9: sustainedFGthreshold failed after focus loss. Only60frames foreground,42two-present reports (allforeground), lastforegroundframe99. Earlier attempt exit13unabletofocus preserved. Neither is a full foregroundFGpass. User gameplay, optics and long-session validation remain required.

Engine changes only add sampled asynchronous timestamps. Every8bridge ticks,
4slots/stage; D11 disjoint/timestamps read DONOTFLUSH; D12 results polled after
existing copy-submission fence. No added waits, flushes, resource ownership changes,
optimization or companion changes. ExistingCPU FG TIMING and NR GPU windows remain.
Eachsample bucket labels actualFG/scoped/knownforeground; NRstate not observed.
GPU elapsed intervals may include scheduling/preemption and are not opaqueFGGPUcost
or crossqueuewaittimings. Missing/rejected samples are not zero cost. CPU/GPU
intervals overlap and must not be summed as frame time. Instrumentation overhead
is not independently benchmarked against baseline.

Player test: same save/view/settings, separate30second NRoff/on x FGoff/on runs;
record rendered AND presentedFPS and preserve bothlogs perrun. Optional NRoff/on
with-dxgi_old andFGoff bypasses presentation/Reflex but is not a singlefence test.
Install/rollback: back up existingEXE, replace onlyEXE on current optionalFGbase;
restorebackup to roll back. No changes to INI, models, db or MO2patches.

Evidence and delivery: C:/GAMMA/sqa-previews/personal-frame-generation/performance-diagnostic-20261019/
`package-verification.json`, `probe-validation.json`, `ci-run.json`, `ci-artifacts.json`,
probe logs/cadence, source patch, source/runtime licenses and playerZIP.
