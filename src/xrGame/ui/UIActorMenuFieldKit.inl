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
