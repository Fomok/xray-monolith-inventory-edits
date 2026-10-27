#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include "model.h"
#include "presentation.h"
#include "persistence.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Read-only diagnostic renderer. Uses the shared model for every size/mass and
// legal placement; no packing, transfer or inventory simulation lives here.
using namespace inventory_next;
namespace {
constexpr int width = 1600, height = 1100;
struct Surface {
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old_bitmap = nullptr;
    void* pixels = nullptr;
    HFONT font = nullptr;
    Surface() {
        dc = CreateCompatibleDC(nullptr);
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
        if (!dc || !bitmap || !pixels) throw std::runtime_error("gdi_surface");
        old_bitmap = SelectObject(dc, bitmap);
        font = CreateFontW(-19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        SetBkMode(dc, TRANSPARENT);
    }
    ~Surface() {
        if (dc && old_bitmap) SelectObject(dc, old_bitmap);
        if (font) DeleteObject(font);
        if (bitmap) DeleteObject(bitmap);
        if (dc) DeleteDC(dc);
    }
    void box(int x,int y,int w,int h,COLORREF color) {
        RECT rect{x,y,x+w,y+h};
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &rect, brush); DeleteObject(brush);
    }
    void text(int x,int y,int w,const std::wstring& value,COLORREF color=RGB(220,224,213)) {
        auto old = SelectObject(dc, font);
        SetTextColor(dc,color);
        RECT rect{x,y,x+w,y+25};
        DrawTextW(dc,value.c_str(),static_cast<int>(value.size()),&rect,DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
        SelectObject(dc,old);
    }
    void save(const std::filesystem::path& path) {
        GdiFlush();
        BITMAPFILEHEADER header{};
        header.bfType=0x4d42;
        header.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER);
        header.bfSize=header.bfOffBits+width*height*4;
        BITMAPINFOHEADER info{};
        info.biSize=sizeof(info);info.biWidth=width;info.biHeight=-height;
        info.biPlanes=1;info.biBitCount=32;info.biCompression=BI_RGB;
        info.biSizeImage=width*height*4;
        HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("output_exists_or_unwritable");
        DWORD count=0;
        bool ok=WriteFile(file,&header,sizeof(header),&count,nullptr)&&count==sizeof(header);
        ok=ok&&WriteFile(file,&info,sizeof(info),&count,nullptr)&&count==sizeof(info);
        ok=ok&&WriteFile(file,pixels,info.biSizeImage,&count,nullptr)&&count==info.biSizeImage;
        ok=ok&&FlushFileBuffers(file);
        CloseHandle(file);
        if(!ok)throw std::runtime_error("output_write_failed");
    }
};
std::wstring num(std::uint64_t n){return std::to_wstring(n);}
std::wstring mass(Mass n){return std::to_wstring(n)+L" g";}
Id first_root(const Model& model,Kind kind){
    for(const auto& [id,n]:model.nodes())if(n.kind==kind)return id;
    return 0;
}
void panel(Surface& s,const Model& model,int x,const std::wstring& title,Id id){
    constexpr int y=160,w=496;
    s.box(x,y,w,870,RGB(24,29,28));
    s.text(x+16,y+15,w-32,title,RGB(218,187,139));
    if(!id){s.text(x+16,y+55,w-32,L"No source selected");return;}
    const auto it=model.nodes().find(id);
    if(it==model.nodes().end()||!it->second.container)throw std::runtime_error("selected_container_missing");
    const auto& n=it->second;
    s.text(x+16,y+48,w-32,L"ID "+num(id)+L"  |  "+mass(model.subtree_mass(id)));
    s.text(x+16,y+76,w-32,num(n.grid.w)+L" x "+num(n.grid.h)+L" interior cells");
    const int cell=std::max(1,std::min({44,(w-32)/n.grid.w,560/n.grid.h}));
    const int gx=x+16,gy=y+117;
    for(int row=0;row<n.grid.h;++row)for(int col=0;col<n.grid.w;++col) {
        auto region=model.cell_region(id,col,row);
        COLORREF shade=region?(region%2?RGB(40,46,43):RGB(49,42,57)):RGB(14,15,15);
        s.box(gx+col*cell,gy+row*cell,cell-1,cell-1,shade);
    }
    int children=0,equipped=0,mounted=0,overflow=0;
    for(const auto& [child,c]:model.nodes())if(c.kind==Kind::Item&&c.parent==id){
        if(c.location==Location::Overflow){++overflow;continue;}
        if(c.location==Location::Equipment){++equipped;continue;}
        if(c.location==Location::Mounted){++mounted;continue;}
        ++children;
        Size f=model.footprint(child);
        if(c.position.rotated)std::swap(f.w,f.h);
        const int px=gx+c.position.x*cell,py=gy+c.position.y*cell;
        s.box(px,py,f.w*cell-1,f.h*cell-1,c.container?RGB(92,109,75):RGB(65,83,91));
        if(cell>=27)s.text(px+3,py+2,f.w*cell-6,num(child)+(c.quantity>1?L" x"+num(c.quantity):L""));
    }
    int line=gy+n.grid.h*cell+20;
    if(overflow) {
        Presentation view;view.synchronize(model,id,{});view.open(model,id);
        const auto layout=view.overflow_layout(model,id);
        s.text(x+16,line,w-32,L"Does not fit: "+num(overflow)+L" - recovery overflow",RGB(255,120,120));line+=32;
        const int overflowCell=std::max(1,std::min(cell,(w-32)/std::max(1,layout.columns)));
        for(const auto& tile:layout.tiles){const auto& v=tile.first;const auto& p=tile.second;
            s.box(gx+p.x*overflowCell,line+p.y*overflowCell,v.footprint.w*overflowCell-1,v.footprint.h*overflowCell-1,RGB(95,35,35));
            s.text(gx+p.x*overflowCell+3,line+p.y*overflowCell+2,v.footprint.w*overflowCell-6,L"#"+num(v.id),RGB(255,170,170));
        }
        line+=layout.rows*overflowCell+16;
    }
    s.text(x+16,line,w-32,num(children)+L" grid | "+num(equipped)+L" equipped | "+num(mounted)+L" mounted");
    line+=35;
    int shown=0;
    for(const auto& [child,c]:model.nodes())if(c.kind==Kind::Item&&c.parent==id){
        if(line+25>y+850)break;
        Size f=model.footprint(child);
        s.text(x+16,line,w-32,L"#"+num(child)+L" x"+num(c.quantity)+L"  "+num(f.w)+L"x"+num(f.h)+L"  "+mass(model.subtree_mass(child))+(c.location==Location::Overflow?L"  [overflow]":c.location==Location::Equipment?L"  [equipped]":c.location==Location::Mounted?L"  [mount "+num(c.mounted_slot)+L"]":(c.container?L"  [container]":L"")));
        line+=27;++shown;
    }
    if(shown<children+equipped+mounted+overflow)s.text(x+16,y+839,w-32,L"Additional item labels omitted; grid is complete",RGB(206,139,155));
}
}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
    std::filesystem::path output;
    try{
        int argc=0;
        LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
        if(!argv)return 2;
        std::filesystem::path input;
        Id selected=0,actor=0,world=0;
        for(int i=1;i<argc;++i){
            const std::wstring key=argv[i];
            if(i+1>=argc){LocalFree(argv);return 2;}
            const std::wstring value=argv[++i];
            if(key==L"--snapshot")input=value;
            else if(key==L"--output")output=value;
            else if(key==L"--container"||key==L"--actor"||key==L"--world"){
                if(value.empty()||value.find_first_not_of(L"0123456789")!=std::wstring::npos){LocalFree(argv);return 2;}
                const Id id=std::stoull(value);
                if(key==L"--container")selected=id;else if(key==L"--actor")actor=id;else world=id;
            }else{LocalFree(argv);return 2;}
        }
        LocalFree(argv);
        if(input.empty()||output.empty())return 2;
        Model model;
        const auto result=load_file(model,input);
        if(!result)throw std::runtime_error("invalid_snapshot: "+result.error);
        const auto before=model.serialize();
        if(!actor)actor=first_root(model,Kind::ActorRoot);
        if(!world)world=first_root(model,Kind::WorldRoot);
        if(actor&&(!model.nodes().count(actor)||model.nodes().at(actor).kind!=Kind::ActorRoot))throw std::runtime_error("actor_root_kind");
        if(world&&(!model.nodes().count(world)||model.nodes().at(world).kind!=Kind::WorldRoot))throw std::runtime_error("world_root_kind");
        if(!selected)for(const auto& [id,n]:model.nodes())if(n.kind==Kind::Item&&n.container){selected=id;break;}
        Surface surface;
        surface.box(0,0,width,height,RGB(17,21,21));
        surface.text(30,22,1540,L"SQUARED AWAY 3.0  /  Background inventory snapshot",RGB(218,187,139));
        surface.text(30,55,1540,L"Diagnostic view of the shared C++ model. No game rendering, pickup, input or inventory changes.");
        surface.text(30,92,1540,L"Revision "+num(model.revision())+L"  |  Carried mass "+mass(model.carried_mass())+L"  |  "+num(model.nodes().size())+L" model nodes");
        panel(surface,model,24,L"CARRIED ROOT",actor);
        panel(surface,model,552,L"OPEN CONTAINER",selected);
        panel(surface,model,1080,L"OBSERVED WORLD ROOT",world);
        surface.text(30,1055,1540,L"Select container IDs to inspect nested storage. World discovery and in-game UI remain separate integration work.",RGB(149,163,155));
        if(before!=model.serialize())throw std::runtime_error("renderer_mutated_model");
        surface.save(output);
        return 0;
    }catch(const std::exception& error){
        if(!output.empty()){
            std::ofstream log(std::filesystem::path(output.wstring()+L".error.txt"));
            log<<error.what()<<'\n';
        }
        return 1;
    }
}
