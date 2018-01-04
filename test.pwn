#include "bag.inc"

main() {
	new ItemType:itemType = DefineItemType("Medkit", "Medkit", 11736, 1, 0.0, 0.0, 0.0, 0.004, 0.197999, 0.038000, 0.021000,  79.700012, 0.000000, 90.899978);

	DefineBagType("name",
		itemType,
		12,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0);
}
