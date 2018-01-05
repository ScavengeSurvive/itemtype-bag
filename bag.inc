// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_bag
    #undef _inc_bag
#endif
// custom include-guard to ensure we don't duplicate
#if defined _bag_included
	#endinput
#endif
#define _bag_included


#include <a_samp>
#include <logger>
#include <language>
#include <item>
#include <item-array-data>
#include <container>
#include <inventory>

#include <YSI\y_hooks>


#if !defined MAX_BAG_TYPE
	#define MAX_BAG_TYPE (10)
#endif

#if !defined BAG_ATTACH_INDEX
	#define BAG_ATTACH_INDEX (10)
#endif

enum E_BAG_TYPE_DATA {
	ItemType:bag_itemtype,
	bag_size,
	Float:bag_attachOffsetX,
	Float:bag_attachOffsetY,
	Float:bag_attachOffsetZ,
	Float:bag_attachRotX,
	Float:bag_attachRotY,
	Float:bag_attachRotZ,
	Float:bag_attachScaleX,
	Float:bag_attachScaleY,
	Float:bag_attachScaleZ
}

static
	bag_TypeData[MAX_BAG_TYPE][E_BAG_TYPE_DATA],
	bag_TypeTotal,
	bag_ItemTypeBagType[MAX_ITEM_TYPE] = {-1, ...},

	bag_ContainerItem[MAX_CONTAINER],
	bag_ContainerPlayer[MAX_CONTAINER],

	bag_PlayerBagID[MAX_PLAYERS],
	bag_InventoryOptionID[MAX_PLAYERS],
	bool:bag_PuttingInBag[MAX_PLAYERS],
	bool:bag_TakingOffBag[MAX_PLAYERS],
	bag_CurrentBag[MAX_PLAYERS],
	Timer:bag_OtherPlayerEnter[MAX_PLAYERS],
	bag_LookingInBag[MAX_PLAYERS];


forward OnPlayerWearBag(playerid, itemid);
forward OnPlayerRemoveBag(playerid, itemid);


hook OnScriptInit() {
	for(new i; i < MAX_CONTAINER; i++) {
		bag_ContainerPlayer[i] = INVALID_PLAYER_ID;
		bag_ContainerItem[i] = INVALID_ITEM_ID;
	}
}

hook OnPlayerConnect(playerid) {
	bag_PlayerBagID[playerid] = INVALID_ITEM_ID;
	bag_PuttingInBag[playerid] = false;
	bag_TakingOffBag[playerid] = false;
	bag_CurrentBag[playerid] = INVALID_ITEM_ID;
	bag_LookingInBag[playerid] = INVALID_PLAYER_ID;
}


/*==============================================================================

	Core

==============================================================================*/


stock DefineBagType(name[MAX_ITEM_NAME], ItemType:itemtype, size, Float:attachOffsetX, Float:attachOffsetY, Float:attachOffsetZ, Float:attachRotX, Float:attachRotY, Float:attachRotZ, Float:attachScaleX, Float:attachScaleY, Float:attachScaleZ) {
	if(bag_TypeTotal == MAX_BAG_TYPE-1) {
		fatal("maximum amount of bags defined");
		return -1;
	}

	SetItemTypeMaxArrayData(itemtype, 2);

	bag_TypeData[bag_TypeTotal][bag_itemtype]		= itemtype;
	bag_TypeData[bag_TypeTotal][bag_size]			= size;
	bag_TypeData[bag_TypeTotal][bag_attachOffsetX]	= attachOffsetX;
	bag_TypeData[bag_TypeTotal][bag_attachOffsetY]	= attachOffsetY;
	bag_TypeData[bag_TypeTotal][bag_attachOffsetZ]	= attachOffsetZ;
	bag_TypeData[bag_TypeTotal][bag_attachRotX]		= attachRotX;
	bag_TypeData[bag_TypeTotal][bag_attachRotY]		= attachRotY;
	bag_TypeData[bag_TypeTotal][bag_attachRotZ]		= attachRotZ;
	bag_TypeData[bag_TypeTotal][bag_attachScaleX]	= attachScaleX;
	bag_TypeData[bag_TypeTotal][bag_attachScaleY]	= attachScaleY;
	bag_TypeData[bag_TypeTotal][bag_attachScaleZ]	= attachScaleZ;

	bag_ItemTypeBagType[itemtype] = bag_TypeTotal;

	return bag_TypeTotal++;
}

