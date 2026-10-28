namespace {
class SqaContainerPanel final : public CUIStatic {
public:
    bool OnMouseAction(float x,float y,EUIMessages action) override {
        const bool handled=CUIStatic::OnMouseAction(x,y,action);
        return handled||(x>=0&&y>=0&&x<GetWndSize().x&&y<GetWndSize().y);
    }
};
}
// Included in UIActorMenu.cpp: presentation owns windows, never inventory state.
CUIDragDropListEx* CUIActorMenu::SqaContainerList(u64 owner)
{
    for(auto& window:m_sqaContainers)if(window.owner==owner)return window.list;
    return nullptr;
}
bool CUIActorMenu::SqaActorList(CUIDragDropListEx* list) const
{
    if(list==m_pInventoryBagList)return true;
    if(list)for(auto equipment:m_sqaCustomEquipment)if(list==equipment)return true;
    if(InventoryNextFieldKitEnabled())for(const auto& window:m_sqaContainers)if(window.list==list)return true;
    return false;
}
bool CUIActorMenu::SqaOpenContainer(PIItem item)
{
    if(!SqaLootActive()||!item||CUIDragDropListEx::m_drag_item)return false;
    const auto owner=InventoryNextContainer(item->object_id());if(!owner)return false;
    if(SqaContainerList(owner))return true;
    // Bound presentation allocations independently of the number of owned bags.
    if(m_sqaContainers.size()>=4){InventoryNextPickupStatus("Close a container window before opening another (4 open).");return true;}
    SqaContainerWindow window;window.owner=owner;window.live=item->object_id();
    window.frame=xr_new<SqaContainerPanel>();window.frame->SetAutoDelete(true);AttachChild(window.frame);
    const float offset=float(m_sqaContainers.size())*28.0f;
    window.frame->SetWndPos(Fvector2().set(340+offset,140+offset));window.frame->SetWndSize(Fvector2().set(290,380));
    window.frame->TextureOff();
    auto rim=window.rim=xr_new<CUIFrameWindow>();rim->SetAutoDelete(true);window.frame->AttachChild(rim);rim->Enable(false);rim->SetWndSize(window.frame->GetWndSize());rim->InitTexture("sqa3v1_panel_frame");
    auto title=window.title=xr_new<CUIStatic>();title->SetAutoDelete(true);window.frame->AttachChild(title);title->Enable(false);
    title->SetWndPos(Fvector2().set(10,5));title->SetWndSize(Fvector2().set(246,20));title->TextureOff();
    title->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);title->TextItemControl()->SetText(item->NameItem());
    auto close=window.close=xr_new<CUI3tButton>();close->SetAutoDelete(true);window.frame->AttachChild(close);
    close->InitButton(Fvector2().set(266,4),Fvector2().set(20,20));close->InitTexture("sqa3v2_close");
    window.list=xr_new<CUIDragDropListEx>();window.list->SetAutoDelete(true);window.frame->AttachChild(window.list);
    window.list->InitDragDropList(Fvector2().set(12,32),Fvector2().set(266,334));
    window.list->SetCellSize(Ivector2().set(32,32));window.list->SetCellsSpacing(Ivector2().set(0,0));
    window.list->SetAutoGrow(false);window.list->SetGrouping(false);window.list->SetCustomPlacement(true);
    window.list->back_color=0x40454545;window.list->sqaViewOwner=owner;BindDragDropListEvents(window.list);
    m_sqaContainers.push_back(window);m_sqaRefresh=true;
    Msg("* SQA3 field kit opened container=%llu live=%u",owner,window.live);
    return true;
}
void CUIActorMenu::SqaPrepareContainers()
{
    for(auto it=m_sqaContainers.begin();it!=m_sqaContainers.end();) {
        if(!InventoryNextFieldKitEnabled()||InventoryNextContainer(it->live)!=it->owner) {
            SetCurrentItem(nullptr);it->list->ClearAll(true);DetachChild(it->frame);it=m_sqaContainers.erase(it);
        } else {
            int columns=0,rows=0;
            if(InventoryNextStorageExtent(it->owner,columns,rows)){
                const int step=_min(32,_max(1,908/_max(1,columns)));
                if(it->list->CellSize().x!=step)it->list->SetCellSize(Ivector2().set(step,step));
                const float width=_max(290.0f,float(columns*step)+52.0f);
                const float delta=width-it->frame->GetWndSize().x;
                if(delta!=0){auto position=it->frame->GetWndPos();position.x-=delta*0.5f;clamp(position.x,0.0f,1024.0f-width);
                    it->frame->SetWndPos(position);it->frame->SetWndSize(Fvector2().set(width,380));it->rim->SetWndSize(it->frame->GetWndSize());
                    it->title->SetWndSize(Fvector2().set(width-44.0f,20));it->close->SetWndPos(Fvector2().set(width-24.0f,4));
                    it->list->SetWndSize(Fvector2().set(width-24.0f,334));}
            }
            InventoryNextStorageView(it->list,it->owner);
            it->frame->Show(SqaLootActive());++it;
        }
    }
}
bool CUIActorMenu::SqaCloseContainerAt(float x,float y)
{
    if(CUIDragDropListEx::m_drag_item)return false;
    for(size_t n=m_sqaContainers.size();n>0;--n) {
        auto& window=m_sqaContainers[n-1];const auto position=window.frame->GetWndPos();
        const auto width=window.frame->GetWndSize().x;
        if(window.frame->IsShown()&&x>=position.x+width-24&&x<position.x+width-4&&y>=position.y+4&&y<position.y+24) {
            SetCurrentItem(nullptr);window.list->ClearAll(true);DetachChild(window.frame);
            m_sqaContainers.erase(m_sqaContainers.begin()+n-1);m_sqaRefresh=true;return true;
        }
    }
    return false;
}

