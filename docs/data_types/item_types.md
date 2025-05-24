# EverQuest 2 Item Types

Duplicate IDs are intentional and reflect legacy client behavior.

| ID | Constant Name              | Display Name        | Description (best guess)                                  |
|----|----------------------------|---------------------|-----------------------------------------------------------|
| 0  | ITEM_TYPE_NORMAL           | Normal              | Generic item with no special use.                         |
| 1  | ITEM_TYPE_WEAPON           | Weapon              | Melee weapons such as swords, maces, axes, etc.           |
| 2  | ITEM_TYPE_RANGED           | Ranged Weapon       | Bows and other primary ranged weapons.                    |
| 3  | ITEM_TYPE_ARMOR            | Armor               | Wearable armor pieces (helm, chest, legs, etc.).          |
| 4  | ITEM_TYPE_SHIELD           | Shield              | Shields for blocking damage.                              |
| 5  | ITEM_TYPE_BAG              | Bag                 | Inventory containers that hold other items.               |
| 6  | ITEM_TYPE_SKILL            | Skill Book          | Teaches or upgrades a player skill.                       |
| 7  | ITEM_TYPE_RECIPE           | Recipe Book         | Unlocks tradeskill or spell recipes.                      |
| 8  | ITEM_TYPE_FOOD             | Food / Drink        | Consumables that grant short buffs or restore stats.      |
| 9  | ITEM_TYPE_BAUBLE           | Bauble              | Click-to-use items with an activated effect.              |
| 10 | ITEM_TYPE_HOUSE            | House Item          | Furniture or trophy for player housing.                   |
| 11 | ITEM_TYPE_THROWN           | Thrown Weapon       | Daggers, shurikens, and other single-use thrown items.    |
| 12 | ITEM_TYPE_HOUSE_CONTAINER  | House Container     | Storage placed inside houses (e.g., vault, strongbox).    |
| 13 | ITEM_TYPE_ADORNMENT        | Adornment           | Permanent stat-boosting attachment for gear.              |
| 14 | ITEM_TYPE_GENERIC_ADORNMENT| Generic Adornment   | Older or non-specific adornment type.                     |
| 16 | ITEM_TYPE_PROFILE          | Profile             | Appearance set or cosmetic profile item.                  |
| 17 | ITEM_TYPE_PATTERN          | Armor Pattern       | Pattern traded for class-specific armor.                  |
| 18 | ITEM_TYPE_ARMORSET         | Armor Set Token     | Token representing a complete armor set.                  |
| 18 | ITEM_TYPE_ITEMCRATE        | Item Crate          | Crate containing multiple items; shares ID 18.            |
| 19 | ITEM_TYPE_BOOK             | Lore Book           | Readable book; may start quests or provide lore.          |
| 20 | ITEM_TYPE_DECORATION       | Decoration          | Non-functional decorative item (house or world).          |
| 21 | ITEM_TYPE_DUNGEON_MAKER    | Dungeon Maker Item  | Piece used in player-built dungeons.                      |
| 22 | ITEM_TYPE_MARKETPLACE      | Marketplace Item    | Purchased via Station Cash / real-money store.            |

*Note:* `ITEM_TYPE_ARMORSET` and `ITEM_TYPE_ITEMCRATE` both use ID `18`; this mirrors the original client data where multiple labels referred to the same numeric type.
