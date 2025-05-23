### Function: RemoveWidgetFromZoneMap(zone, widget_id)

**Description:**
Removes the widget from the 3d world space in the client and server.  Must be used in preinit_zone_script

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `widget_id` (uint32) - Integer value `widget_id`.

**Returns:** None.

**Example:**

```lua
-- From ZoneScripts/ThunderingSteppes.lua
function preinit_zone_script(Zone)
    RemoveWidgetFromZoneMap(Zone, 2909687498)
    RemoveWidgetFromZoneMap(Zone, 506885674)
    RemoveWidgetFromZoneMap(Zone, 800135876)
    RemoveWidgetFromZoneMap(Zone, 2944954936)
    
end
```