void CUIActorMenu::SqaUpdateChest()
{
    if(!InventoryNextFieldKitEnabled())return;
    // Vanilla retains two native identities; present them inside one utility frame.
    // If both are equipped, share that frame without hiding or moving either item.
    if(m_pInvList[1]&&m_pInvList[5]){
        const bool knife=m_pInvList[1]->ItemsCount()!=0,binocular=m_pInvList[5]->ItemsCount()!=0;
        const bool both=knife&&binocular;
        m_pInvList[1]->SetWndPos(Fvector2().set(93,171));m_pInvList[1]->SetWndSize(Fvector2().set(both?36.0f:75.0f,42.0f));
        m_pInvList[5]->SetWndPos(Fvector2().set(both?132.0f:93.0f,171.0f));m_pInvList[5]->SetWndSize(Fvector2().set(both?36.0f:75.0f,42.0f));
        m_pInvList[1]->Show(knife||!binocular);m_pInvList[5]->Show(binocular);
        for(int slot:{1,5})if(m_pInvList[slot]->ItemsCount()){
            auto item=m_pInvList[slot]->GetItemIdx(0);const auto grid=item->GetGridSize();
            const float width=m_pInvList[slot]->GetWndSize().x-4.0f;
            const float scale=_min(width/float(grid.x),28.0f/float(grid.y));
            item->SetWndSize(Fvector2().set(scale*grid.x,scale*grid.y));
            item->SetWndPos(Fvector2().set((width+4.0f-scale*grid.x)*0.5f,(28.0f-scale*grid.y)*0.5f));
        }
        if(m_pInvSlotHighlight[5])m_pInvSlotHighlight[5]->Show(false);
    }
    if(m_sqaChestCaption)m_sqaChestCaption->TextItemControl()->SetText(m_sqaRigView?"Rig":"Armour");
    if(m_pInvList[OUTFIT_SLOT])m_pInvList[OUTFIT_SLOT]->Show(!m_sqaRigView);
    if(m_pInvList[16])m_pInvList[16]->Show(false); // reserved native slot, never reused
    const auto rig_live=InventoryNextCustomItem(1,"sqa3.rig");
    const u64 rig_identity=rig_live!=u16(-1)?InventoryNextIdentityLookup(rig_live):0;
    const u16 equipment_live[3]={rig_live,rig_identity?InventoryNextCustomItem(rig_identity,"sqa3.mount.p1"):u16(-1),rig_identity?InventoryNextCustomItem(rig_identity,"sqa3.mount.p2"):u16(-1)};
    for(int index=0;index<3;++index)if(auto list=m_sqaCustomEquipment[index]){
        const bool visible=m_sqaRigView&&SqaLootActive();list->Show(visible);
        if(index&&m_sqaPouchFrames[index-1])m_sqaPouchFrames[index-1]->Show(visible);
        auto item=equipment_live[index]!=u16(-1)?smart_cast<CInventoryItem*>(Level().Objects.net_Find(equipment_live[index])):nullptr;
        const bool same=list->ItemsCount()==(item?1u:0u)&&(!item||list->GetItemIdx(0)->m_pData==item);
        if(!same&&!CUIDragDropListEx::m_drag_item){
            SetCurrentItem(nullptr);list->ClearAll(true);
            if(item){auto cell=create_cell_item(item);cell->SetSqaFootprint(1,1,false);list->SetItem(cell);
                const auto original=item->GetInvGridRect();const float scale=_min((list->GetWndSize().x-4.0f)/float(original.x2),(list->GetWndSize().y-4.0f)/float(original.y2));
                cell->SetWndSize(Fvector2().set(original.x2*scale,original.y2*scale));cell->SetWndPos(Fvector2().set((list->GetWndSize().x-cell->GetWndSize().x)*0.5f,(list->GetWndSize().y-cell->GetWndSize().y)*0.5f));}
        }
    }
    if(m_sqaRigView){
        if(m_pInvSlotHighlight[OUTFIT_SLOT])m_pInvSlotHighlight[OUTFIT_SLOT]->Show(false);
        if(m_pInvSlotProgress[OUTFIT_SLOT])m_pInvSlotProgress[OUTFIT_SLOT]->Show(false);
    }else{
        if(m_pInvSlotHighlight[16])m_pInvSlotHighlight[16]->Show(false);
        if(m_pInvSlotProgress[16])m_pInvSlotProgress[16]->Show(false);
    }
}

