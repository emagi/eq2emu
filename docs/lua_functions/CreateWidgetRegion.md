### Function: CreateWidgetRegion(zone, version, region_name, env_name, grid_id, widget_id, dist)

**Description:**
Creates a new region to track a grid and/or widget id against a region_name and/or env_name.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `version` (uint32) - Integer value `version`.
- `region_name` (string) - String `region_name`.
- `env_name` (string) - String `env_name`.
- `grid_id` (uint32) - Integer value `grid_id`.
- `widget_id` (uint32) - Integer value `widget_id`.
- `dist` (float) - float value `dist`.

**Returns:** None.

**Example:**

```lua
-- From ZoneScripts/IsleRefuge1.lua
function init_zone_script(Zone)
CreateWidgetRegion(Zone, 0, "TestRegion", "", 924281492, 4117633379, 2.0)
end
```
