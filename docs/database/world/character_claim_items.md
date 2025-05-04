## Table: `character_claim_items`

**Description:**

Defines `character_claim_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), DEFAULT 0)
- `account_id` (int(10), DEFAULT 0)
- `item_id` (int(10), DEFAULT 0)
- `max_claim` (int(10), DEFAULT 0)
- `curr_claim` (int(10), DEFAULT 0)
- `one_per_char` (int(11), DEFAULT 0)
- `last_claim` (bigint(20), DEFAULT 0)
- `veteran_reward_time` (bigint(20), DEFAULT 0 COMMENT 'account age in seconds') → account age in seconds

**Primary Keys:**
- id