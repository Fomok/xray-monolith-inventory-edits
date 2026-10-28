// Included in UIActorMenu.cpp: presentation owns windows, never inventory state.
CUIDragDropListEx* CUIActorMenu::SqaContainerList(u64 owner)
{
    for(auto& window:m_sqaContainers)if(window.owner==owner)return window.list;
    return nullptr;
}
bool CUIActorMenu::SqaActorList(CUIDragDropListEx* list) const
{
    if(list==m_pInventoryBagList)return true;
    if(InventoryNextFieldKitEnabled())for(const auto& window:m_sqaContainers)if(window.list==list)return true;
    return false;
}
bool CUIActorMenu::SqaOpenContainer(PIItem item)
{
    if(!item||CUIDragDropListEx::m_drag_item)return false;
    const auto owner=InventoryNextContainer(item->object_id());if(!owner)return false;
    if(SqaContainerList(owner))return true;
    // Bound presentation allocations independently of the number of owned bags.
    if(m_sqaContainers.size()>=4)return true;
    SqaContainerWindow window;window.owner=owner;window.live=item->object_id();
    window.frame=xr_new<CUIStatic>();window.frame->SetAutoDelete(true);AttachChild(window.frame);
    const float offset=float(m_sqaContainers.size())*28.0f;
    window.frame->SetWndPos(Fvector2().set(340+offset,140+offset));window.frame->SetWndSize(Fvector2().set(290,380));
    window.frame->InitTexture("sqa_fk_window");window.frame->SetStretchTexture(true);
    auto title=xr_new<CUIStatic>();title->SetAutoDelete(true);window.frame->AttachChild(title);title->Enable(false);
    title->SetWndPos(Fvector2().set(10,5));title->SetWndSize(Fvector2().set(246,20));title->TextureOff();
    title->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);title->TextItemControl()->SetText(item->NameItem());
    auto close=xr_new<CUIStatic>();close->SetAutoDelete(true);window.frame->AttachChild(close);close->Enable(false);
    close->SetWndPos(Fvector2().set(264,4));close->SetWndSize(Fvector2().set(20,20));close->InitTexture("sqa_fk_close");close->SetStretchTexture(true);
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
            InventoryNextStorageView(it->list,it->owner);
            it->frame->Show(m_currMenuMode==mmInventory||m_currMenuMode==mmDeadBodySearch);++it;
        }
    }
}
bool CUIActorMenu::SqaCloseContainerAt(float x,float y)
{
    if(CUIDragDropListEx::m_drag_item)return false;
    for(size_t n=m_sqaContainers.size();n>0;--n) {
        auto& window=m_sqaContainers[n-1];const auto position=window.frame->GetWndPos();
        if(window.frame->IsShown()&&x>=position.x+264&&x<position.x+284&&y>=position.y+4&&y<position.y+24) {
            SetCurrentItem(nullptr);window.list->ClearAll(true);DetachChild(window.frame);
            m_sqaContainers.erase(m_sqaContainers.begin()+n-1);m_sqaRefresh=true;return true;
        }
    }
    return false;
}

void CUIActorMenu::SqaUpdateChest()
{
    if(!InventoryNextFieldKitEnabled())return;
    if(m_pInvList[OUTFIT_SLOT])m_pInvList[OUTFIT_SLOT]->Show(!m_sqaRigView);
    if(m_pInvList[16])m_pInvList[16]->Show(m_sqaRigView);
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
    return InventoryNextFieldKitEnabled()&&(m_currMenuMode==mmInventory||m_currMenuMode==mmDeadBodySearch);
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
    if(unchanged)for(size_t n=0;n<sources.size();++n)if(sources[n].live!=m_sqaLootSources[n].live||sources[n].identity!=m_sqaLootSources[n].identity||sources[n].items!=m_sqaLootSources[n].items||sources[n].grams!=m_sqaLootSources[n].grams){unchanged=false;break;}
    if(!force&&unchanged&&!sources.empty()&&selected->live==m_sqaLootSource&&selected->identity==m_sqaLootSourceIdentity&&m_pDeadBodyBagList->ItemsCount()==selected->items.size())return;
    SetCurrentItem(nullptr);InfoCurItem(nullptr);m_pDeadBodyBagList->ClearAll(true);m_sqaLootIdentity.clear();
    for(auto tab:m_sqaLootTabs)DetachChild(tab);m_sqaLootTabs.clear();
    const bool available=!sources.empty();
    m_LeftBackground->Show(available);m_pDeadBodyBagList->Show(available);
    m_takeall_button->Show(available);m_putall_button->Show(available&&selected->live!=0xffff);
    m_PartnerCharacterInfo->Show(false);m_PartnerBottomInfo->Show(available);m_PartnerWeight->Show(available);
    if(!available){m_sqaLootSources.clear();return;}
    m_sqaLootSource=selected->live;m_sqaLootSourceIdentity=selected->identity;
    const float tabHeight=float((sources.size()+2)/3)*22.0f;
    m_pDeadBodyBagList->SetWndPos(Fvector2().set(680.0f,114.0f+tabHeight));
    m_pDeadBodyBagList->SetWndSize(Fvector2().set(306.0f,_max(100.0f,574.0f-tabHeight)));
    for(size_t n=0;n<sources.size();++n){
        auto tab=xr_new<CUIStatic>();tab->SetAutoDelete(true);AttachChild(tab);tab->Enable(false);
        tab->SetWndPos(Fvector2().set(652+float(n%3)*120,90+float(n/3)*22));tab->SetWndSize(Fvector2().set(116,20));
        tab->InitTexture(sources[n].live==selected->live?"sqa_fk_surface_olive":"sqa_fk_surface_rubber");tab->SetStretchTexture(true);
        tab->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);tab->TextItemControl()->SetEllipsis(true);tab->TextItemControl()->SetText(sources[n].name.c_str());
        m_sqaLootTabs.push_back(tab);
    }
    const double weight=double(selected->grams)/1000.0;
    for(auto live:selected->items){auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));if(!item)continue;
        m_sqaLootIdentity[live]=InventoryNextIdentity(live);
        m_pDeadBodyBagList->SetItem(create_cell_item(item));
    }
    string64 amount;xr_sprintf(amount,"%.2f kg",weight);m_PartnerWeight->SetText(amount);
    m_sqaLootSources=std::move(sources);
}
bool CUIActorMenu::SqaLootClick(float x,float y)
{
    if(!SqaLootActive()||CUIDragDropListEx::m_drag_item)return false;
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
    SetCurrentItem(nullptr);InfoCurItem(nullptr);m_pDeadBodyBagList->ClearAll(true);m_sqaLootIdentity.clear();m_sqaLootAfter=0;
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
        if(window.frame->IsShown()&&x>=p.x&&x<p.x+260&&y>=p.y&&y<p.y+28){
            m_sqaMovingWindow=window.owner;m_sqaWindowOffset.set(x-p.x,y-p.y);return true;
        }
    }
    return false;
}
