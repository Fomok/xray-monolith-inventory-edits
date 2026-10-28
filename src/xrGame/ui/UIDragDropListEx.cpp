#include "stdafx.h"
#include "UIDragDropListEx.h"
#include "UIStatic.h"
#include "UIFrameWindow.h"
#include "UITextureMaster.h"
#include "UIFieldKitButton.h"
#include "UIScrollBar.h"
#include "object_broker.h"
#include "UICellItem.h"
#include "UICursor.h"
#include "../Level.h"
//Alundaio
#include "../Inventory.h"
#include "../InventoryNextIdentity.h"
#include "../InventoryNextProbe.h"
#include <dinput.h>
//-Alundaio

CUIDragItem* CUIDragDropListEx::m_drag_item = NULL;

void CUICell::Clear()
{
	m_bMainItem = false;
	if (m_item) m_item->SetOwnerList(NULL);
	m_item = NULL;
}

CUIDragDropListEx::CUIDragDropListEx()
{
	m_flags.zero();
	m_container = xr_new<CUICellContainer>(this);
	m_vScrollBar = xr_new<CUIScrollBar>();
	m_vScrollBar->SetAutoDelete(true);
	m_selected_item = NULL;
	m_bConditionProgBarVisible = false;

	SetCellSize(Ivector2().set(50, 50));
	SetCellsCapacity(Ivector2().set(0, 0));

	AttachChild(m_container);
	AttachChild(m_vScrollBar);

	m_vScrollBar->SetWindowName("scroll_v");
	Register(m_vScrollBar);
	AddCallbackStr("scroll_v", SCROLLBAR_VSCROLL, CUIWndCallback::void_function(this, &CUIDragDropListEx::OnScrollV));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_DRAG,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemStartDragging));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_DROP,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemDrop));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_SELECTED,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemSelected));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_LBUTTON_CLICK,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemLButtonClick));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_RBUTTON_CLICK,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemRButtonClick));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_DB_CLICK,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemDBClick));
	AddCallbackStr("cell_item", DRAG_DROP_ITEM_FOCUSED_UPDATE,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemFocusedUpdate));
	AddCallbackStr("cell_item", WINDOW_FOCUS_RECEIVED,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemFocusReceived));
	AddCallbackStr("cell_item", WINDOW_FOCUS_LOST,
	               CUIWndCallback::void_function(this, &CUIDragDropListEx::OnItemFocusLost));

	back_color = 0xFFFFFFFF;
}

CUIDragDropListEx::~CUIDragDropListEx()
{
	DestroyDragItem();

	delete_data(m_container);
}

void CUIDragDropListEx::SetAutoGrow(bool b)
{
	m_flags.set(flAutoGrow, b);
}

bool CUIDragDropListEx::IsAutoGrow()
{
	return !!m_flags.test(flAutoGrow);
}

void CUIDragDropListEx::SetGrouping(bool b)
{
	m_flags.set(flGroupSimilar, b);
}

bool CUIDragDropListEx::IsGrouping()
{
	return !!m_flags.test(flGroupSimilar);
}

void CUIDragDropListEx::SetCustomPlacement(bool b)
{
	m_flags.set(flCustomPlacement, b);
}

bool CUIDragDropListEx::GetCustomPlacement()
{
	return !!m_flags.test(flCustomPlacement);
}

void CUIDragDropListEx::SetVerticalPlacement(bool b)
{
	m_flags.set(flVerticalPlacement, b);
}

void CUIDragDropListEx::SetAlwaysShowScroll(bool b)
{
	m_flags.set(flAlwaysShowScroll, b);
}

bool CUIDragDropListEx::GetVerticalPlacement()
{
	return !!m_flags.test(flVerticalPlacement);
}

void CUIDragDropListEx::SetVirtualCells(bool b)
{
	m_flags.set(flVirtualCells, b);
}

bool CUIDragDropListEx::GetVirtualCells()
{
	return !!m_flags.test(flVirtualCells);
}

void CUIDragDropListEx::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIDragDropListEx::InitDragDropList(Fvector2 pos, Fvector2 size)
{
	inherited::SetWndPos(pos);
	inherited::SetWndSize(size);
	m_vScrollBar->InitScrollBar(Fvector2().set(size.x, 0.0f), size.y, false);
	m_vScrollBar->SetWndPos(Fvector2().set(m_vScrollBar->GetWndPos().x - m_vScrollBar->GetWidth(),
	                                       m_vScrollBar->GetWndPos().y));
}

void CUIDragDropListEx::OnScrollV(CUIWindow* w, void* pData)
{
	m_container->SetWndPos(Fvector2().set(m_container->GetWndPos().x, float(-m_vScrollBar->GetScrollPos())));
}

void CUIDragDropListEx::CreateDragItem(CUICellItem* itm)
{
	R_ASSERT(!m_drag_item);
	m_drag_item = itm->CreateDragItem();

	if (m_drag_item)
	{
		GetParent()->SetCapture(m_drag_item, true);
	}
}

void CUIDragDropListEx::DestroyDragItem()
{
	if (m_selected_item && m_drag_item && m_drag_item->ParentItem() == m_selected_item)
	{
		VERIFY(m_drag_item->IsMouseCapturer());
		GetParent()->SetCapture(m_drag_item, false);

		delete_data(m_drag_item);
	}
}

Fvector2 CUIDragDropListEx::GetDragItemPosition()
{
	return GetUICursor().GetCursorPosition(); //return m_drag_item->GetPosition();
}


void CUIDragDropListEx::OnDragEvent(CUIDragItem* drag_item, bool b_receive)
{
	if (m_f_drag_event)
		m_f_drag_event(drag_item, b_receive);
}

void CUIDragDropListEx::OnItemStartDragging(CUIWindow* w, void* pData)
{
	OnItemSelected(w, pData);
	CUICellItem* itm = smart_cast<CUICellItem*>(w);

	if (itm != m_selected_item) return;

	if (m_f_item_start_drag && m_f_item_start_drag(itm)) return;

	CreateDragItem(itm);
}

void CUIDragDropListEx::OnItemDrop(CUIWindow* w, void* pData)
{
	OnItemSelected(w, pData);
	CUICellItem* itm = smart_cast<CUICellItem*>(w);
	VERIFY(itm->OwnerList() == itm->OwnerList());

	if (m_f_item_drop && m_f_item_drop(itm))
	{
		DestroyDragItem();
		return;
	}

	CUIDragDropListEx* old_owner = itm->OwnerList();
	CUIDragDropListEx* new_owner = m_drag_item->BackList();

	bool b = (old_owner == new_owner) && !GetCustomPlacement();

	if (old_owner && new_owner && !b)
	{
		CUICellItem* i = old_owner->RemoveItem(itm, (old_owner == new_owner));
		while (i->ChildsCount())
		{
			CUICellItem* _chld = i->PopChild(NULL);
			new_owner->SetItem(_chld, old_owner->GetDragItemPosition());
		}
		new_owner->SetItem(i, old_owner->GetDragItemPosition());
	}
	DestroyDragItem();
}

