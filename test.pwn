#include "bag.inc"

main() {
	new ItemType:itemType = DefineItemType("Medkit", "Medkit", 11736, 1, 0.0, 0.0, 0.0, 0.004, 0.197999, 0.038000, 0.021000,  79.700012, 0.000000, 90.899978);

	DefineBagType(itemType, 12,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	);

	GivePlayerBag(0, 0);
	RemovePlayerBag(0);
	DestroyPlayerBag(0);
	AddItemToPlayer(0, 0);
	IsItemTypeBag(ItemType:0);
	GetItemBagType(ItemType:0);
	GetPlayerBagItem(0);
	GetContainerPlayerBag(0);
	GetContainerBagItem(0);
	new containerid;
	GetBagItemContainerID(0, containerid);
}