stock GivePlayerBag(playerid, itemid) {
	if(!IsValidItem(itemid)) {
		return 0;
	}

	new bagtype = bag_ItemTypeBagType[GetItemType(itemid)];

	if(bagtype != -1) {
		new containerid = GetItemArrayDataAtCell(itemid, 1);

		if(!IsValidContainer(containerid)) {
			err("bag containerid was invalid and will be recreated",
				_i("itemid", itemid),
				_i("containerid", containerid));
			
			_bag_createContainerForItem(itemid);
		}

		new colour;
		GetItemTypeColour(bag_TypeData[bagtype][bag_itemtype], colour);

		bag_PlayerBagID[playerid] = itemid;

		SetPlayerAttachedObject(playerid, BAG_ATTACH_INDEX, GetItemTypeModel(bag_TypeData[bagtype][bag_itemtype]), 1,
			bag_TypeData[bagtype][bag_attachOffsetX],
			bag_TypeData[bagtype][bag_attachOffsetY],
			bag_TypeData[bagtype][bag_attachOffsetZ],
			bag_TypeData[bagtype][bag_attachRotX],
			bag_TypeData[bagtype][bag_attachRotY],
			bag_TypeData[bagtype][bag_attachRotZ],
			bag_TypeData[bagtype][bag_attachScaleX],
			bag_TypeData[bagtype][bag_attachScaleY],
			bag_TypeData[bagtype][bag_attachScaleZ], colour, colour);

		bag_ContainerItem[containerid] = itemid;
		bag_ContainerPlayer[containerid] = playerid;
		RemoveItemFromWorld(itemid);
		RemoveCurrentItem(GetItemHolder(itemid));

		return 1;
	}

	return 0;
}

stock RemovePlayerBag(playerid)
{
	if(!IsPlayerConnected(playerid))
		return 0;

	if(!IsValidItem(bag_PlayerBagID[playerid]))
		return 0;

	new containerid = GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1);

	if(!IsValidContainer(containerid)) {
		err("bag containerid was invalid and will be recreated",
			_i("itemid", bag_PlayerBagID[playerid]),
			_i("containerid", containerid));

		_bag_createContainerForItem(bag_PlayerBagID[playerid]);
	}

	RemovePlayerAttachedObject(playerid, BAG_ATTACH_INDEX);
	CreateItemInWorld(bag_PlayerBagID[playerid], 0.0, 0.0, 0.0, .world = GetPlayerVirtualWorld(playerid), .interior = GetPlayerInterior(playerid));

	bag_ContainerPlayer[containerid] = INVALID_PLAYER_ID;
	bag_PlayerBagID[playerid] = INVALID_ITEM_ID;

	return 1;
}

stock DestroyPlayerBag(playerid) {
	if(!(0 <= playerid < MAX_PLAYERS)) {
		return 0;
	}

	if(!IsValidItem(bag_PlayerBagID[playerid])) {
		return 0;
	}

	new containerid = GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1);

	if(IsValidContainer(containerid)) {
		bag_ContainerPlayer[containerid] = INVALID_PLAYER_ID;
		DestroyContainer(containerid);
	}

	RemovePlayerAttachedObject(playerid, BAG_ATTACH_INDEX);
	DestroyItem(bag_PlayerBagID[playerid]);

	bag_PlayerBagID[playerid] = INVALID_ITEM_ID;

	return 1;
}

// Automatically determines whether to add to the player's inventory or bag.
stock AddItemToPlayer(playerid, itemid, useinventory = false, playeraction = true)
{
	new ItemType:itemtype = GetItemType(itemid);

	if(IsItemTypeCarry(itemtype))
		return -1;

	new required;

	if(useinventory) {
		required = AddItemToInventory(playerid, itemid);
	}

	if(required == 0)
		return 0;

	if(!IsValidItem(bag_PlayerBagID[playerid]))
	{
		if(required > 0) {
			new message[128];
			format(message, sizeof(message), ls(playerid, "CNTEXTRASLO", true), required);
			ShowActionText(playerid, message, 3000, 150);
		}

		return -3;
	}

	new containerid;
	if(GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1, containerid) != 0) {
		return -3;
	}

	if(!IsValidContainer(containerid))
		return -3;

	new
		itemsize = GetItemTypeSize(GetItemType(itemid)),
		freeslots;
	
	if(GetContainerFreeSlots(containerid, freeslots) != 0) {
		return -3;
	}

	if(itemsize > freeslots)
	{
		new message[128];
		format(message, sizeof message, ls(playerid, "BAGEXTRASLO", true), itemsize - freeslots);
		ShowActionText(playerid, message, 3000, 150);
		return -4;
	}

	if(playeraction)
	{
		ShowActionText(playerid, ls(playerid, "BAGITMADDED", true), 3000, 150);
		ApplyAnimation(playerid, "PED", "PHONE_IN", 4.0, 1, 0, 0, 0, 300);
		bag_PuttingInBag[playerid] = true;
		defer bag_PutItemIn(playerid, itemid, containerid);
	}
	else
	{
		return AddItemToContainer(containerid, itemid, playerid);
	}

	return 0;
}


