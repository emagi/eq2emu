### Command: /knowledgewindow_sort book sort_by order pattern

**Handler Macro:** COMMAND_KNOWLEDGEWINDOWSORT

**Handler Value:** 511

**Required Status:** 0

**Arguments:**
- `arg[0]`: `int book`
- `arg[1]`: `int sort_by`
- `arg[2]`: `int order`
- `arg[3]`: `int pattern`

**Notes:**
- Sorts the spell book based on the options provided.  A minimum of 3 arguments is required for KoS and earlier client versions (561), later versions will require all 4 arguments.
- book: 0 - spells, 1 - combat, 2 - abilities, 3 - tradeskill
- sort_by: 0 - alpha, 1 - level, 2 - category
- order: 0 - ascending, 1 - descending
- pattern: 0 - zigzag, 1 - down, 2 - across