bool CUIActorMenu::SqaLootActive() const
{
    return InventoryNextFieldKitEnabled()&&(m_currMenuMode==mmInventory||(m_currMenuMode==mmDeadBodySearch&&(!m_pPartnerInvOwner||!m_pPartnerInvOwner->is_alive())));
}
void CUIActorMenu::SqaUpdateLoot(bool force)
{
    if(!InventoryNextFieldKitEnabled())return;
    const bool active=SqaLootActive();
    for(auto tab:m_sqaLootTabs)tab->Show(active);
    if(!active)return;
    if(CUIDragDropListEx::m_drag_item)return;
    if(!force&&Device.dwTimeGlobal<m_sqaLootAfter)return;
    m_sqaLootAfter=Device.dwTimeGlobal+500;
    const u16 corpse=m_currMenuMode==mmDeadBodySearch&&m_pPartnerInvOwner?m_pPartnerInvOwner->object_id():0xffff;
    auto sources=InventoryNextNearbyLoot(corpse);
    auto selected=std::find_if(sources.begin(),sources.end(),[&](const InventoryNextLootSource& source){return source.live==m_sqaLootSource&&source.identity==m_sqaLootSourceIdentity;});
    if(selected==sources.end()&&!sources.empty())selected=sources.begin();
    bool unchanged=sources.size()==m_sqaLootSources.size();
    if(unchanged)for(size_t n=0;n<sources.size();++n)if(sources[n].live!=m_sqaLootSources[n].live||sources[n].identity!=m_sqaLootSources[n].identity||sources[n].items!=m_sqaLootSources[n].items||sources[n].grams!=m_sqaLootSources[n].grams||sources[n].name!=m_sqaLootSources[n].name){unchanged=false;break;}
    if(!force&&unchanged&&sources.empty()&&m_pDeadBodyBagList->ItemsCount()==0){
        m_LeftBackground->Show(false);m_pDeadBodyBagList->Show(false);m_takeall_button->Show(false);m_putall_button->Show(false);
        m_PartnerCharacterInfo->Show(false);m_PartnerMoney->Show(false);m_PartnerBottomInfo->Show(false);m_PartnerWeight->Show(false);return;
    }
    if(!force&&unchanged&&!sources.empty()&&m_LeftBackground->IsShown()&&m_pDeadBodyBagList->IsShown()&&selected->live==m_sqaLootSource&&selected->identity==m_sqaLootSourceIdentity&&m_sqaLootFilter==current_sort_mode()&&m_pDeadBodyBagList->ItemsCount()==m_sqaLootRenderedCount)return;
    if(CurrentItem()&&CurrentItem()->OwnerList()==m_pDeadBodyBagList){SetCurrentItem(nullptr);InfoCurItem(nullptr);m_UIPropertiesBox->Hide();}
    m_pDeadBodyBagList->ClearAll(true);m_sqaLootIdentity.clear();
    for(auto tab:m_sqaLootTabs)DetachChild(tab);m_sqaLootTabs.clear();
    const bool available=!sources.empty();
    if(!unchanged){Msg("* SQA3 nearby source count=%u",static_cast<unsigned>(sources.size()));for(const auto& source:sources)Msg("* SQA3 nearby source live=%u items=%u grams=%lld",source.live,static_cast<unsigned>(source.items.size()),source.grams);}
    m_LeftBackground->Show(available);m_pDeadBodyBagList->Show(available);
    m_takeall_button->Show(available);m_putall_button->Show(available&&selected->live!=0xffff);
    m_PartnerCharacterInfo->Show(available);m_PartnerMoney->Show(available);m_PartnerBottomInfo->Show(available);m_PartnerWeight->Show(available);
    if(!available){m_sqaLootSources.clear();return;}
    m_sqaLootSource=selected->live;m_sqaLootSourceIdentity=selected->identity;
    if(selected->live==corpse && m_pPartnerInvOwner && !m_pPartnerInvOwner->use_simplified_visual()) {
        m_PartnerCharacterInfo->InitCharacter(selected->live);
        string64 money;xr_sprintf(money,"%u RU",m_pPartnerInvOwner->get_money());m_PartnerMoney->SetText(money);
    } else {
        const bool ground=selected->live==0xffff;
        const bool box=!ground&&smart_cast<CInventoryBox*>(Level().Objects.net_Find(selected->live));
        m_PartnerCharacterInfo->InitSource(selected->name.c_str(),ground?"Nearby ground":box?"Accessible stash":"Corpse",
            box?"ui_inGame2_PDA_icon_backpackstash":nullptr);
        string64 contents;xr_sprintf(contents,"%u items | %.2f kg",static_cast<unsigned>(selected->items.size()),double(selected->grams)/1000.0);
        m_PartnerMoney->SetText(contents);
    }
    const float tabHeight=float((sources.size()+2)/3)*22.0f;
    m_pDeadBodyBagList->InitDragDropList(Fvector2().set(680.0f,138.0f+tabHeight),Fvector2().set(306.0f,_max(100.0f,550.0f-tabHeight)));
    m_pDeadBodyBagList->ReinitScroll();
    for(size_t n=0;n<sources.size();++n){
        auto tab=xr_new<CUIStatic>();tab->SetAutoDelete(true);AttachChild(tab);tab->Enable(false);
        tab->SetWndPos(Fvector2().set(652+float(n%3)*120,114+float(n/3)*22));tab->SetWndSize(Fvector2().set(116,20));
        tab->InitTexture(sources[n].live==selected->live?"sqa_fk_surface_olive":"sqa_fk_surface_rubber");tab->SetStretchTexture(true);
        tab->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);tab->TextItemControl()->SetEllipsis(true);tab->TextItemControl()->SetText(sources[n].name.c_str());
        m_sqaLootTabs.push_back(tab);
    }
    const double weight=double(selected->grams)/1000.0;
    for(auto live:selected->items){auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));if(!item)continue;
        const int mode=current_sort_mode();bool matches=0==xr_strcmp(m_sort_kinds[mode],"s_all");
        if(!matches)for(int n=0;n<_GetItemCount(m_sort_kinds[mode]);++n){string256 kind;_GetItem(m_sort_kinds[mode],n,kind);if(item->m_kind!=NULL&&item->m_kind.equal(kind)){matches=true;break;}}
        if(!matches)continue;
        m_sqaLootIdentity[live]=InventoryNextIdentity(live);
        m_pDeadBodyBagList->SetItem(create_cell_item(item));
    }
    m_sqaLootFilter=current_sort_mode();m_sqaLootRenderedCount=m_pDeadBodyBagList->ItemsCount();
    string64 amount;xr_sprintf(amount,"%.2f kg",weight);m_PartnerWeight->SetText(amount);
    // Independent windows stay above the source tabs, including after a source refresh.
    auto& children=GetChildWndList();
    for(const auto& window:m_sqaContainers){auto it=std::find(children.begin(),children.end(),window.frame);if(it!=children.end()){children.erase(it);children.push_back(window.frame);}}
    m_sqaLootSources=std::move(sources);
}
bool CUIActorMenu::SqaLootClick(float x,float y)
{
    if(!SqaLootActive()||CUIDragDropListEx::m_drag_item)return false;
    for(const auto& window:m_sqaContainers){const auto p=window.frame->GetWndPos();const auto size=window.frame->GetWndSize();if(window.frame->IsShown()&&x>=p.x&&x<p.x+size.x&&y>=p.y&&y<p.y+size.y)return false;}
    for(size_t n=0;n<m_sqaLootTabs.size();++n){auto tab=m_sqaLootTabs[n];const auto p=tab->GetWndPos();const auto size=tab->GetWndSize();
        if(x>=p.x&&x<p.x+size.x&&y>=p.y&&y<p.y+size.y){m_sqaLootSource=m_sqaLootSources[n].live;m_sqaLootSourceIdentity=m_sqaLootSources[n].identity;SqaUpdateLoot(true);return true;}}
    return false;
}
bool CUIActorMenu::SqaTakeLoot(PIItem item,u16 slot)
{
    if(!item)return false;const auto found=m_sqaLootIdentity.find(item->object_id());if(found==m_sqaLootIdentity.end())return false;
    const bool result=InventoryNextLootTake(m_sqaLootSource,m_sqaLootSourceIdentity,found->first,found->second,slot);
    m_sqaLootAfter=0;m_sqaRefresh=true;return result;
}
bool CUIActorMenu::SqaPutLoot(PIItem item)
{
    if(!item)return false;
    const bool result=InventoryNextLootPut(m_sqaLootSource,m_sqaLootSourceIdentity,item->object_id());
    m_sqaLootAfter=0;m_sqaRefresh=true;return result;
}

