## Table: `failed_jobs`

**Description:**

Defines `failed_jobs` table in the World database.

**Columns:**
- `id` (bigint(20), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `connection` (text, NOT NULL)
- `queue` (text, NOT NULL)
- `payload` (longtext, NOT NULL)
- `exception` (longtext, NOT NULL)
- `failed_at` (timestamp, NOT NULL, DEFAULT current_timestamp())

**Primary Keys:**
- id