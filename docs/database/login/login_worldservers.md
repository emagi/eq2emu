## Table: `login_worldservers`

**Description:**

Defines `login_worldservers` table in the Login database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(250), NOT NULL, DEFAULT '')
- `disabled` (tinyint(3), NOT NULL, DEFAULT 0)
- `account` (varchar(30), NOT NULL, DEFAULT '')
- `chat_shortname` (varchar(20), NOT NULL, DEFAULT '')
- `description` (text, NOT NULL)
- `server_type` (varchar(15), NOT NULL, DEFAULT '0')
- `password` (varchar(256), NOT NULL, DEFAULT '')
- `serverop` (varchar(64), NOT NULL, DEFAULT '')
- `lastseen` (int(10), NOT NULL, DEFAULT 0)
- `admin_id` (int(11), NOT NULL, DEFAULT 0)
- `greenname` (tinyint(1), NOT NULL, DEFAULT 0)
- `showdown` (tinyint(4), NOT NULL, DEFAULT 0)
- `chat` (varchar(20), NOT NULL, DEFAULT '0')
- `note` (tinytext, NOT NULL)
- `ip_address` (varchar(50), NOT NULL, DEFAULT '0')
- `reset_needed` (tinyint(3), NOT NULL, DEFAULT 0)
- `created_date` (int(10), NOT NULL, DEFAULT 0)
- `hide_status` (tinyint(1), NOT NULL, DEFAULT 0)
- `hide_details` (tinyint(1), NOT NULL, DEFAULT 0)
- `hide_admin` (tinyint(1), NOT NULL, DEFAULT 0)
- `hide_serverlist` (tinyint(1), NOT NULL, DEFAULT 0)
- `server_url` (varchar(250), DEFAULT '')
- `server_category` (enum('Standard','Preferred','Premium','Development'), DEFAULT 'Standard')
- `server_admin` (varchar(64), NOT NULL, DEFAULT '')
- `server_sticky` (tinyint(1), NOT NULL, DEFAULT 0)
- `gm_list` (text, DEFAULT NULL)
- `reset_zone_descriptions` (tinyint(1), NOT NULL, DEFAULT 0)
- `reset_login_appearances` (tinyint(1), NOT NULL, DEFAULT 0)
- `login_version` (varchar(32), NOT NULL, DEFAULT '0')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `account` (`account`)
- UNIQUE KEY `NameIDX` (`name`)