/*==============================================================================

	Internal Functions and Hooks

==============================================================================*/


_bag_createContainerForItem(itemid) {
	new
		ItemType:itemType = GetItemType(itemid),
		bagtype,
		containerid;
	
	bagtype = bag_ItemTypeBagType[itemType];

	if(bagtype == -1) {
		err("attempt to create bag container for item that is not a bag");
		return 1;
	}

	new name[MAX_ITEM_NAME];
	GetItemTypeName(itemType, name);
	containerid = CreateContainer(name, bag_TypeData[bagtype][bag_size]);

	bag_ContainerItem[containerid] = itemid;
	bag_ContainerPlayer[containerid] = INVALID_PLAYER_ID;

	SetItemArrayDataSize(itemid, 2);
	SetItemArrayDataAtCell(itemid, containerid, 1);

	return 0;
}

hook OnItemCreate(itemid)
{
	new bagtype = bag_ItemTypeBagType[GetItemType(itemid)];

	if(bagtype != -1) {
		_bag_createContainerForItem(itemid);
	}
}

hook OnItemCreateInWorld(itemid)
{
	if(IsItemTypeBag(GetItemType(itemid)))
	{
		SetButtonText(GetItemButtonID(itemid), "Hold "ITEM_GAMETEXT_INTERACT" to pick up~n~Press "ITEM_GAMETEXT_INTERACT" to open");
	}
}

hook OnItemDestroy(itemid)
{
	if(IsItemTypeBag(GetItemType(itemid)))
	{
		new containerid = GetItemArrayDataAtCell(itemid, 1);

		if(IsValidContainer(containerid))
		{
			bag_ContainerPlayer[containerid] = INVALID_PLAYER_ID;
			bag_ContainerItem[containerid] = INVALID_ITEM_ID;
			DestroyContainer(containerid);
		}
	}
}

