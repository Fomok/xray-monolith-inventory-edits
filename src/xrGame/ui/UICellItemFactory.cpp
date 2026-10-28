#include "stdafx.h"
#include "UICellItemFactory.h"
#include "UICellCustomItems.h"
#include "../InventoryNextProbe.h"


CUICellItem* create_cell_item(CInventoryItem* itm)
{
	VERIFY(itm);
	CUICellItem* cell_item;

	CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(itm);
	CWeapon* pWeapon = smart_cast<CWeapon*>(itm);
	if (pAmmo)
	{
		cell_item = xr_new<CUIAmmoCellItem>(pAmmo);
	}
	else if (pWeapon)
	{
		cell_item = xr_new<CUIWeaponCellItem>(pWeapon);
	}
	else
	{
		cell_item = xr_new<CUIInventoryCellItem>(itm);
	}
	int width=0,height=0;
	if(InventoryNextNativeFootprint(itm->object_id(),width,height))cell_item->SetSqaFootprint(width,height,false);
	return cell_item;
}
