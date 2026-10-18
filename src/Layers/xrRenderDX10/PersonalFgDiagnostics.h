#pragma once
#include <cstdint>

// Diagnostic-only snapshots. Never used to decide eligibility, pacing or input reuse.
// Scoped/cumulative reporting inspired by Brodrigz 1.0.2; no extra SDK queries.
namespace personal_fg_diag
{
struct Context
{
    bool known=false, scoped=false, pip=false, foreground=false, scopeInputs=false;
    uint32_t engineFrame=0, scopeEpoch=0, scopeSession=0;
};
struct Counters
{
    uint64_t enabled=0, valid=0, invalid=0, presented=0, generated=0;
    uint64_t scopeValid=0, scopeGenerated=0, unknownContext=0;
    void Observe(bool on, bool usable, uint32_t count, const Context& context)
    {
        if (!on) return;
        ++enabled;
        if (!usable) { ++invalid; return; }
        ++valid; presented+=count;
        if (count>1) ++generated;
        if (!context.known) ++unknownContext;
        if (context.known && context.scoped && context.scopeInputs)
        {
            ++scopeValid;
            if (count>1) ++scopeGenerated;
        }
    }
};
}