void CUIDragDropListEx::OnItemDBClick(CUIWindow* w, void* pData)
{
	OnItemSelected(w, pData);
	CUICellItem* itm = smart_cast<CUICellItem*>(w);

	if (m_f_item_db_click)
	{
		if (Level().IR_GetKeyState(DIK_LCONTROL))
		{
			u32 size = itm->ChildsCount();
			for (u32 j = 0; j < size; j++)
				m_f_item_db_click(itm);
		}

		if (m_f_item_db_click(itm))
		{
			DestroyDragItem();
			return;
		}
	}

	CUIDragDropListEx* old_owner = itm->OwnerList();
	VERIFY(m_drag_item==NULL);
	VERIFY(old_owner == this);

	if (old_owner && old_owner->GetCustomPlacement())
	{
		CUICellItem* i = old_owner->RemoveItem(itm, true);
		old_owner->SetItem(i);
	}

	DestroyDragItem();
}

void CUIDragDropListEx::OnItemSelected(CUIWindow* w, void* pData)
{
	m_selected_item = smart_cast<CUICellItem*>(w);
	VERIFY(m_selected_item);
	if (m_f_item_selected)
		m_f_item_selected(m_selected_item);
}

void CUIDragDropListEx::OnItemFocusReceived(CUIWindow* w, void* pData)
{
	if (m_f_item_focus_received)
	{
		CUICellItem* itm = smart_cast<CUICellItem*>(w);
		m_f_item_focus_received(itm);
	}
}

void CUIDragDropListEx::OnItemFocusLost(CUIWindow* w, void* pData)
{
	if (m_f_item_focus_lost)
	{
		CUICellItem* itm = smart_cast<CUICellItem*>(w);
		m_f_item_focus_lost(itm);
	}
}

void CUIDragDropListEx::OnItemFocusedUpdate(CUIWindow* w, void* pData)
{
	if (m_f_item_focused_update)
	{
		CUICellItem* itm = smart_cast<CUICellItem*>(w);
		m_f_item_focused_update(itm);
	}
}

void CUIDragDropListEx::OnItemRButtonClick(CUIWindow* w, void* pData)
{
	//*	OnItemSelected						(w, pData); // instead call function "SetCurrentItem(itm)";
	CUICellItem* itm = smart_cast<CUICellItem*>(w);
	if (m_f_item_rbutton_click)
		m_f_item_rbutton_click(itm);
}

void CUIDragDropListEx::OnItemLButtonClick(CUIWindow* w, void* pData)
{
	//*	OnItemSelected						(w, pData); // instead call function "SetCurrentItem(itm)";
	CUICellItem* itm = smart_cast<CUICellItem*>(w);
	if (m_f_item_lbutton_click)
		m_f_item_lbutton_click(itm);
}

void CUIDragDropListEx::GetClientArea(Frect& r)
{
	GetAbsoluteRect(r);
	if (m_vScrollBar->GetVisible() || m_flags.test(flAlwaysShowScroll))
		r.x2 -= m_vScrollBar->GetWidth();
}

void CUIDragDropListEx::ClearAll(bool bDestroy)
{
    for(auto& header:m_storageHeaders) DetachChild(header.second); // auto-delete
    m_storageHeaders.clear();m_storageHeaderOwners.clear();m_storageHeaderWeights.clear();m_storageHeaderSeen.clear();
    sqaProjectionExtraX=sqaProjectionExtraY=0;
	DestroyDragItem();
	m_container->ClearAll(bDestroy);
	m_selected_item = NULL;
	m_container->SetWndPos(Fvector2().set(0, 0));
	ResetCellsCapacity();
}

void CUIDragDropListEx::Compact()
{
	CUIWindow::WINDOW_LIST wl = m_container->GetChildWndList();
	ClearAll(false);

	CUIWindow::WINDOW_LIST_it it = wl.begin();
	CUIWindow::WINDOW_LIST_it it_e = wl.end();
	for (; it != it_e; ++it)
	{
		CUICellItem* itm = smart_cast<CUICellItem*>(*it);
		SetItem(itm);
	}
}

