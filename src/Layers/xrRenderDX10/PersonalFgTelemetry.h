// Personal engine/companion telemetry ABI v1. Windows, same process, read-only consumer.
#pragma once
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace sqa_fps {
inline int64_t Clock() { LARGE_INTEGER q{}; QueryPerformanceCounter(&q); return q.QuadPart; }
inline int64_t Frequency() { LARGE_INTEGER q{}; QueryPerformanceFrequency(&q); return q.QuadPart; }
inline void Name(wchar_t (&name)[80]) { swprintf_s(name,L"Local\\SQA_PersonalFG_v1_%lu",GetCurrentProcessId()); }
struct Packet {
    uint32_t magic=0x31514653, version=1, pid=0, flags=0; // bit0 valid, bit1 FG enabled
    int64_t frequency=0, stamp=0, epoch=0;
    uint64_t app=0, presented=0;
};
struct alignas(8) Shared { volatile LONG sequence; LONG reserved; Packet data; };
static_assert(sizeof(Packet)==56 && sizeof(Shared)==64,"Telemetry ABI changed");

class Publisher {
    HANDLE mapping=nullptr; Shared* view=nullptr; Packet packet{};
    bool attempted=false;
public:
    Publisher()=default; Publisher(const Publisher&)=delete; Publisher& operator=(const Publisher&)=delete;
    ~Publisher() { Stop(); if(view)UnmapViewOfFile(view); if(mapping)CloseHandle(mapping); }
    void Publish(uint32_t presented, bool valid, bool enabled, int64_t now=Clock()) {
        if(!attempted) {
            attempted=true; wchar_t name[80]; Name(name);
            mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Shared),name);
            if(!mapping)return;
            if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(mapping);mapping=nullptr;return;}
            view=static_cast<Shared*>(MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(Shared)));
            packet.pid=GetCurrentProcessId(); packet.frequency=Frequency(); packet.epoch=now;
        }
        if(!view)return;
        // A reader may have been hidden throughout loading. Mark a fresh epoch
        // so its next sample never averages across a presentation gap.
        if(packet.stamp && (now<packet.stamp || now-packet.stamp>packet.frequency))packet.epoch=now;
        packet.stamp=now; packet.flags=(valid?1u:0u)|(enabled?2u:0u);
        ++packet.app; if(valid)packet.presented+=presented;
        InterlockedIncrement(&view->sequence); MemoryBarrier();
        view->data=packet;
        MemoryBarrier(); InterlockedIncrement(&view->sequence);
    }
    void Stop() { if(view)Publish(0,false,false); }
};

enum class State { Unavailable, Warming, Live, Stale };
struct Rates { State state=State::Unavailable; double app=0, presented=0; bool enabled=false; };
class Reader {
    HANDLE mapping=nullptr; const Shared* view=nullptr; int64_t nextOpen=0;
    Packet base{}; bool haveBase=false; Rates rates{};
public:
    Reader()=default; Reader(const Reader&)=delete; Reader& operator=(const Reader&)=delete;
    ~Reader(){if(view)UnmapViewOfFile(view);if(mapping)CloseHandle(mapping);}
    Rates Read(int64_t now=Clock()) {
        if(!view && now>=nextOpen) {
            nextOpen=now+Frequency(); wchar_t name[80]; Name(name);
            mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
            if(mapping) {
                view=static_cast<const Shared*>(MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(Shared)));
                if(!view){CloseHandle(mapping);mapping=nullptr;}
            }
        }
        Packet p{}; bool copied=false;
        if(view)for(int attempt=0;attempt<3;++attempt){
            const LONG before=view->sequence; if(before&1)continue;
            MemoryBarrier(); std::memcpy(&p,&view->data,sizeof(p)); MemoryBarrier();
            if(before==view->sequence){copied=true;break;}
        }
        if(!copied || p.magic!=0x31514653 || p.version!=1 || p.pid!=GetCurrentProcessId() || p.frequency<=0 || !(p.flags&1)) {
            haveBase=false; rates={}; return rates;
        }
        if(now<p.stamp || now-p.stamp>p.frequency) {
            haveBase=false; rates={}; rates.state=State::Stale; return rates;
        }
        if(!haveBase || p.epoch!=base.epoch || p.flags!=base.flags || p.app<base.app || p.presented<base.presented || p.stamp<base.stamp) {
            base=p;haveBase=true;rates={};rates.state=State::Warming;rates.enabled=(p.flags&2)!=0;
        }
        const int64_t elapsed=p.stamp-base.stamp;
        if(elapsed>=p.frequency/2) {
            rates.app=double(p.app-base.app)*double(p.frequency)/double(elapsed);
            rates.presented=double(p.presented-base.presented)*double(p.frequency)/double(elapsed);
            rates.state=State::Live; rates.enabled=(p.flags&2)!=0; base=p;
        }
        return rates;
    }
};
} // namespace sqa_fps
