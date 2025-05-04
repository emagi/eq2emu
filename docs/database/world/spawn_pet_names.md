## Table: `spawn_pet_names`

**Description:**

Defines `spawn_pet_names` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `pet_name` (varchar(64), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `PetNameIDX` (`pet_name`)