void CUIDragDropListEx::CenterStorageContent()
{
    const float contentWidth=_max(1.0f,GetWndSize().x-m_vScrollBar->GetWidth()-6.0f);
    const float gridWidth=float(CellsCapacity().x*CellSize().x+_max(0,CellsCapacity().x-1)*CellsSpacing().x+sqaProjectionExtraX);
    m_sqaStorageInset=_max(0.0f,(contentWidth-gridWidth)*0.5f);
    m_container->SetWndPos(Fvector2().set(m_sqaStorageInset,m_container->GetWndPos().y));
}
int CUIDragDropListEx::SqaGridContentWidth()
{
    return _max(1,int(GetWndSize().x-m_vScrollBar->GetWidth()-6.0f));
}
void CUIDragDropListEx::AddStorageHeader(int row,LPCSTR text,u32 color,u64 owner,s64 grams,bool collapsed)
{
    for(size_t index=0;index<m_storageHeaders.size();++index)
        if(m_storageHeaders[index].first==row&&m_storageHeaderOwners[index]==owner){
            m_storageHeaderSeen[index]=1;
            if(auto weight=m_storageHeaderWeights[index]){
                string64 amount;xr_sprintf(amount,"%.2f kg",double(grams)/1000.0);
                weight->TextItemControl()->SetText(amount);
            }
            return;
        }
    auto label=xr_new<CUIStatic>();label->SetAutoDelete(true);AttachChild(label);
    CUIStatic* weightLabel=nullptr;
    label->SetWndSize(Fvector2().set(GetWndSize().x,18.0f));label->TextureOff();label->Enable(false);
    label->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);
    label->TextItemControl()->SetEllipsis(true);
    label->TextItemControl()->SetTextColor(color);label->TextItemControl()->SetText(text);
    if(InventoryNextFieldKitEnabled()) {
        const float contentWidth=_max(1.0f,GetWndSize().x-m_vScrollBar->GetWidth()-6.0f);
        label->InitTexture("sqa3v1_section_bar");label->TextureOn();label->SetStretchTexture(true);
        label->SetWndSize(Fvector2().set(contentWidth,22.0f));
        label->TextItemControl()->SetTextAlignment(CGameFont::alCenter);
        label->TextItemControl()->SetText("");
        auto title=xr_new<CUIStatic>();title->SetAutoDelete(true);label->AttachChild(title);title->Enable(false);title->TextureOff();
        title->SetWndPos(Fvector2().set(78,2));title->SetWndSize(Fvector2().set(contentWidth-156.0f,18.0f));
        title->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);
        title->TextItemControl()->SetTextAlignment(CGameFont::alCenter);title->TextItemControl()->SetEllipsis(true);
        title->TextItemControl()->SetTextColor(color);title->TextItemControl()->SetText(text);
        auto weight=xr_new<CUIStatic>();weight->SetAutoDelete(true);label->AttachChild(weight);weight->Enable(false);
        weightLabel=weight;
        weight->SetWndPos(Fvector2().set(contentWidth-78.0f,2.0f));weight->SetWndSize(Fvector2().set(74,18));weight->TextureOff();
        weight->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);
        weight->TextItemControl()->SetTextAlignment(CGameFont::alRight);weight->TextItemControl()->SetTextColor(color);
        string64 amount;xr_sprintf(amount,"%.2f kg",double(grams)/1000.0);weight->TextItemControl()->SetText(amount);
        if(owner && !sqaViewOwner) {
            label->Enable(true);
            auto button=xr_new<CUIFieldKitButton>([owner](){InventoryNextStorageToggle(owner);});button->SetAutoDelete(true);label->AttachChild(button);
            button->InitButton(Fvector2().set(2,2),Fvector2().set(18,18));button->InitTexture(collapsed?"sqa3v1_arrow_down":"sqa3v1_arrow_up");
        }
    }
    m_storageHeaderOwners.push_back(owner);
    m_storageHeaders.emplace_back(row,label);
    m_storageHeaderWeights.push_back(weightLabel);
    m_storageHeaderSeen.push_back(1);
}
void CUIDragDropListEx::SqaBeginStorageHeaderUpdate()
{
    m_storageHeaderSeen.assign(m_storageHeaders.size(),0);
}
void CUIDragDropListEx::SqaEndStorageHeaderUpdate()
{
    for(size_t index=m_storageHeaders.size();index>0;--index)if(!m_storageHeaderSeen[index-1]){
        DetachChild(m_storageHeaders[index-1].second);
        m_storageHeaders.erase(m_storageHeaders.begin()+index-1);
        m_storageHeaderOwners.erase(m_storageHeaderOwners.begin()+index-1);
        m_storageHeaderWeights.erase(m_storageHeaderWeights.begin()+index-1);
        m_storageHeaderSeen.erase(m_storageHeaderSeen.begin()+index-1);
    }
}
Ivector2 CUIDragDropListEx::SqaCellPosition(CUICellItem* item)
{
    return m_container->GetItemPos(item);
}
void CUIDragDropListEx::SqaRepositionProjectedItems()
{
    if(!HasStorageHeaders())return;
    const int pitch_x=CellSize().x+CellsSpacing().x,pitch_y=CellSize().y+CellsSpacing().y;
    for(u32 index=0;index<ItemsCount();++index){
        auto item=GetItemIdx(index);const auto cell=SqaCellPosition(item);
        float dx=0.0f,dy=0.0f;
        if(!InventoryNextStoragePixelOffset(cell.x,cell.y,sqaViewOwner,pitch_x,dx,dy))continue;
        item->SetWndPos(Fvector2().set(float(cell.x*pitch_x)+dx,float(cell.y*pitch_y)+dy));
    }
}
void CUIDragDropListEx::SqaRestoreScrollPos(int position)
{
    m_vScrollBar->SetScrollPos(position);
    OnScrollV(nullptr,nullptr);
}
void CUIDragDropListEx::Draw()
{
    for(auto& header:m_storageHeaders) {
        const float y=float(header.first*(CellSize().y+CellsSpacing().y)-ScrollPos());
        header.second->SetWndPos(Fvector2().set(2.0f,y));
        header.second->Show(y>=0&&y+header.second->GetWndSize().y<=GetWndSize().y);
    }
	inherited::Draw();

	if (0 && bDebug)
	{
		CGameFont* F = UI().Font().pFontDI;
		F->SetAligment(CGameFont::alCenter);
		F->SetHeightI(0.02f);
		F->OutSetI(0.f, -0.5f);
		F->SetColor(0xffffffff);
		Ivector2 pt = m_container->PickCell(GetUICursor().GetCursorPosition());
		F->OutNext("%d-%d", pt.x, pt.y);
	};
}

void CUIDragDropListEx::Update()
{
	inherited::Update();

	if (m_drag_item)
	{
		Frect wndRect;
		GetAbsoluteRect(wndRect);
		Fvector2 cp = GetUICursor().GetCursorPosition();
		if (wndRect.in(cp))
		{
			if (NULL == m_drag_item->BackList())
				m_drag_item->SetBackList(this);
		}
		else if (this == m_drag_item->BackList())
			m_drag_item->SetBackList(NULL);
	}
}

void CUIDragDropListEx::ReinitScroll()
{
	float h1 = m_container->GetWndSize().y;
	float h2 = GetWndSize().y;
	VERIFY(_valid(h1));
	VERIFY(_valid(h2));
	float dh = h1 - h2;
	m_vScrollBar->Show((dh > 0) || m_flags.test(flAlwaysShowScroll));
	m_vScrollBar->Enable((dh > 0) || m_flags.test(flAlwaysShowScroll));

	if (dh < 0)
	{
		//			dh = 0;
		m_vScrollBar->SetRange(0, 0);
	}
	else
	{
		m_vScrollBar->SetRange(0, iFloor(dh));
	}
	m_vScrollBar->SetScrollPos(0);
	m_vScrollBar->SetStepSize(CellSize().y / 3);
	//		m_vScrollBar->SetPageSize	(iFloor(GetWndSize().y/float(CellSize().y)));
	m_vScrollBar->SetPageSize(1/*CellSize().y*/);

	m_container->SetWndPos(Fvector2().set(m_sqaStorageInset, 0.0f));
}

bool CUIDragDropListEx::OnMouseAction(float x, float y, EUIMessages mouse_action)
{

	bool b = inherited::OnMouseAction(x, y, mouse_action);

	if (m_vScrollBar->IsShown())
	{
		switch (mouse_action)
		{
		case WINDOW_MOUSE_WHEEL_DOWN:
			for (u8 i = 0; i < 4; ++i) { m_vScrollBar->TryScrollInc(); }
			return true;
			break;

		case WINDOW_MOUSE_WHEEL_UP:
			for (u8 i = 0; i < 4; ++i) { m_vScrollBar->TryScrollDec(); }
			return true;
			break;
		}
	}
	return b;
}

const Ivector2& CUIDragDropListEx::CellsCapacity()
{
	return m_container->CellsCapacity();
}

void CUIDragDropListEx::SetCellsCapacity(const Ivector2 c)
{
	m_container->SetCellsCapacity(c);
}

const Ivector2& CUIDragDropListEx::CellSize()
{
	return m_container->CellSize();
}

const Ivector2& CUIDragDropListEx::CellsSpacing()
{
	return m_container->CellsSpacing();
}

void CUIDragDropListEx::SetCellSize(const Ivector2 new_sz)
{
	m_container->SetCellSize(new_sz);
}

