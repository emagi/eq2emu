## Table: `collection_rewards`

**Description:**

Defines `collection_rewards` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `collection_id` (int(10), NOT NULL, DEFAULT 0)
- `reward_type` (enum('None','Item','Selectable','Coin','XP'), NOT NULL, DEFAULT 'None')
- `reward_value` (bigint(20), NOT NULL, DEFAULT 0)
- `reward_quantity` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_collection_rewards` (`collection_id`)
- CONSTRAINT `FK_collection_rewards` FOREIGN KEY (`collection_id`) REFERENCES `collections` (`id`) ON DELETE CASCADE ON UPDATE CASCADE