void CUIActorMenu::SqaForgetLoot(u16 live)
{
    if(!m_sqaLootIdentity.count(live))return;
    if(CUIDragDropListEx::m_drag_item&&CUIDragDropListEx::m_drag_item->ParentItem()->OwnerList()==m_pDeadBodyBagList)m_pDeadBodyBagList->DestroyDragItem();
    if(CurrentItem()&&CurrentItem()->OwnerList()==m_pDeadBodyBagList){SetCurrentItem(nullptr);InfoCurItem(nullptr);m_UIPropertiesBox->Hide();}
    m_pDeadBodyBagList->ClearAll(true);m_sqaLootIdentity.clear();m_sqaLootAfter=0;
}

bool CUIActorMenu::SqaMoveWindow(float x,float y,EUIMessages action)
{
    if(action==WINDOW_LBUTTON_UP){const bool moved=m_sqaMovingWindow!=0;m_sqaMovingWindow=0;return moved;}
    if(CUIDragDropListEx::m_drag_item){m_sqaMovingWindow=0;return false;}
    if(m_sqaMovingWindow&&action==WINDOW_MOUSE_MOVE){
        for(auto& window:m_sqaContainers)if(window.owner==m_sqaMovingWindow){
            float left=x-m_sqaWindowOffset.x,top=y-m_sqaWindowOffset.y;
            clamp(left,0.0f,1024.0f-window.frame->GetWndSize().x);clamp(top,0.0f,768.0f-window.frame->GetWndSize().y);
            window.frame->SetWndPos(Fvector2().set(left,top));return true;
        }
        m_sqaMovingWindow=0;
    }
    if(action==WINDOW_LBUTTON_DOWN)for(size_t n=m_sqaContainers.size();n>0;--n){
        auto& window=m_sqaContainers[n-1];const auto p=window.frame->GetWndPos();
        if(window.frame->IsShown()&&x>=p.x&&x<p.x+window.frame->GetWndSize().x-30.0f&&y>=p.y&&y<p.y+28){
            m_sqaMovingWindow=window.owner;m_sqaWindowOffset.set(x-p.x,y-p.y);return true;
        }
    }
    return false;
}