void CUIDragDropListEx::SetCellsSpacing(const Ivector2& new_sz)
{
	m_container->SetCellsSpacing(new_sz);
}

int CUIDragDropListEx::ScrollPos()
{
	return m_vScrollBar->GetScrollPos();
}
void CUIDragDropListEx::RevealItem(CUICellItem* item)
{
    if(!item||item->OwnerList()!=this)return;
    m_vScrollBar->SetScrollPos(_max(0,int(item->GetWndPos().y-GetWndSize().y*0.25f)));
    OnScrollV(nullptr,nullptr);
}

void CUIDragDropListEx::SetItem(CUICellItem* itm) //auto
{
	if (m_container->AddSimilar(itm))
	{
		return;
	}

	Ivector2 dest_cell_pos = m_container->FindFreeCell(itm->GetGridSize());

	SetItem(itm, dest_cell_pos);
}

void CUIDragDropListEx::SetItem(CUICellItem* itm, Fvector2 abs_pos) // start at cursor pos
{
	if (m_container->AddSimilar(itm)) return;

	const Ivector2 dest_cell_pos = m_container->PickCell(abs_pos);

	if (m_container->ValidCell(dest_cell_pos) && m_container->IsRoomFree(dest_cell_pos, itm->GetGridSize()))
		SetItem(itm, dest_cell_pos);
	else
		SetItem(itm);
}

void CUIDragDropListEx::SetItem(CUICellItem* itm, Ivector2 cell_pos) // start at cell
{
	if (m_container->AddSimilar(itm)) return;
	R_ASSERT(m_container->IsRoomFree(cell_pos, itm->GetGridSize()));

	m_container->PlaceItemAtPos(itm, cell_pos);

	itm->SetWindowName("cell_item");
	Register(itm);
	itm->SetOwnerList(this);
}

bool CUIDragDropListEx::CanSetItem(CUICellItem* itm)
{
	if (m_container->HasFreeSpace(itm->GetGridSize()))
		return true;
	Compact();

	return m_container->HasFreeSpace(itm->GetGridSize());
}

bool CUIDragDropListEx::CanSetItemAt(CUICellItem* item,const Ivector2& cell)
{
	return m_container->IsRoomFree(cell,item->GetGridSize());
}

CUICellItem* CUIDragDropListEx::RemoveItem(CUICellItem* itm, bool force_root)
{
	CUICellItem* i = m_container->RemoveItem(itm, force_root);
	i->SetOwnerList((CUIDragDropListEx*)NULL);
	return i;
}

u32 CUIDragDropListEx::ItemsCount()
{
	return m_container->GetChildWndList().size();
}

bool CUIDragDropListEx::IsOwner(CUICellItem* itm)
{
	return m_container->IsChild(itm);
}

CUICellItem* CUIDragDropListEx::GetItemIdx(u32 idx)
{
	R_ASSERT(idx<ItemsCount());
	WINDOW_LIST_it it = m_container->GetChildWndList().begin();
	std::advance(it, idx);
	return smart_cast<CUICellItem*>(*it);
}

void CUIDragDropListEx::clear_select_armament()
{
	m_container->clear_select_armament();
}

void CUIDragDropListEx::SetCellsVertAlignment(xr_string alignment)
{
	if (strchr(alignment.c_str(), 't'))
	{
		m_virtual_cells_alignment.y = 0;
		return;
	}
	if (strchr(alignment.c_str(), 'b'))
	{
		m_virtual_cells_alignment.y = 2;
		return;
	}
	m_virtual_cells_alignment.y = 1;
}

void CUIDragDropListEx::SetCellsHorizAlignment(xr_string alignment)
{
	if (strchr(alignment.c_str(), 'l'))
	{
		m_virtual_cells_alignment.x = 0;
		return;
	}
	if (strchr(alignment.c_str(), 'r'))
	{
		m_virtual_cells_alignment.x = 2;
		return;
	}
	m_virtual_cells_alignment.x = 1;
}

Ivector2 CUIDragDropListEx::PickCell(const Fvector2& abs_pos)
{
	return m_container->PickCell(abs_pos);
};

CUICell& CUIDragDropListEx::GetCellAt(const Ivector2& pos)
{
	return m_container->GetCellAt(pos);
};
// =================================================================================================

CUICellContainer::CUICellContainer(CUIDragDropListEx* parent)
{
	m_pParentDragDropList = parent;
	hShader->create("hud\\fog_of_war", "ui\\ui_grid");
	if (InventoryNextFieldKitEnabled()) CUITextureMaster::InitTexture("sqa3v1_grid_line","hud\\fog_of_war",hSqaGrid,m_sqaGridRect);
    else if (InventoryNextEnabled()) {hSqaGrid->create("hud\\fog_of_war", "ui\\sqa_fk_controls");m_sqaGridRect.set(8,8,136,136);}
	//	hShader_selected->create	( "hud\\fog_of_war", "ui_grid_selected" );
	m_cellSpacing.set(0, 0);
}

CUICellContainer::~CUICellContainer()
{
}

bool CUICellContainer::AddSimilar(CUICellItem* itm)
{
	if (!m_pParentDragDropList->IsGrouping()) return false;

	//Alundaio: Don't stack equipped items
	PIItem iitem = (PIItem)itm->m_pData;
	if (iitem && iitem->m_pInventory)
	{
		if (iitem->m_pInventory->ItemFromSlot(iitem->BaseSlot()) == iitem)
			return false;

		if (pSettings->line_exist(iitem->m_section_id, "dont_stack") && pSettings->r_bool(
			iitem->m_section_id, "dont_stack") == TRUE)
			return false;
	}
	//-Alundaio

	CUICellItem* i = FindSimilar(itm);
	if (i == NULL || i == itm || itm->ChildsCount() > 0)
		return false;

	i->PushChild(itm);
	itm->SetOwnerList(m_pParentDragDropList);

	return true;
}

CUICellItem* CUICellContainer::FindSimilar(CUICellItem* itm)
{
	for (WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end() != it; ++it)
	{
#ifdef DEBUG
		CUICellItem* i = smart_cast<CUICellItem*>(*it);
#else
		CUICellItem* i = (CUICellItem*)(*it);
#endif
		//Alundaio: Don't stack equipped items
		PIItem iitem = (PIItem)i->m_pData;
		if (iitem && iitem->m_pInventory)
		{
			if (iitem->m_pInventory->ItemFromSlot(iitem->BaseSlot()) == iitem)
				continue;

			if (pSettings->line_exist(iitem->m_section_id, "dont_stack") && pSettings->r_bool(
				iitem->m_section_id, "dont_stack") == TRUE)
				continue;
		}
		//-Alundaio

		if (i == itm)
			continue;

		if (i->EqualTo(itm))
			return i;
	}
	return NULL;
}

