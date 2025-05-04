## Table: `claim_items`

**Description:**

Defines `claim_items` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(11), NOT NULL, DEFAULT 0)
- `max_claim` (int(11), NOT NULL, DEFAULT 0)
- `one_per_char` (int(10), DEFAULT 0)
- `veteran_reward_time` (bigint(20), DEFAULT 0)
- `comment` (text, DEFAULT NULL)

**Primary Keys:**
- id