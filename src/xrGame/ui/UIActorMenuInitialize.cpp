#include "stdafx.h"
#include "UIActorMenu.h"
#include "../InventoryNextIdentity.h"
#include "../InventoryNextProbe.h"
#include "UITextureMaster.h"
#include "UIFrameWindow.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UIActorStateInfo.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIMessageBoxEx.h"
#include "UIPropertiesBox.h"
#include "UI3tButton.h"

#include "UIInventoryUpgradeWnd.h"
#include "UIInvUpgradeInfo.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "object_broker.h"
#include "UIWndCallback.h"
#include "UIHelper.h"
#include "UIProgressBar.h"
#include "ui_base.h"
#include "../string_table.h"

CUIActorMenu::CUIActorMenu()
{
	m_currMenuMode = mmUndefined;
	m_trade_partner_inventory_state = 0;
	Construct();
}

CUIActorMenu::~CUIActorMenu()
{
	xr_delete(m_message_box_yes_no);
	xr_delete(m_message_box_ok);
	xr_delete(m_UIPropertiesBox);
	xr_delete(m_hint_wnd);
	xr_delete(m_ItemInfo);

	ClearAllLists();
}

void CUIActorMenu::Construct()
{
	CUIXml uiXml;
	if(InventoryNextEnabled()) {
        CUITextureMaster::ParseShTexInfo("sqa3_native.xml");
        if(InventoryNextFieldKitEnabled())CUITextureMaster::ParseShTexInfo("sqa3_visual_assets.xml");
        uiXml.Load(CONFIG_PATH, UI_PATH, InventoryNextFieldKitEnabled()?"sqa3_field_kit.xml":"sqa3_actor_menu.xml");
        Msg("* SQA3 replacement actor menu: native inventory, equipment, trade and loot controls; no 3D model");
    } else uiXml.Load(CONFIG_PATH, UI_PATH, "actor_menu.xml");

    if(InventoryNextEnabled()&&!InventoryNextFieldKitEnabled()) {
        auto frame=xr_new<CUIFrameWindow>();frame->SetAutoDelete(true);AttachChild(frame);
        frame->SetWndPos(Fvector2().set(0,0));frame->SetWndSize(Fvector2().set(1024,768));
        frame->InitTexture("sqa_fk_window");frame->Enable(false);
    }
	CUIXmlInit xml_init;

	xml_init.InitWindow(uiXml, "main", 0, this);
    if(InventoryNextFieldKitEnabled()){
        auto frame=xr_new<CUIStatic>();frame->SetAutoDelete(true);AttachChild(frame);frame->Enable(false);
        frame->SetWndPos(Fvector2().set(0,0));frame->SetWndSize(Fvector2().set(260.5f,768.0f));frame->InitTexture("sqa3v1_equipment_panel");frame->SetStretchTexture(true);
    }

	if (InventoryNextEnabled()) m_sqaStatus=UIHelper::CreateStatic(uiXml,"sqa_status",this);
	m_hint_wnd = UIHelper::CreateHint(uiXml, "hint_wnd");

	m_LeftBackground = xr_new<CUIStatic>();
	m_LeftBackground->SetAutoDelete(true);
	AttachChild(m_LeftBackground);
	xml_init.InitStatic(uiXml, "left_background", 0, m_LeftBackground);


	m_pUpgradeWnd = xr_new<CUIInventoryUpgradeWnd>();
	AttachChild(m_pUpgradeWnd);
	m_pUpgradeWnd->SetAutoDelete(true);
	m_pUpgradeWnd->Init();

	m_ActorCharacterInfo = xr_new<CUICharacterInfo>();
	m_ActorCharacterInfo->SetAutoDelete(true);
	AttachChild(m_ActorCharacterInfo);
	m_ActorCharacterInfo->InitCharacterInfo(&uiXml, "actor_ch_info");

	m_PartnerCharacterInfo = xr_new<CUICharacterInfo>();
	m_PartnerCharacterInfo->SetAutoDelete(true);
	AttachChild(m_PartnerCharacterInfo);
	m_PartnerCharacterInfo->InitCharacterInfo(&uiXml, "partner_ch_info");

	m_RightDelimiter = UIHelper::CreateStatic(uiXml, "right_delimiter", this);
	//	m_ActorTradeCaption			= UIHelper::CreateTextWnd(uiXml, "right_delimiter:trade_caption", m_RightDelimiter);
	m_ActorTradePrice = UIHelper::CreateTextWnd(uiXml, "right_delimiter:trade_price", m_RightDelimiter);
	m_ActorTradeWeightMax = UIHelper::CreateTextWnd(uiXml, "right_delimiter:trade_weight_max", m_RightDelimiter);
	//	m_ActorTradeCaption->AdjustWidthToText();

	m_LeftDelimiter = UIHelper::CreateStatic(uiXml, "left_delimiter", this);
	//	m_PartnerTradeCaption		= UIHelper::CreateTextWnd(uiXml, "left_delimiter:trade_caption", m_LeftDelimiter);
	m_PartnerTradePrice = UIHelper::CreateTextWnd(uiXml, "left_delimiter:trade_price", m_LeftDelimiter);
	m_PartnerTradeWeightMax = UIHelper::CreateTextWnd(uiXml, "left_delimiter:trade_weight_max", m_LeftDelimiter);
	//	m_PartnerTradeCaption->AdjustWidthToText();

	m_ActorBottomInfo = UIHelper::CreateStatic(uiXml, "actor_weight_caption", this);
	m_ActorWeight = UIHelper::CreateTextWnd(uiXml, "actor_weight", this);
	m_ActorWeightMax = UIHelper::CreateTextWnd(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	m_PartnerBottomInfo = UIHelper::CreateStatic(uiXml, "partner_weight_caption", this);
	m_PartnerWeight = UIHelper::CreateTextWnd(uiXml, "partner_weight", this);
	m_PartnerBottomInfo->AdjustWidthToText();
	m_PartnerWeight_end_x = m_PartnerWeight->GetWndPos().x;

	m_QuickSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
	m_QuickSlotsHighlight[0]->Show(false);
	m_ArtefactSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "artefact_slot_highlight", this);
	m_ArtefactSlotsHighlight[0]->Show(false);

	Fvector2 pos;
	pos = m_QuickSlotsHighlight[0]->GetWndPos();
	float dx = uiXml.ReadAttribFlt("quick_slot_highlight", 0, "dx", 24.0f);
	for (u8 i = 1; i < 4; i++)
	{
		pos.x += dx;
		m_QuickSlotsHighlight[i] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
		m_QuickSlotsHighlight[i]->SetWndPos(pos);
		m_QuickSlotsHighlight[i]->Show(false);
	}
	pos = m_ArtefactSlotsHighlight[0]->GetWndPos();
	dx = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dx", 24.0f);
	for (u8 i = 1; i < e_af_count; i++)
	{
		pos.x += dx;
		m_ArtefactSlotsHighlight[i] = UIHelper::CreateStatic(uiXml, "artefact_slot_highlight", this);
		m_ArtefactSlotsHighlight[i]->SetWndPos(pos);
		m_ArtefactSlotsHighlight[i]->Show(false);
	}

	m_pInventoryBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bag", this);
	m_pInventoryBeltList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_belt", this);
	m_pTradeActorBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade_bag", this);
	m_pTradeActorList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade", this);
	m_pTradePartnerBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_bag", this);
	m_pTradePartnerList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_trade", this);
	m_pDeadBodyBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_deadbody_bag", this);
    if(InventoryNextFieldKitEnabled())for(int index=0;index<4;++index){
        auto frame=xr_new<CUIStatic>();frame->SetAutoDelete(true);AttachChild(frame);frame->Enable(false);
        frame->SetWndPos(Fvector2().set(12.0f+40.0f*index,433.0f));frame->SetWndSize(Fvector2().set(33,44));frame->InitTexture("sqa3v1_slot_quick");frame->SetStretchTexture(true);
    }
	m_pQuickSlot = UIHelper::CreateDragDropReferenceList(uiXml, "dragdrop_quick_slots", this);
	m_pQuickSlot->Initialize();

	m_pTrashList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_trash", this);
	m_pTrashList->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemDrop);
	m_pTrashList->m_f_drag_event = CUIDragDropListEx::DRAG_ITEM_EVENT(this, &CUIActorMenu::OnDragItemOnTrash);

	m_belt_list_over[0] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
	pos = m_belt_list_over[0]->GetWndPos();
	dx = uiXml.ReadAttribFlt("belt_list_over", 0, "dx", 10.0f);
	for (u8 i = 1; i < e_af_count; ++i)
	{
		pos.x += dx;
		m_belt_list_over[i] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
		m_belt_list_over[i]->SetWndPos(pos);
	}
	m_HelmetOver = UIHelper::CreateStatic(uiXml, "helmet_over", this);
	m_HelmetOver->Show(false);

	m_BackpackOver = UIHelper::CreateStatic(uiXml, "backpack_over", this);
	m_BackpackOver->Show(false);

	m_ActorMoney = UIHelper::CreateTextWnd(uiXml, "actor_money_static", this);
	m_PartnerMoney = UIHelper::CreateTextWnd(uiXml, "partner_money_static", this);
	m_QuickSlot1 = UIHelper::CreateTextWnd(uiXml, "quick_slot1_text", this);
	m_QuickSlot2 = UIHelper::CreateTextWnd(uiXml, "quick_slot2_text", this);
	m_QuickSlot3 = UIHelper::CreateTextWnd(uiXml, "quick_slot3_text", this);
	m_QuickSlot4 = UIHelper::CreateTextWnd(uiXml, "quick_slot4_text", this);

	m_trade_buy_button = UIHelper::Create3tButton(uiXml, "trade_buy_button", this);
	m_trade_sell_button = UIHelper::Create3tButton(uiXml, "trade_sell_button", this);
	m_takeall_button = UIHelper::Create3tButton(uiXml, "takeall_button", this);
	m_putall_button = UIHelper::Create3tButton(uiXml, "putall_button", this);
	m_exit_button = UIHelper::Create3tButton(uiXml, "exit_button", this);

	//Alun: Dynamic UI slots bro
	for (u8 i = 0; i <= LAST_SLOT; ++i)
	{
		m_pInvList[i] = NULL;
		m_pInvSlotHighlight[i] = NULL;
		m_pInvSlotProgress[i] = NULL;
	}

	XML_NODE* stored_root = uiXml.GetLocalRoot();
	XML_NODE* node = uiXml.NavigateToNode("inventory_slot_wnd", 0);
	uiXml.SetLocalRoot(node);

	m_slot_count = (u8)uiXml.GetNodesNum(node, "slot");
	for (u8 i = 1; i <= m_slot_count; ++i)
	{
		uiXml.SetLocalRoot(node);
		XML_NODE* slot_node = uiXml.NavigateToNode("slot", i - 1);
		uiXml.SetLocalRoot(slot_node);

		if (uiXml.GetNodesNum(slot_node, "slot_dragdrop") == 0)
			continue;

        if(InventoryNextFieldKitEnabled()&&i!=16&&i!=5){
            auto frame=xr_new<CUIFrameWindow>();frame->SetAutoDelete(true);AttachChild(frame);frame->Enable(false);
            frame->SetWndPos(Fvector2().set(uiXml.ReadAttribFlt("slot_dragdrop",0,"x",0),uiXml.ReadAttribFlt("slot_dragdrop",0,"y",0)));
            frame->SetWndSize(Fvector2().set(uiXml.ReadAttribFlt("slot_dragdrop",0,"width",75),uiXml.ReadAttribFlt("slot_dragdrop",0,"height",42)));
            frame->InitTexture("sqa3v1_slot_frame");
            const LPCSTR labels[]={"","Knife / binoc.","Weapon 2","Weapon 1","Grenade","Binoculars","Bolt","Armour / rig","PDA","Detector","Torch","","Helmet","Backpack"};
            if(i<14){auto caption=xr_new<CUIStatic>();caption->SetAutoDelete(true);frame->AttachChild(caption);caption->Enable(false);caption->TextureOff();
                caption->SetWndPos(Fvector2().set(4.0f,frame->GetWndSize().y-14.0f));caption->SetWndSize(Fvector2().set(frame->GetWndSize().x-8.0f,12.0f));
                caption->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);caption->TextItemControl()->SetTextAlignment(CGameFont::alCenter);caption->TextItemControl()->SetText(labels[i]);if(i==OUTFIT_SLOT)m_sqaChestCaption=caption;}

        }
		m_pInvList[i] = xr_new<CUIDragDropListEx>();
		AttachChild(m_pInvList[i]);
		CUIXmlInit::InitDragDropListEx(uiXml, "slot_dragdrop", 0, m_pInvList[i]);
		m_pInvList[i]->SetAutoDelete(true);

		BindDragDropListEvents(m_pInvList[i]);

		m_pInvSlotHighlight[i] = xr_new<CUIStatic>();
		AttachChild(m_pInvSlotHighlight[i]);
		CUIXmlInit::InitStatic(uiXml, "slot_highlight", 0, m_pInvSlotHighlight[i]);
		m_pInvSlotHighlight[i]->SetAutoDelete(true);
		m_pInvSlotHighlight[i]->Show(false);

		if (uiXml.GetNodesNum(slot_node, "slot_progress") == 0)
			continue;

		m_pInvSlotProgress[i] = xr_new<CUIProgressBar>();
		AttachChild(m_pInvSlotProgress[i]);
		CUIXmlInit::InitProgressBar(uiXml, "slot_progress", 0, m_pInvSlotProgress[i]);
		m_pInvSlotProgress[i]->SetAutoDelete(true);
	}
	uiXml.SetLocalRoot(stored_root);
    if(InventoryNextFieldKitEnabled()){
        // Explicitly visual-only in vanilla: no inventory list, ownership or drop target.
        auto pistol=xr_new<CUIFrameWindow>();pistol->SetAutoDelete(true);AttachChild(pistol);pistol->Enable(false);
        pistol->SetWndPos(Fvector2().set(174,171));pistol->SetWndSize(Fvector2().set(75,42));pistol->InitTexture("sqa3v1_slot_frame");
        auto caption=xr_new<CUIStatic>();caption->SetAutoDelete(true);pistol->AttachChild(caption);caption->Enable(false);caption->TextureOff();
        caption->SetWndPos(Fvector2().set(4,28));caption->SetWndSize(Fvector2().set(67,12));
        caption->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);caption->TextItemControl()->SetTextAlignment(CGameFont::alCenter);caption->TextItemControl()->SetText("Pistol");
        for(int side=0;side<2;++side){
            auto button=xr_new<CUIStatic>();button->SetAutoDelete(true);AttachChild(button);button->Enable(false);
            button->SetWndPos(Fvector2().set(side?163:88,75));button->SetWndSize(Fvector2().set(10,18));
            button->InitTexture(side?"sqa3v1_arrow_right_e":"sqa3v1_arrow_left_e");button->SetStretchTexture(true);
        }
    }
	//-Alun

	m_ActorStateInfo = xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete(true);
	AttachChild(m_ActorStateInfo);

	/*XML_NODE**/
	stored_root = uiXml.GetLocalRoot();
	uiXml.SetLocalRoot(uiXml.NavigateToNode("action_sounds", 0));
	::Sound->create(sounds[eSndOpen], uiXml.Read("snd_open", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eSndClose], uiXml.Read("snd_close", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eItemToSlot], uiXml.Read("snd_item_to_slot", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eItemToBelt], uiXml.Read("snd_item_to_belt", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eItemToRuck], uiXml.Read("snd_item_to_ruck", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eProperties], uiXml.Read("snd_properties", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eDropItem], uiXml.Read("snd_drop_item", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eAttachAddon], uiXml.Read("snd_attach_addon", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eDetachAddon], uiXml.Read("snd_detach_addon", 0, NULL), st_Effect, sg_SourceType);
	::Sound->create(sounds[eItemUse], uiXml.Read("snd_item_use", 0, NULL), st_Effect, sg_SourceType);
	uiXml.SetLocalRoot(stored_root);

	m_ItemInfo = xr_new<CUIItemInfo>();
	//-	m_ItemInfo->SetAutoDelete			(true);
	//-	AttachChild							(m_ItemInfo);
	m_ItemInfo->InitItemInfo("actor_menu_item.xml");

	m_upgrade_info = NULL;
	if (ai().get_alife())
	{
		m_upgrade_info = xr_new<UIInvUpgradeInfo>();
		m_upgrade_info->SetAutoDelete(true);
		AttachChild(m_upgrade_info);
		m_upgrade_info->init_from_xml("actor_menu_item.xml");
	}

	m_message_box_yes_no = xr_new<CUIMessageBoxEx>();
	m_message_box_yes_no->InitMessageBox("message_box_yes_no");
	m_message_box_yes_no->SetAutoDelete(true);
	m_message_box_yes_no->SetText("");

	m_message_box_ok = xr_new<CUIMessageBoxEx>();
	m_message_box_ok->InitMessageBox("message_box_ok");
	m_message_box_ok->SetAutoDelete(true);
	m_message_box_ok->SetText("");

	m_UIPropertiesBox = xr_new<CUIPropertiesBox>();
	m_UIPropertiesBox->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300));
	AttachChild(m_UIPropertiesBox);
	m_UIPropertiesBox->Hide();
	m_UIPropertiesBox->SetWindowName("property_box");

	b_sort_hotkeys = pSettings->r_bool("button_sort_settings", "hotkeys");
	u16 buttons = pSettings->r_u16("button_sort_settings", "button_amount");

	for (u16 i = 1; i <= buttons; i++)
	{
		xr_string buttonName = "button_sort_tab_";
		buttonName.append(std::to_string(i).c_str());

		CUI3tButton* newbutton = UIHelper::Create3tButton(uiXml, buttonName.c_str(), this);
		m_sort_buttons.push_back(newbutton);
		Register(m_sort_buttons.back());

		AddCallback(m_sort_buttons.back(), BUTTON_CLICKED,
			CUIWndCallback::void_function(this, &CUIActorMenu::sort_button_callback));

		m_sort_kinds.push_back(pSettings->r_string(buttonName.c_str(), "kinds"));
	}

	InitCallbacks();

	BindDragDropListEvents(m_pInventoryBeltList);
	BindDragDropListEvents(m_pInventoryBagList);
	BindDragDropListEvents(m_pTradeActorBagList);
	BindDragDropListEvents(m_pTradeActorList);
	BindDragDropListEvents(m_pTradePartnerBagList);
	BindDragDropListEvents(m_pTradePartnerList);
	BindDragDropListEvents(m_pDeadBodyBagList);
	BindDragDropListEvents(m_pQuickSlot);

	m_allowed_drops[iTrashSlot].push_back(iActorBag);
	m_allowed_drops[iTrashSlot].push_back(iActorSlot);
	m_allowed_drops[iTrashSlot].push_back(iActorBelt);
	m_allowed_drops[iTrashSlot].push_back(iQuickSlot);

	m_allowed_drops[iActorSlot].push_back(iActorBag);
	m_allowed_drops[iActorSlot].push_back(iActorSlot);
	m_allowed_drops[iActorSlot].push_back(iActorTrade);
	m_allowed_drops[iActorSlot].push_back(iDeadBodyBag);

	m_allowed_drops[iActorBag].push_back(iActorSlot);
	m_allowed_drops[iActorBag].push_back(iActorBelt);
	m_allowed_drops[iActorBag].push_back(iActorTrade);
	m_allowed_drops[iActorBag].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBag].push_back(iActorBag);
	m_allowed_drops[iActorBag].push_back(iQuickSlot);

	m_allowed_drops[iActorBelt].push_back(iActorBag);
	m_allowed_drops[iActorBelt].push_back(iActorTrade);
	m_allowed_drops[iActorBelt].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBelt].push_back(iActorBelt);

	m_allowed_drops[iActorTrade].push_back(iActorSlot);
	m_allowed_drops[iActorTrade].push_back(iActorBag);
	m_allowed_drops[iActorTrade].push_back(iActorBelt);
	m_allowed_drops[iActorTrade].push_back(iActorTrade);
	m_allowed_drops[iActorTrade].push_back(iQuickSlot);

	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTrade);
	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTrade);

	m_allowed_drops[iDeadBodyBag].push_back(iActorSlot);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBag);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBelt);
	m_allowed_drops[iDeadBodyBag].push_back(iDeadBodyBag);

	m_allowed_drops[iQuickSlot].push_back(iActorBag);
	m_allowed_drops[iQuickSlot].push_back(iActorTrade);

	m_upgrade_selected = NULL;
	SetCurrentItem(NULL);
	SetActor(NULL);
	SetPartner(NULL);
	SetInvBox(NULL);

	m_actor_trade = NULL;
	m_partner_trade = NULL;
	m_repair_mode = false;
	m_item_info_view = false;
	m_highlight_clear = true;

	DeInitInventoryMode();
	DeInitTradeMode();
	DeInitUpgradeMode();
	DeInitDeadBodySearchMode();
}