void CUICellContainer::PlaceItemAtPos(CUICellItem* itm, Ivector2& cell_pos)
{
	Ivector2 cs = itm->GetGridSize();
	if (m_pParentDragDropList->GetVerticalPlacement())
		std::swap(cs.x, cs.y);

	for (int x = 0; x < cs.x; ++x)
	{
		for (int y = 0; y < cs.y; ++y)
		{
			CUICell& C = GetCellAt(Ivector2().set(x, y).add(cell_pos));
			C.SetItem(itm, (x == 0 && y == 0));
		}
	}
	if(InventoryNextFieldKitEnabled()&&m_pParentDragDropList->HasStorageHeaders())
        itm->SetWndSize(Fvector2().set(float(m_cellSize.x*cs.x+m_cellSpacing.x*(cs.x-1)),
            float(m_cellSize.y*cs.y+m_cellSpacing.y*(cs.y-1))));
    else itm->SetWndSize(Fvector2().set((m_cellSize.x * cs.x), (m_cellSize.y * cs.y)));
	if (!m_pParentDragDropList->GetVirtualCells()){
        float dx=0.0f,dy=0.0f;
        if(m_pParentDragDropList->HasStorageHeaders())
            InventoryNextStoragePixelOffset(cell_pos.x,cell_pos.y,m_pParentDragDropList->sqaViewOwner,
                m_cellSpacing.x+m_cellSize.x,dx,dy);
        itm->SetWndPos(Fvector2().set(float((m_cellSpacing.x+m_cellSize.x)*cell_pos.x)+dx,
            float((m_cellSpacing.y+m_cellSize.y)*cell_pos.y)+dy));
    }
	else
	{
		Ivector2 alignment_vec = m_pParentDragDropList->GetVirtualCellsAlignment();
		Fvector2 pos = Fvector2().set(0, 0);
		if (alignment_vec.x == 1)
			pos.x = (m_pParentDragDropList->GetWndSize().x - cs.x * (m_cellSpacing.x + m_cellSize.x)) / 2;
		else if (alignment_vec.x == 2)
			pos.x = m_pParentDragDropList->GetWndSize().x - cs.x * (m_cellSpacing.x + m_cellSize.x);

		if (alignment_vec.y == 1)
			pos.y = (m_pParentDragDropList->GetWndSize().y - cs.y * (m_cellSpacing.y + m_cellSize.y)) / 2;
		else if (alignment_vec.y == 2)
			pos.y = m_pParentDragDropList->GetWndSize().y - cs.y * (m_cellSpacing.y + m_cellSize.y);
		itm->SetWndPos(pos);
	}


	AttachChild(itm);
	itm->OnAfterChild(m_pParentDragDropList);
}

CUICellItem* CUICellContainer::RemoveItem(CUICellItem* itm, bool force_root)
{
	for (WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end() != it; ++it)
	{
		CUICellItem* i = (CUICellItem*)(*it);

		if (i->HasChild(itm))
		{
			CUICellItem* iii = i->PopChild(itm);
			R_ASSERT(0==iii->ChildsCount());
			return iii;
		}
	}

	if (!force_root && itm->ChildsCount())
	{
		CUICellItem* iii = itm->PopChild(NULL);
		R_ASSERT(0==iii->ChildsCount());
		return iii;
	}

	Ivector2 pos = GetItemPos(itm);
	Ivector2 cs = itm->GetGridSize();

	if (m_pParentDragDropList->GetVerticalPlacement())
		std::swap(cs.x, cs.y);

	for (int x = 0; x < cs.x; ++x)
		for (int y = 0; y < cs.y; ++y)
		{
			CUICell& C = GetCellAt(Ivector2().set(x, y).add(pos));
			C.Clear();
		}

	itm->SetOwnerList(NULL);
	DetachChild(itm);
	return itm;
}

Ivector2 CUICellContainer::FindFreeCell(const Ivector2& _size)
{
	Ivector2 tmp;
	Ivector2 size = _size;

	if (m_pParentDragDropList->GetVerticalPlacement())
		std::swap(size.x, size.y);

	for (tmp.y = 0; tmp.y <= m_cellsCapacity.y - size.y; ++tmp.y)
		for (tmp.x = 0; tmp.x <= m_cellsCapacity.x - size.x; ++tmp.x)
			if (IsRoomFree(tmp, _size))
				return tmp;

	if (m_pParentDragDropList->IsAutoGrow())
	{
		Grow();
		return FindFreeCell(size);
	}
	else
	{
		m_pParentDragDropList->Compact();
		for (tmp.y = 0; tmp.y <= m_cellsCapacity.y - size.y; ++tmp.y)
			for (tmp.x = 0; tmp.x <= m_cellsCapacity.x - size.x; ++tmp.x)
				if (IsRoomFree(tmp, _size))
					return tmp;

		R_ASSERT2(0, "there are no free room to place item");
	}
	return tmp;
}

bool CUICellContainer::HasFreeSpace(const Ivector2& _size)
{
	Ivector2 tmp;
	Ivector2 size = _size;

	if (m_pParentDragDropList->GetVerticalPlacement())
		std::swap(size.x, size.y);

	for (tmp.y = 0; tmp.y <= m_cellsCapacity.y - size.y; ++tmp.y)
		for (tmp.x = 0; tmp.x <= m_cellsCapacity.x - size.x; ++tmp.x)
			if (IsRoomFree(tmp, _size))
				return true;

	return false;
}

bool CUICellContainer::IsRoomFree(const Ivector2& pos, const Ivector2& _size)
{
	Ivector2 tmp;

	Ivector2 size = _size;
	if (m_pParentDragDropList->GetVerticalPlacement())
		std::swap(size.x, size.y);

	for (tmp.x = pos.x; tmp.x < pos.x + size.x; ++tmp.x)
		for (tmp.y = pos.y; tmp.y < pos.y + size.y; ++tmp.y)
		{
			if (!ValidCell(tmp)) return false;

			CUICell& C = GetCellAt(tmp);

			if (!C.Empty()) return false;
		}
	return true;
}

void CUICellContainer::GetTexUVLT(Fvector2& uv, u32 col, u32 row, u8 select_mode)
{
	switch (select_mode)
	{
	case 0: uv.set(0.00f, 0.0f);
		break;
	case 1: uv.set(0.25f, 0.0f);
		break;
	case 2: uv.set(0.50f, 0.0f);
		break;
	case 3: uv.set(0.75f, 0.0f);
		break;
	default: uv.set(0.00f, 0.0f);
		break;
	}
}


void CUICellContainer::SetCellsCapacity(const Ivector2& c)
{
	m_cellsCapacity = c;
	m_cells.resize(c.x * c.y);
	ReinitSize();
}

void CUICellContainer::SetCellSize(const Ivector2& new_sz)
{
	m_cellSize = new_sz;
	ReinitSize();
}

void CUICellContainer::SetCellsSpacing(const Ivector2& c)
{
	m_cellSpacing = c;
	ReinitSize();
}

