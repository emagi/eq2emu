### Command: /grid [spawns|boundary] [grid_id]

**Handler Macro:** COMMAND_GRID

**Handler Value:** 271

**Required Status:** 200

**Arguments:**
- `arg[0]`: `string type`
- `arg[1]`: `int grid_id`

**Notes:**
- /grid - with no arguments, provides players current grid id.
- /grid spawns grid_id - lists the spawns in the current grid, grid_id optionally can be provided to get spawns listed in that grid.
- /grid boundary grid_id - shows the map boundary for the current grid.
