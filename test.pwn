#include "bag.inc"

#include <test-boilerplate-items>
#include <zcmd>
#include <YSI\y_hooks>


hook OnScriptInit() {
	logger_debug("inventory-dialog", true);
	logger_debug("contianer-dialog", true);
	logger_debug("bag", true);

	DefineBagType(item_Backpack, 11,
		-0.110900, -0.073500, 0.000000,
		0.000000, 0.000000, 0.000000,
		1.000000, 1.000000, 1.000000);

	// AddItemToPlayer(playerid, Item:itemid, useinventory = false, playeraction = true)
	// IsItemTypeBag(ItemType:itemtype)
	// BagType:GetItemBagType(ItemType:itemtype)
	// GetPlayerBagItem(playerid)
	// GetContainerPlayerBag(Container:containerid)
	// Item:GetContainerBagItem(Container:containerid)
	// GetBagItemContainerID(itemid, &Container:containerid)
}

CMD:givebag(playerid, params[]) {
	new Item:itemid = CreateItem(item_Backpack, 302.0, 1805.0, 16.8);
	new ret = GivePlayerBag(playerid, itemid);
	log("GivePlayerBag called", _i("ret", ret));
	return 1;
}

CMD:bag(playerid, params[]) {
	return !ShowPlayerBag(playerid);
}

CMD:remove(playerid, params[]) {
	new ret = RemovePlayerBag(playerid);
	log("RemovePlayerBag called", _i("ret", ret));
	return 1;
}