Ivector2 CUICellContainer::TopVisibleCell()
{
	return Ivector2().set(0, iFloor(m_pParentDragDropList->ScrollPos() / float(CellSize().y + m_cellSpacing.y)));
}

CUICell& CUICellContainer::GetCellAt(const Ivector2& pos)
{
	R_ASSERT(ValidCell(pos));
	CUICell& c = m_cells[m_cellsCapacity.x * pos.y + pos.x];
	return c;
}

Ivector2 CUICellContainer::GetItemPos(CUICellItem* itm)
{
	for (int x = 0; x < m_cellsCapacity.x; ++x)
		for (int y = 0; y < m_cellsCapacity.y; ++y)
		{
			Ivector2 p;
			p.set(x, y);
			if (GetCellAt(p).m_item == itm)
				return p;
		}

	R_ASSERT(0);
	return Ivector2().set(-1, -1);
}

u32 CUICellContainer::GetCellsInRange(const Irect& rect, UI_CELLS_VEC& res)
{
	res.clear_not_free();
	for (int x = rect.x1; x <= rect.x2; ++x)
		for (int y = rect.y1; y <= rect.y2; ++y)
			res.push_back(GetCellAt(Ivector2().set(x, y)));

	std::unique(res.begin(), res.end());
	return res.size();
}

void CUICellContainer::ReinitSize()
{
	Ivector2 sz;
	sz.add(CellsSpacing(), CellSize());
	sz.mul(CellsCapacity());
	sz.sub(CellsSpacing());
	if(InventoryNextFieldKitEnabled()){
        sz.x+=m_pParentDragDropList->sqaProjectionExtraX;
        sz.y+=m_pParentDragDropList->sqaProjectionExtraY;
    }

	SetWndSize(Fvector2().set(sz.x, sz.y));
	m_pParentDragDropList->ReinitScroll();
}

void CUICellContainer::Grow()
{
	SetCellsCapacity(Ivector2().set(m_cellsCapacity.x, m_cellsCapacity.y + 1));
}

void CUICellContainer::Shrink()
{
}

bool CUICellContainer::ValidCell(const Ivector2& pos) const
{
	return !(pos.x < 0 || pos.y < 0 || pos.x >= m_cellsCapacity.x || pos.y >= m_cellsCapacity.y);
}

void CUICellContainer::ClearAll(bool bDestroy)
{
	{
		UI_CELLS_VEC_IT it = m_cells.begin();
		UI_CELLS_VEC_IT it_e = m_cells.end();
		for (; it != it_e; ++it)
			(*it).Clear();
	}
	while (!m_ChildWndList.empty())
	{
		CUIWindow* w = m_ChildWndList.back();
		CUICellItem* wc = smart_cast<CUICellItem*>(w);
		VERIFY(!wc->IsAutoDelete());
		DetachChild(wc);

		while (wc->ChildsCount())
		{
			CUICellItem* ci = wc->PopChild(NULL);
			R_ASSERT(ci->ChildsCount()==0);

			if (bDestroy)
				delete_data(ci);
		}

		if (bDestroy)
		{
			delete_data(wc);
		}
	}
}

Ivector2 CUICellContainer::PickCell(const Fvector2& abs_pos)
{
	Ivector2 res;
	if(m_cellsCapacity.x<1||m_cellsCapacity.y<1){res.set(-1,-1);return res;}
	Fvector2 ap;
	GetAbsolutePos(ap);
	ap.sub(abs_pos);
	ap.mul(-1);
	if(!InventoryNextFieldKitEnabled()){
	    res.y=iFloor(ap.y/(m_cellSize.y+m_cellSpacing.y*(m_cellsCapacity.y-1)/m_cellsCapacity.y));
	    res.x=iFloor(ap.x/(m_cellSize.x+m_cellSpacing.x*(m_cellsCapacity.x-1)/m_cellsCapacity.x));
	    if(!ValidCell(res))res.set(-1,-1);
	    return res;
	}
	if(m_pParentDragDropList->HasStorageHeaders()){
        Frect clip;m_pParentDragDropList->GetClientArea(clip);
        if(!clip.in(abs_pos.x,abs_pos.y)){res.set(-1,-1);return res;}
        if(InventoryNextStoragePixelHit(ap.x,ap.y,m_pParentDragDropList->sqaViewOwner,
            m_cellSize.x,m_cellSpacing.x,res.x,res.y))return res;
        res.set(-1,-1);return res;
    }
	const float pitch_y=float(m_cellSize.y+m_cellSpacing.y);
	const float pitch_x=float(m_cellSize.x+m_cellSpacing.x);
	res.y = iFloor(ap.y/pitch_y);
    const float inset=m_pParentDragDropList->HasStorageHeaders()?InventoryNextStorageInset(res.y,m_pParentDragDropList->sqaViewOwner)*float(m_cellSize.x+m_cellSpacing.x):0.0f;
	const float local_x=ap.x-inset;
    res.x=iFloor(local_x/pitch_x);
	if (!ValidCell(res)||local_x-float(res.x)*pitch_x>=float(m_cellSize.x)||
	    ap.y-float(res.y)*pitch_y>=float(m_cellSize.y))
		res.set(-1, -1);
	return res;
}


void CUICellContainer::Draw()
{
	Frect clientArea;
	m_pParentDragDropList->GetClientArea(clientArea);

	Ivector2 cell_cnt = m_pParentDragDropList->CellsCapacity();
	if (cell_cnt.x == 0 || cell_cnt.y == 0) return;

	Ivector2 cell_sz = CellSize();
	cell_sz.add(m_cellSpacing);

	Irect tgt_cells;
	tgt_cells.lt = TopVisibleCell();
    if(InventoryNextFieldKitEnabled()&&m_pParentDragDropList->HasStorageHeaders())
        tgt_cells.lt.y=_max(0,tgt_cells.lt.y-1);
	tgt_cells.x2 = iFloor((float(clientArea.width()) + float(cell_sz.x) - EPS) / float(cell_sz.x)) + tgt_cells.lt.x;
	tgt_cells.y2 = iFloor((float(clientArea.height()) + float(cell_sz.y) - EPS) / float(cell_sz.y)) + tgt_cells.lt.y;

	clamp(tgt_cells.x2, 0, cell_cnt.x - 1);
	clamp(tgt_cells.y2, 0, cell_cnt.y - 1);

	Fvector2 lt_abs_pos;
	GetAbsolutePos(lt_abs_pos);

	Fvector2 drawLT;
	if(InventoryNextFieldKitEnabled())drawLT.set(lt_abs_pos.x+tgt_cells.lt.x*cell_sz.x,
	    lt_abs_pos.y+tgt_cells.lt.y*cell_sz.y);
	else drawLT.set(lt_abs_pos.x + tgt_cells.lt.x * (cell_sz.x + m_cellSpacing.x),
	           lt_abs_pos.y + tgt_cells.lt.y * (cell_sz.y + m_cellSpacing.y));
	UI().ClientToScreenScaled(drawLT, drawLT.x, drawLT.y);

	const Fvector2 pts[6] = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
	};