hook OnPlayerUseItem(playerid, itemid)
{
	if(bag_ItemTypeBagType[GetItemType(itemid)] != -1)
	{
		// is the player inside a container view? is the item usage, out of world?
		// if(IsValidContainer(GetPlayerCurrentContainer(playerid)))
		//	return Y_HOOKS_CONTINUE_RETURN_0;

		if(IsItemInWorld(itemid))
			_DisplayBagDialog(playerid, itemid, true);

		else
			_DisplayBagDialog(playerid, itemid, false);

		return Y_HOOKS_BREAK_RETURN_1;
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerUseItemWithItem(playerid, itemid, withitemid)
{
	if(bag_ItemTypeBagType[GetItemType(withitemid)] != -1)
	{
		_DisplayBagDialog(playerid, withitemid, true);
		return Y_HOOKS_BREAK_RETURN_1;
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
	if(GetPlayerSpecialAction(playerid) == SPECIAL_ACTION_CUFFED || GetPlayerAnimationIndex(playerid) == 1381) {
		return 1;
	}

	if(IsPlayerInAnyVehicle(playerid))
		return 1;

	if(newkeys & KEY_YES)
	{
		_BagEquipHandler(playerid);
	}

	if(newkeys & KEY_NO)
	{
		_BagDropHandler(playerid);
	}

	if(newkeys & 16)
	{
		_BagRummageHandler(playerid);
	}

	return 1;
}

_BagEquipHandler(playerid)
{
	new itemid = GetPlayerItem(playerid);

	if(!IsValidItem(itemid))
		return 0;

	if(bag_PuttingInBag[playerid])
		return 0;

	new ItemType:itemtype = GetItemType(itemid);

	if(IsItemTypeBag(itemtype))
	{
		if(IsValidItem(bag_PlayerBagID[playerid]))
		{
			new currentbagitem = bag_PlayerBagID[playerid];

			RemovePlayerBag(playerid);
			GivePlayerBag(playerid, itemid);
			GiveWorldItemToPlayer(playerid, currentbagitem, 1);
		}
		else
		{
			if(CallLocalFunction("OnPlayerWearBag", "dd", playerid, itemid))
				return 0;

			GivePlayerBag(playerid, itemid);
		}

		return 0;
	}
	else
	{
		AddItemToPlayer(playerid, itemid);
	}

	return 1;
}

_BagDropHandler(playerid)
{
	if(!IsValidItem(bag_PlayerBagID[playerid]))
		return 0;

	if(IsValidItem(GetPlayerItem(playerid)))
		return 0;

	if(IsValidItem(GetPlayerInteractingItem(playerid)))
		return 0;

	if(CallLocalFunction("OnPlayerRemoveBag", "dd", playerid, bag_PlayerBagID[playerid]))
		return 0;

	new containerid = GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1);

	if(!IsValidContainer(containerid))
		return 0;

	RemovePlayerAttachedObject(playerid, BAG_ATTACH_INDEX);
	CreateItemInWorld(bag_PlayerBagID[playerid], 0.0, 0.0, 0.0, .world = GetPlayerVirtualWorld(playerid), .interior = GetPlayerInterior(playerid));
	GiveWorldItemToPlayer(playerid, bag_PlayerBagID[playerid], 1);
	bag_ContainerPlayer[containerid] = INVALID_PLAYER_ID;
	bag_PlayerBagID[playerid] = INVALID_ITEM_ID;
	bag_TakingOffBag[playerid] = true;

	return 1;
}

_BagRummageHandler(playerid)
{
	new targetid;

	// todo: get player using personal-space

	new
		Float:px,
		Float:py,
		Float:pz,
		Float:tx,
		Float:ty,
		Float:tz,
		Float:tr,
		Float:angle;

	GetPlayerPos(playerid, px, py, pz);
	GetPlayerPos(targetid, tx, ty, tz);
	GetPlayerFacingAngle(targetid, tr);

	angle = absoluteangle(tr - GetAngleToPoint(tx, ty, px, py));

	if(155.0 < angle < 205.0)
	{
		CancelPlayerMovement(playerid);
		bag_OtherPlayerEnter[playerid] = defer bag_EnterOtherPlayer(playerid, targetid);
		break;
	}

	return 1;
}

timer bag_PutItemIn[300](playerid, itemid, containerid)
{
	AddItemToContainer(containerid, itemid, playerid);
	bag_PuttingInBag[playerid] = false;
}

timer bag_EnterOtherPlayer[250](playerid, targetid)
{
	_DisplayBagDialog(playerid, bag_PlayerBagID[targetid], false);
	bag_LookingInBag[playerid] = targetid;
}

PlayerBagUpdate(playerid)
{
	if(IsPlayerConnected(bag_LookingInBag[playerid]))
	{
		if(GetPlayerDist3D(playerid, bag_LookingInBag[playerid]) > 1.0)
		{
			ClosePlayerContainer(playerid);
			CancelSelectTextDraw(playerid);
			bag_LookingInBag[playerid] = -1;
		}
	}
}

_DisplayBagDialog(playerid, itemid, animation)
{
	DisplayContainerInventory(playerid, GetItemArrayDataAtCell(itemid, 1));
	bag_CurrentBag[playerid] = itemid;

	if(animation)
		ApplyAnimation(playerid, "BOMBER", "BOM_PLANT_IN", 4.0, 0, 0, 0, 1, 0);

	else
		CancelPlayerMovement(playerid);
}

hook OnItemAddToInventory(playerid, itemid, slot)
{
	new ItemType:itemtype = GetItemType(itemid);

	if(IsItemTypeBag(itemtype))
		return Y_HOOKS_BREAK_RETURN_1;

	if(IsItemTypeCarry(itemtype))
		return Y_HOOKS_BREAK_RETURN_1;

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerAddToInventory(playerid, itemid, success)
{
	if(success)
	{
		new ItemType:itemtype = GetItemType(itemid);

		if(IsItemTypeBag(itemtype))
			return Y_HOOKS_BREAK_RETURN_1;

		if(IsItemTypeCarry(itemtype))
			return Y_HOOKS_BREAK_RETURN_1;
	}
	else
	{
		new ItemType:itemtype = GetItemType(itemid);

		if(IsItemTypeBag(itemtype))
			return Y_HOOKS_BREAK_RETURN_1;

		if(IsItemTypeCarry(itemtype))
			return Y_HOOKS_BREAK_RETURN_1;

		new
			itemsize = GetItemTypeSize(GetItemType(itemid)),
			freeslots = GetInventoryFreeSlots(playerid);

		ShowActionText(playerid, sprintf(ls(playerid, "CNTEXTRASLO", true), itemsize - freeslots), 3000, 150);
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerCloseContainer(playerid, containerid)
{
	if(IsValidItem(bag_CurrentBag[playerid]))
	{
		ClearAnimations(playerid);
		bag_CurrentBag[playerid] = INVALID_ITEM_ID;
		bag_LookingInBag[playerid] = -1;
	}
}

hook OnPlayerDropItem(playerid, itemid)
{
	if(IsItemTypeBag(GetItemType(itemid)))
	{
		if(bag_TakingOffBag[playerid])
		{
			bag_TakingOffBag[playerid] = false;
			return Y_HOOKS_BREAK_RETURN_1;
		}
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerGiveItem(playerid, targetid, itemid)
{
	if(IsItemTypeBag(GetItemType(itemid)))
	{
		if(bag_TakingOffBag[playerid])
		{
			bag_TakingOffBag[playerid] = false;
			return Y_HOOKS_BREAK_RETURN_1;
		}
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerViewInvOpt(playerid)
{
	if(IsValidItem(bag_PlayerBagID[playerid]) && !IsValidContainer(GetPlayerCurrentContainer(playerid)))
	{
		bag_InventoryOptionID[playerid] = AddInventoryOption(playerid, "Move to bag");
	}
}

hook OnPlayerSelectInvOpt(playerid, option)
{
	if(IsValidItem(bag_PlayerBagID[playerid]) && !IsValidContainer(GetPlayerCurrentContainer(playerid)))
	{
		if(option == bag_InventoryOptionID[playerid])
		{
			new
				containerid,
				slot,
				itemid;
			
			containerid = GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1);
			slot = GetPlayerSelectedInventorySlot(playerid);
			itemid = GetInventorySlotItem(playerid, slot);

			if(!IsValidItem(itemid))
			{
				DisplayPlayerInventory(playerid);
				return Y_HOOKS_CONTINUE_RETURN_0;
			}

			new required = AddItemToContainer(containerid, itemid, playerid);

			if(required > 0)
				ShowActionText(playerid, sprintf(ls(playerid, "BAGEXTRASLO", true), required), 3000, 150);

			DisplayPlayerInventory(playerid);
		}
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerViewCntOpt(playerid, containerid)
{
	if(IsValidItem(bag_PlayerBagID[playerid]) && containerid != GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1))
	{
		bag_InventoryOptionID[playerid] = AddContainerOption(playerid, "Move to bag");
	}
}

hook OnPlayerSelectCntOpt(playerid, containerid, option)
{
	if(IsValidItem(bag_PlayerBagID[playerid]) && containerid != GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1))
	{
		if(option == bag_InventoryOptionID[playerid])
		{
			new
				bagcontainerid,
				slot,
				itemid;

			bagcontainerid = GetItemArrayDataAtCell(bag_PlayerBagID[playerid], 1);
			slot = GetPlayerContainerSlot(playerid);
			itemid = GetContainerSlotItem(containerid, slot);

			if(!IsValidItem(itemid))
			{
				DisplayContainerInventory(playerid, containerid);
				return Y_HOOKS_CONTINUE_RETURN_0;
			}

			new required = AddItemToContainer(bagcontainerid, itemid, playerid);

			if(required > 0)
				ShowActionText(playerid, sprintf(ls(playerid, "BAGEXTRASLO", true), required), 3000, 150);

			DisplayContainerInventory(playerid, containerid);
		}
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnItemAddToContainer(containerid, itemid, playerid)
{
if(GetContainerBagItem(containerid) != INVALID_ITEM_ID)
	{
	if(IsItemTypeCarry(GetItemType(itemid)))
		{
		return Y_HOOKS_BREAK_RETURN_1;
		}
	}

	return Y_HOOKS_CONTINUE_RETURN_0;
}


/*==============================================================================

	Interface

==============================================================================*/


stock IsItemTypeBag(ItemType:itemtype)
{
	if(!IsValidItemType(itemtype))
		return 0;

	return (bag_ItemTypeBagType[itemtype] != -1) ? (true) : (false);
}

stock GetItemBagType(ItemType:itemtype)
{
	if(!IsValidItemType(itemtype))
		return 0;

	return bag_ItemTypeBagType[itemtype];
}

stock GetPlayerBagItem(playerid)
{
	if(!(0 <= playerid < MAX_PLAYERS))
		return INVALID_ITEM_ID;

	return bag_PlayerBagID[playerid];
}

stock GetContainerPlayerBag(containerid)
{
	if(!IsValidContainer(containerid))
		return INVALID_PLAYER_ID;

	return bag_ContainerPlayer[containerid];
}

stock GetContainerBagItem(containerid)
{
	if(!IsValidContainer(containerid))
		return INVALID_ITEM_ID;

	return bag_ContainerItem[containerid];
}

stock GetBagItemContainerID(itemid)
{
	if(!IsItemTypeBag(GetItemType(itemid)))
		return INVALID_CONTAINER_ID;

	return GetItemArrayDataAtCell(itemid, 1);
}
