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
}

hook OnPlayerSpawn(playerid) {
	new Item:itemid = CreateItem(item_Backpack, 302.0, 1805.0, 16.8);
	new ret = GivePlayerBag(playerid, itemid);
	log("GivePlayerBag called", _i("ret", ret));
}

CMD:bag(playerid, params[]) {
	return !ShowPlayerBag(playerid);
}

CMD:remove(playerid, params[]) {
	new ret = RemovePlayerBag(playerid);
	log("RemovePlayerBag called", _i("ret", ret));
	return 1;
}

CMD:destroy(playerid, params[]) {
	new ret = DestroyPlayerBag(playerid);
	log("DestroyPlayerBag called", _i("ret", ret));
	return 1;
}