#define ty 1.0f
#define tx 0.25f
	const Fvector2 uvs[6] = {
		{0.0f, 0.0f}, {tx, 0.0f}, {tx,ty},
		{0.0f, 0.0f}, {tx,ty}, {0.0f,ty}
	};

	// calculate cell size in screen pixels
	Fvector2 f_len, sp_len;
	UI().ClientToScreenScaled(f_len, float(CellSize().x), float(CellSize().y));
	UI().ClientToScreenScaled(sp_len, float(CellsSpacing().x), float(CellsSpacing().y));

	GetCellsInRange(tgt_cells, m_cells_to_draw);

    Ivector2 previewCell;previewCell.set(-1,-1);int previewWidth=0,previewHeight=0,preview=0;
    auto drag=CUIDragDropListEx::m_drag_item;
    if(InventoryNextFieldKitEnabled()&&m_pParentDragDropList->HasStorageHeaders()&&drag&&drag->BackList()==m_pParentDragDropList){
        previewCell=m_pParentDragDropList->PickCell(m_pParentDragDropList->GetDragItemPosition());
        auto item=static_cast<PIItem>(drag->ParentItem()->m_pData);
        if(item)preview=InventoryNextNativePreview(item->object_id(),previewCell.x,previewCell.y,m_pParentDragDropList->sqaViewOwner,previewWidth,previewHeight);
    }
	// Draw the descriptor-defined cell surface; assets own its atlas and crop.
	// Native selection highlights and item icons are drawn above this pass.
	if (InventoryNextEnabled() && color_get_A(m_pParentDragDropList->back_color) != 0)
	{
        UIRender->SetShader(*hSqaGrid);
        Fvector2 gridTextureSize;UIRender->GetActiveTextureResolution(gridTextureSize);
		UI().PushScissor(clientArea);
        UIRender->StartPrimitive((tgt_cells.width()+1)*(tgt_cells.height()+1)*60,
            IUIRender::ptTriList,UI().m_currentPointType);
        const auto storageAt=[&](int x,int y){
            if(x<0||y<0||x>=cell_cnt.x||y>=cell_cnt.y)return 0;
            return m_pParentDragDropList->HasStorageHeaders()?InventoryNextStorageCell(x,y,m_pParentDragDropList->sqaViewOwner):1;
        };
        const auto quad=[&](float left,float top,float right,float bottom,u32 color,bool line){
            for(const auto& point:pts){
                const float u=line?m_sqaGridRect.x1+0.5f:m_sqaGridRect.x1+point.x*m_sqaGridRect.width();
                const float v=line?m_sqaGridRect.y1+0.5f:m_sqaGridRect.y1+point.y*m_sqaGridRect.height();
                UIRender->PushPoint(iFloor(left+point.x*(right-left))-0.5f,iFloor(top+point.y*(bottom-top))-0.5f,0,color,u/gridTextureSize.x,v/gridTextureSize.y);
            }
        };
        if(InventoryNextFieldKitEnabled()&&m_pParentDragDropList->HasStorageHeaders()){
            Fvector2 one;UI().ClientToScreenScaled(one,1.0f,1.0f);
            const float line_x=_max(1.0f,one.x),line_y=_max(1.0f,one.y);
            const auto same=[&](int x,int y,int nx,int ny){
                const int current=storageAt(x,y),neighbor=storageAt(nx,ny);
                if(!current||!neighbor)return false;
                if(current==2&&neighbor==2)return true;
                return InventoryNextStorageSameCompartment(x,y,nx,ny,m_pParentDragDropList->sqaViewOwner);
            };
            const auto origin=[&](int cx,int cy,float& left,float& top){
                float dx=0.0f,dy=0.0f;
                InventoryNextStoragePixelOffset(cx,cy,m_pParentDragDropList->sqaViewOwner,
                    m_cellSize.x+m_cellSpacing.x,dx,dy);
                Fvector2 scaled;UI().ClientToScreenScaled(scaled,dx,dy);
                left=drawLT.x+(f_len.x+sp_len.x)*float(cx-tgt_cells.lt.x)+scaled.x;
                top=drawLT.y+(f_len.y+sp_len.y)*float(cy-tgt_cells.lt.y)+scaled.y;
            };
            for(int x=0;x<=tgt_cells.width();++x)for(int y=0;y<=tgt_cells.height();++y){
                const int cx=x+tgt_cells.lt.x,cy=y+tgt_cells.lt.y;
                if(!storageAt(cx,cy))continue;
                float left=0.0f,top=0.0f;origin(cx,cy,left,top);
                const float right=left+f_len.x+(same(cx,cy,cx+1,cy)?sp_len.x:0.0f);
                const float bottom=top+f_len.y+(same(cx,cy,cx,cy+1)?sp_len.y:0.0f);
                quad(left,top,right,bottom,color_rgba(21,21,21,255),true);
            }
            if(preview)for(int x=0;x<=tgt_cells.width();++x)for(int y=0;y<=tgt_cells.height();++y){
                const int cx=x+tgt_cells.lt.x,cy=y+tgt_cells.lt.y;
                if(!storageAt(cx,cy)||cx<previewCell.x||cx>=previewCell.x+previewWidth||
                    cy<previewCell.y||cy>=previewCell.y+previewHeight)continue;
                float left=0.0f,top=0.0f;origin(cx,cy,left,top);
                quad(left,top,left+f_len.x,top+f_len.y,
                    preview>0?color_rgba(90,210,100,220):color_rgba(225,70,55,220),true);
            }
            for(int x=0;x<=tgt_cells.width();++x)for(int y=0;y<=tgt_cells.height();++y){
                const int cx=x+tgt_cells.lt.x,cy=y+tgt_cells.lt.y;
                if(!storageAt(cx,cy))continue;
                float left=0.0f,top=0.0f;origin(cx,cy,left,top);
                const bool right=same(cx,cy,cx+1,cy),bottom=same(cx,cy,cx,cy+1);
                const bool above=same(cx,cy,cx,cy-1),before=same(cx,cy,cx-1,cy);
                const float edge_right=left+f_len.x+(right?sp_len.x:0.0f);
                const float edge_bottom=top+f_len.y+(bottom?sp_len.y:0.0f);
                const u32 edge=color_rgba(88,88,88,255),rule=color_rgba(48,48,48,255);
                if(!before)quad(left,top,left+line_x,edge_bottom,edge,true);
                if(!above)quad(left,top,edge_right,top+line_y,edge,true);
                if(!right)quad(edge_right-line_x,top,edge_right,edge_bottom,edge,true);
                else {
                    const float center=left+f_len.x+sp_len.x*0.5f;
                    quad(center-line_x*0.5f,top+(above?0.0f:line_y),center+line_x*0.5f,
                        edge_bottom-(bottom?0.0f:line_y),rule,true);
                }
                if(!bottom)quad(left,edge_bottom-line_y,edge_right,edge_bottom,edge,true);
                else {
                    const float center=top+f_len.y+sp_len.y*0.5f;
                    quad(left+(before?0.0f:line_x),center-line_y*0.5f,
                        edge_right-(right?0.0f:line_x),center+line_y*0.5f,rule,true);
                }
            }
        } else for(int x=0;x<=tgt_cells.width();++x)for(int y=0;y<=tgt_cells.height();++y){
            const int cx=x+tgt_cells.lt.x,cy=y+tgt_cells.lt.y,storage=storageAt(cx,cy);
            if(!storage)continue;
            float offset_x=0.0f,offset_y=0.0f;
            if(m_pParentDragDropList->HasStorageHeaders())
                InventoryNextStoragePixelOffset(cx,cy,m_pParentDragDropList->sqaViewOwner,
                    m_cellSize.x+m_cellSpacing.x,offset_x,offset_y);
            Fvector2 offset_scaled;UI().ClientToScreenScaled(offset_scaled,offset_x,offset_y);
            const float left=drawLT.x+(f_len.x+sp_len.x)*float(x)+offset_scaled.x;
            const float top=drawLT.y+(f_len.y+sp_len.y)*float(y)+offset_scaled.y;
            const bool projected=preview&&cx>=previewCell.x&&cx<previewCell.x+previewWidth&&cy>=previewCell.y&&cy<previewCell.y+previewHeight;
            const u32 color=projected?(preview>0?color_rgba(90,210,100,220):color_rgba(225,70,55,220)):(storage==2?color_rgba(210,135,110,255):color_rgba(255,255,255,255));
            quad(left,top,left+f_len.x,top+f_len.y,color,false);
            // Asset owns top/left lines; close only actual bottom/right boundaries.
            if(InventoryNextFieldKitEnabled()){
                const auto region=m_pParentDragDropList->HasStorageHeaders()?InventoryNextStorageRegion(cx,cy,m_pParentDragDropList->sqaViewOwner):0;
                const bool rightSplit=region&&storageAt(cx+1,cy)&&!InventoryNextStorageSameCompartment(cx,cy,cx+1,cy,m_pParentDragDropList->sqaViewOwner);
                const bool bottomSplit=region&&storageAt(cx,cy+1)&&!InventoryNextStorageSameCompartment(cx,cy,cx,cy+1,m_pParentDragDropList->sqaViewOwner);
                if(!storageAt(cx+1,cy)||rightSplit)quad(left+f_len.x-f_len.x/CellSize().x,top,left+f_len.x,top+f_len.y,color,true);
                if(!storageAt(cx,cy+1)||bottomSplit)quad(left,top+f_len.y-f_len.y/CellSize().y,left+f_len.x,top+f_len.y,color,true);
            }
        }
		UIRender->SetShader(*hSqaGrid);
		UIRender->FlushPrimitive();
		UI().PopScissor();
	}

	// fill cell buffer
	u32 max_prim_cnt = ((tgt_cells.width() + 1) * (tgt_cells.height() + 1) * 6);
	UIRender->StartPrimitive(max_prim_cnt, IUIRender::ptTriList, UI().m_currentPointType);

	//	u32 cell_i = 0;
	for (int x = 0; x <= tgt_cells.width(); ++x)
	{
		for (int y = 0; y <= tgt_cells.height(); ++y/*, ++cell_i*/)
		{
			Fvector2 rect_offset;
			rect_offset.set((drawLT.x + f_len.x * x + sp_len.x * x), (drawLT.y + f_len.y * y + sp_len.y * y));
            bool active_storage_cell=true;
            if(m_pParentDragDropList->HasStorageHeaders()){
                float offset_x=0.0f,offset_y=0.0f;
                active_storage_cell=InventoryNextStoragePixelOffset(x+tgt_cells.lt.x,y+tgt_cells.lt.y,
                    m_pParentDragDropList->sqaViewOwner,m_cellSize.x+m_cellSpacing.x,offset_x,offset_y);
                Fvector2 offset_scaled;UI().ClientToScreenScaled(offset_scaled,offset_x,offset_y);
                rect_offset.x+=offset_scaled.x;rect_offset.y+=offset_scaled.y;
            }

			Ivector2 cpos;
			cpos.set(x, y);
			cpos.add(TopVisibleCell());
			CUICell& ui_cell = GetCellAt(cpos);

			u8 select_mode = 0;
			if (!ui_cell.Empty())
			{
				if (ui_cell.m_item->m_cur_mark)
				{
					select_mode = 2;
				}
				else if (ui_cell.m_item->m_selected)
				{
					select_mode = 1;
				}
				else if (ui_cell.m_item->m_select_armament)
				{
					select_mode = 3;
				}
				else if (ui_cell.m_item->m_select_equipped)
				{
					select_mode = 2;
				}
			}

			Fvector2 tp;
			GetTexUVLT(tp, tgt_cells.x1 + x, tgt_cells.y1 + y, select_mode);

			//for (u32 k=0; k<6; ++k,++pv)
			for (u32 k = 0; k < 6; ++k)
			{
				const Fvector2& p = pts[k];
				const Fvector2& uv = uvs[k];
				//pv->set			(iFloor(drawLT.x + p.x*(f_len.x) + f_len.x*x)-0.5f, 
				//				 iFloor(drawLT.y + p.y*(f_len.y) + f_len.y*y)-0.5f, 
				//				 0xFFFFFFFF,tp.x+uv.x,tp.y+uv.y);
				UIRender->PushPoint(iFloor(rect_offset.x + p.x * (f_len.x)) - 0.5f,
				                    iFloor(rect_offset.y + p.y * (f_len.y)) - 0.5f,
				                    0,
				                    active_storage_cell?m_pParentDragDropList->back_color:0,
				                    tp.x + uv.x, tp.y + uv.y);
			} //for k
		} //for y
	} // for x
	UI().PushScissor(clientArea);

	UIRender->SetShader(*hShader);
	UIRender->FlushPrimitive();

	//draw shown items in range
	if (m_cells_to_draw.size())
	{
		UI_CELLS_VEC_IT it = m_cells_to_draw.begin();
		for (; it != m_cells_to_draw.end(); ++it) // all cells
		{
			CUICell& cell = (*it);
			if (!cell.Empty() && (cell.m_item->m_drawn_frame != Device.dwFrame))
			{
				cell.m_item->Draw();
			}
		}
	}

	UI().PopScissor();
}

void CUICellContainer::clear_select_armament()
{
	UI_CELLS_VEC_IT itb = m_cells.begin();
	UI_CELLS_VEC_IT ite = m_cells.end();
	for (; itb != ite; ++itb)
	{
		CUICell& cell = (*itb);
		if (cell.m_item)
		{
			cell.m_item->m_select_armament = false;
		}
	}
}
