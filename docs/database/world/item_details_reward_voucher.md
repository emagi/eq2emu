## Table: `item_details_reward_voucher`

**Description:**

Defines `item_details_reward_voucher` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `voucher_item_id` (int(10), NOT NULL, DEFAULT 0)
- `reward_item_id` (int(10), DEFAULT NULL)
- `soe_item_id` (int(10), NOT NULL, DEFAULT 0)
- `soe_item_crc` (int(10), NOT NULL, DEFAULT 0)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `name` (varchar(255), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `VoucherUIDX` (`voucher_item_id`,`soe_item_id`)
- KEY `FK_item_details_reward_voucher_items` (`reward_item_id`)
- CONSTRAINT `FK_item_details_reward_voucher_items` FOREIGN KEY (`reward_item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_item_details_rewardvoucher` FOREIGN KEY (`voucher_item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE