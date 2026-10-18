#pragma once
// Diagnostic build only. Sampled, asynchronous timestamps; never flush or wait
// for telemetry. GPU intervals include scheduling/preemption, not pure shader cost.
#include <d3d11_4.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdio>

class PersonalFgGpuTiming
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    static constexpr unsigned Slots = 4, Stages = 3, Buckets = 8;
    struct Meta { unsigned frame=0, bucket=4; };
    struct Sample11 {
        Ptr<ID3D11Query> disjoint, start, end;
        Meta meta;
        bool pending=false, recording=false;
    };
    struct Sample12 { Meta meta; UINT64 fence=0; bool pending=false; };
    struct Stats {
        unsigned count=0;
        double sum=0, maximum=0;
        void Add(double ms) { ++count; sum+=ms; if(ms>maximum)maximum=ms; }
    };
    Sample11 samples11[Stages][Slots];
    Sample12 samples12[Slots];
    Stats stats[Stages+1][Buckets];
    Ptr<ID3D12QueryHeap> heap;
    Ptr<ID3D12Resource> readback;
    UINT64* mapped=nullptr;
    UINT64 frequency=0;
    unsigned frame=0, skipped=0, invalid=0;
    bool ready11=false, ready12=false;
    void (*log)(const char*)=nullptr;
    static const char* Name(unsigned stage) {
        static const char* names[]{"d11_inputs_including_scope", "d11_hudless_copy", "d11_ui_protection", "d12_backbuffer_copy_barriers"};
        return names[stage];
    }
