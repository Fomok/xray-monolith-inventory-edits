#define SQA3_EXCEPTION_BOUNDARY
#include "stdafx.h"
#include "InventoryNextProbe.h"
#include "InventoryNextIdentity.h"
#include "actor.h"
#include "inventory.h"
#include "inventory_item.h"
#include "level.h"
#include "UIGameCustom.h"
#include "ui/UIDialogWnd.h"
#include "ui/InventoryNextView.h"
#include "xr_level_controller.h"
#include "../xrEngine/xr_input.h"
#include "../inventory_next/model.h"
#include "../inventory_next/entity_registry.h"
#include "../inventory_next/persistence.h"
#include <cmath>
#include <exception>
#include <stdexcept>

namespace {
class InventoryNextSnapshotDialog final : public CUIDialogWnd {
public:
    InventoryNextSnapshotDialog(const inventory_next::Model& model) {
        SetWndPos(Fvector2().set(0,0));SetWndSize(Fvector2().set(1024,768));
        AllowCenterCursor(false);
        auto view=xr_new<CInventoryNextView>();view->SetAutoDelete(true);AttachChild(view);
        view->Initialize(Fvector2().set(20,30),Fvector2().set(984,700));
        inventory_next::Presentation state;state.synchronize(model,1,{});state.open(model,1,12);
        if(!view->Bind(model,state,1,{}))throw std::runtime_error("native view binding failed");
    }
    bool OnKeyboardAction(int key,EUIMessages action) noexcept override try {
        if(key==DIK_ESCAPE&&action==WINDOW_KEY_PRESSED){HideDialog();return true;}
        return CUIDialogWnd::OnKeyboardAction(key,action);
    } catch (...) { HideDialog();return true; }
};
}

void InventoryNextProbe(bool export_snapshot, bool show_view) noexcept
{
    // Probe-assigned diagnostic layout/IDs, NOT native placement or game-save state.
    // All native reads occur synchronously on the explicitly invoked console call.
    if (!g_pGameLevel || !Actor()) { Msg("! SQA3 probe requires a loaded actor"); return; }
    try {
        using namespace inventory_next;
        Node root; root.id=1; root.kind=Kind::ActorRoot; root.category=0;
        root.container=true; root.grid={16,64};
        Node world=root; world.id=2; world.kind=Kind::WorldRoot;
        std::vector<Node> nodes{root,world};
        EntityRegistry registry;
        int x=0,y=0,row_height=0;
        double native_grams=0;
        for (auto* item : Actor()->inventory().m_all) {
            if (!item || nodes.size()>=4096) { Msg("! SQA3 probe: invalid/count-limited inventory"); return; }
            const double grams=static_cast<double>(item->Weight())*1000.0;
            const double condition=item->GetCondition();
            const Irect shape=item->GetInvGridRect();
            const int w=shape.x2,h=shape.y2;
            if (!std::isfinite(grams)||grams<0||grams>1e12||!std::isfinite(condition)||condition<0||condition>1||w<1||h<1||w>16||h>64) {
                Msg("! SQA3 probe: unsupported native value for live ID %u",item->object().ID()); return;
            }
            if (x+w>16) { x=0; y+=row_height; row_height=0; }
            if (y+h>64) { Msg("! SQA3 probe: diagnostic layout capacity exceeded"); return; }
            Node n; n.id=InventoryNextIdentity(item->object().ID());
            if(!n.id) { Msg("! SQA3 probe requires registered persistent ALife items");return; } InventoryNextIdentityOnline(*item); n.parent=1; n.intrinsic=static_cast<Mass>(std::llround(grams));
            n.empty={w,h}; n.half=n.empty; n.full=n.empty; n.position={x,y,false};
            n.condition=static_cast<unsigned>(std::llround(condition*10000.0));
            n.attributes["native_section"]=item->m_section_id.c_str();
            n.attributes["native_live_id"]=std::to_string(item->object().ID());
            n.attributes["native_place"]=std::to_string(item->CurrPlace());
            n.attributes["native_slot"]=std::to_string(item->CurrSlot());
            auto added=registry.add(n.id);
            auto bound=added?registry.bind(n.id,item->object().ID()):added;
            if(!bound) { Msg("! SQA3 probe registry: %s",bound.error.c_str()); return; }
            nodes.push_back(n); native_grams+=grams; x+=w; if(h>row_height)row_height=h;
        }
        Model model; auto result=model.initialize(nodes);
        if(!result) { Msg("! SQA3 probe model: %s",result.error.c_str()); return; }
        Msg("* SQA3 native probe: items=%u native_grams=%.3f core_grams=%lld online=%u; diagnostic layout only",
            static_cast<unsigned>(nodes.size()-2),native_grams,static_cast<long long>(model.carried_mass()),static_cast<unsigned>(registry.online_count()));
        if(show_view) {
            auto ui=CurrentGameUI();
            if(!ui||ui->TopInputReceiver()){Msg("! Close existing game dialogs before opening the SQA3 snapshot");return;}
            auto dialog=xr_new<InventoryNextSnapshotDialog>(model);
            ui->SetInventoryNextDialog(dialog);dialog->ShowDialog(false);
            Msg("* SQA3 frozen native snapshot opened; Escape closes, repeat command refreshes");
        }
        if(export_snapshot) {
            string_path path; FS.update_path(path,"$app_data_root$","sqa3-native-probe.inv");
            auto saved=save_atomic(model,std::filesystem::path(path));
            if(!saved) Msg("! SQA3 probe export: %s",saved.error.c_str());
            else Msg("* SQA3 probe exported to lab appdata; no game inventory changed");
        }
    } catch(const std::exception& e) {
        Msg("! SQA3 probe failed: %s",e.what());
    } catch (...) { Msg("! SQA3 probe failed: unknown core exception"); }
}