void CUIActorMenu::BindDragDropListEvents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemDrop);
	lst->m_f_item_start_drag = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemStartDrag);
	lst->m_f_item_db_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemDbClick);
	lst->m_f_item_selected = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemSelected);
	lst->m_f_item_rbutton_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemRButtonClick);
	lst->m_f_item_focus_received = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemFocusReceive);
	lst->m_f_item_focus_lost = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemFocusLost);
	lst->m_f_item_focused_update = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIActorMenu::OnItemFocusedUpdate);
}

void CUIActorMenu::InitCallbacks()
{
	Register(m_trade_buy_button);
	Register(m_trade_sell_button);
	Register(m_takeall_button);
	Register(m_putall_button);
	Register(m_exit_button);
	Register(m_UIPropertiesBox);
	VERIFY(m_pUpgradeWnd);
	Register(m_pUpgradeWnd->m_btn_repair);

	AddCallback(m_trade_buy_button, BUTTON_CLICKED,
	            CUIWndCallback::void_function(this, &CUIActorMenu::OnBtnPerformTradeBuy));
	AddCallback(m_trade_sell_button, BUTTON_CLICKED,
	            CUIWndCallback::void_function(this, &CUIActorMenu::OnBtnPerformTradeSell));
	AddCallback(m_takeall_button, BUTTON_CLICKED,
	            CUIWndCallback::void_function(this, &CUIActorMenu::TakeAllFromPartner)); 
	AddCallback(m_putall_button, BUTTON_CLICKED,
				CUIWndCallback::void_function(this, &CUIActorMenu::PutAllToPartner));
	AddCallback(m_exit_button, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUIActorMenu::OnBtnExitClicked));
	AddCallback(m_UIPropertiesBox, PROPERTY_CLICKED,
	            CUIWndCallback::void_function(this, &CUIActorMenu::ProcessPropertiesBoxClicked));
	AddCallback(m_pUpgradeWnd->m_btn_repair, BUTTON_CLICKED,
	            CUIWndCallback::void_function(this, &CUIActorMenu::TryRepairItem));
}

void CUIActorMenu::UpdateButtonsLayout()
{
	string32 tmp;
	LPCSTR str = CStringTable().translate("quick_use_str_1").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot1->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_2").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot2->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_3").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot3->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_4").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot4->SetTextST(tmp);

	UpdateConditionProgressBars();
}
