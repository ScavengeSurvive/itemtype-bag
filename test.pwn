#include "bag.inc"

#include <test-boilerplate-items>
#include <zcmd>
#include <YSI\y_hooks>


hook OnScriptInit() {
	logger_debug("bag", true);
	DefineBagType(item_Backpack, 12,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	);
}


// GivePlayerBag(0, 0);
// RemovePlayerBag(0);
// DestroyPlayerBag(0);
// AddItemToPlayer(0, 0);
// IsItemTypeBag(ItemType:0);
// GetItemBagType(ItemType:0);
// GetPlayerBagItem(0);
// GetContainerPlayerBag(0);
// GetContainerBagItem(0);
// new containerid;
// GetBagItemContainerID(0, containerid);