public:
    ~PersonalFgGpuTiming() {
        if(mapped) { D3D12_RANGE written{0,0}; readback->Unmap(0,&written); }
    }
    void Initialize(ID3D11Device* d11, ID3D12Device* d12, ID3D12CommandQueue* queue,
        void (*logger)(const char*)) {
        log=logger;
        ready11=true;
        for(auto& stage:samples11)for(auto& s:stage) {
            D3D11_QUERY_DESC desc{D3D11_QUERY_TIMESTAMP_DISJOINT,0};
            if(FAILED(d11->CreateQuery(&desc,s.disjoint.GetAddressOf())))ready11=false;
            desc.Query=D3D11_QUERY_TIMESTAMP;
            if(FAILED(d11->CreateQuery(&desc,s.start.GetAddressOf())))ready11=false;
            if(FAILED(d11->CreateQuery(&desc,s.end.GetAddressOf())))ready11=false;
        }
        D3D12_QUERY_HEAP_DESC q{};q.Type=D3D12_QUERY_HEAP_TYPE_TIMESTAMP;q.Count=Slots*2;
        D3D12_HEAP_PROPERTIES h{};h.Type=D3D12_HEAP_TYPE_READBACK;h.CreationNodeMask=h.VisibleNodeMask=1;
        D3D12_RESOURCE_DESC r{};r.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
        r.Width=Slots*2*sizeof(UINT64);r.Height=r.DepthOrArraySize=r.MipLevels=1;
        r.SampleDesc.Count=1;r.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        if(SUCCEEDED(queue->GetTimestampFrequency(&frequency)) && frequency &&
            SUCCEEDED(d12->CreateQueryHeap(&q,IID_PPV_ARGS(heap.GetAddressOf()))) &&
            SUCCEEDED(d12->CreateCommittedResource(&h,D3D12_HEAP_FLAG_NONE,&r,D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,IID_PPV_ARGS(readback.GetAddressOf())))) {
            D3D12_RANGE range{0,SIZE_T(r.Width)};
            ready12=SUCCEEDED(readback->Map(0,&range,reinterpret_cast<void**>(&mapped)));
        }
        if(log) {
            char line[512];
            snprintf(line,sizeof(line),"[FG GPU DIAG] v1 initialized d11=%u d12=%u sampleEvery=8 slots=4. No telemetry waits/flushes; asynchronous GPU elapsed intervals, not queue-wait or opaque FG/NR execution timings. NR state is not observed by engine.",unsigned(ready11),unsigned(ready12));
            log(line);
        }
    }
    void Tick(ID3D11DeviceContext* context, ID3D12Fence* fence) {
        ++frame;
        // Poll only on sampled frames, bounded work; DONOTFLUSH never forces submission.
        if(frame%8)return;
        if(ready11)for(unsigned stage=0;stage<Stages;++stage)for(auto& s:samples11[stage]) {
            if(!s.pending)continue;
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT d{};UINT64 a=0,b=0;
            const HRESULT hd=context->GetData(s.disjoint.Get(),&d,sizeof(d),D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if(hd==S_FALSE)continue;
            if(FAILED(hd)) { ++invalid;s.pending=false;continue; }
            const HRESULT ha=context->GetData(s.start.Get(),&a,sizeof(a),D3D11_ASYNC_GETDATA_DONOTFLUSH);
            const HRESULT hb=context->GetData(s.end.Get(),&b,sizeof(b),D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if(ha==S_FALSE || hb==S_FALSE)continue;
            if(SUCCEEDED(ha)&&SUCCEEDED(hb)&&!d.Disjoint&&d.Frequency&&b>=a)
                stats[stage][s.meta.bucket].Add(double(b-a)*1000.0/d.Frequency);
            else ++invalid;
            s.pending=false;
        }
        if(ready12 && fence) {
            const UINT64 done=fence->GetCompletedValue();
            for(unsigned i=0;i<Slots;++i) {
                auto& s=samples12[i];if(!s.pending)continue;
                if(done==~UINT64(0)){++invalid;s.pending=false;continue;}
                if(!s.fence || done<s.fence)continue;
                const UINT64 a=mapped[2*i],b=mapped[2*i+1];
                if(b>=a)stats[Stages][s.meta.bucket].Add(double(b-a)*1000.0/frequency);
                else ++invalid;
                s.pending=false;
            }
        }
        if(frame%240 || !log)return;
        for(unsigned stage=0;stage<=Stages;++stage)for(unsigned bucket=0;bucket<Buckets;++bucket) {
            auto& s=stats[stage][bucket];if(!s.count)continue;
            char line[384];
            snprintf(line,sizeof(line),"[FG GPU DIAG] tick=%u stage=%s fg=%u scoped=%u foregroundKnown=%u n=%u gpuElapsedMeanMs=%.4f maxMs=%.4f (completed sampled intervals, not whole-frame cost)",
                frame,Name(stage),bucket&1,(bucket>>1)&1,unsigned(!(bucket&4)),s.count,s.sum/s.count,s.maximum);
            log(line);s=Stats{};
        }
        char line[128];snprintf(line,sizeof(line),"[FG GPU DIAG] tick=%u skippedBusyTotal=%u invalidTotal=%u",frame,skipped,invalid);log(line);
    }
    int Begin11(unsigned stage, ID3D11DeviceContext* context) {
        if(!ready11 || frame%8 || stage>=Stages)return -1;
        for(unsigned i=0;i<Slots;++i) {
            auto& s=samples11[stage][i];if(s.pending||s.recording)continue;
            s.meta={frame,4};s.recording=true;
            context->Begin(s.disjoint.Get());context->End(s.start.Get());return int(i);
        }
        ++skipped;return -1;
    }
    void End11(unsigned stage,int index,ID3D11DeviceContext* context) {
        if(index<0)return;
        auto& s=samples11[stage][index];context->End(s.end.Get());context->End(s.disjoint.Get());
        s.recording=false;s.pending=true;
    }
    int Begin12(ID3D12GraphicsCommandList* list) {
        if(!ready12 || frame%8)return -1;
        for(unsigned i=0;i<Slots;++i) {
            auto& s=samples12[i];if(s.pending)continue;
            s.meta={frame,4};s.fence=0;s.pending=true;
            list->EndQuery(heap.Get(),D3D12_QUERY_TYPE_TIMESTAMP,2*i);return int(i);
        }
        ++skipped;return -1;
    }
    void End12(int index,ID3D12GraphicsCommandList* list) {
        if(index<0)return;
        list->EndQuery(heap.Get(),D3D12_QUERY_TYPE_TIMESTAMP,2*index+1);
        list->ResolveQueryData(heap.Get(),D3D12_QUERY_TYPE_TIMESTAMP,2*index,2,readback.Get(),2*index*sizeof(UINT64));
    }
    void Submitted12(int index,UINT64 value) { if(index>=0)samples12[index].fence=value; }
    void Classify(bool fg,bool scoped,bool foregroundKnown) {
        const unsigned bucket=unsigned(fg)+2*unsigned(scoped)+4*unsigned(!foregroundKnown);
        for(auto& stage:samples11)for(auto& s:stage)if(s.pending && s.meta.frame==frame)s.meta.bucket=bucket;
        for(auto& s:samples12)if(s.pending && s.meta.frame==frame)s.meta.bucket=bucket;
    }
};
