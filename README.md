# Bag Items

Provides an item type definition for wearable containers so players can carry multiple items with them.

## Installation

Simply install to your project:

```bash
sampctl package install ScavengeSurvive/bag
```

Include in your code and begin using the library:

```pawn
#include <library>
```

## Usage

Use `DefineBagType` to define the given ItemType as a bag item with the specified name and attachment offsets. The attachment offsets are used for attaching the bag to the spine bone when the player wears the bag.

You can use `GivePlayerBag` to make a player wear a bag item and `RemovePlayerBag` to remove it.

`DestroyPlayerBag` destroys the player's currently worn bag item along with all the contained items.

While giving items to a player, you can use the helper function `AddItemToPlayer` which determines whether to place the given item into the player's inventory or their bag.

## Testing

To test, simply run the package:

```bash
sampctl package run
```

And connect to `localhost:7777` to test.
