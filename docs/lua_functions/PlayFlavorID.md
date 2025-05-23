### Function: PlayFlavorID(spawn, type, id, index, player, language)

**Description:**
Uses the database dialog_play_flavors and dialog_play_voices.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `type` (int32) - Integer value `type`.
- `id` (uint32) - Integer value `id`.
- `index` (int32) - Integer value `index`.
- `player` (Spawn) - Spawn object representing `player`.
- `language` (int32) - Integer value `language`.

**Returns:** None.

**Example:**

```sql
insert into voiceovers set type_id=2,id=100,indexed=1,mp3_string="voiceover/english/gnoll_base_1/ft/gnoll/gnoll_base_1_2_garbled_2f8caa7b.mp3", text_string="Krovel grarggt ereverrrn", key1=2385604574, key2=3717589402, garbled=1,garble_link_id=1;

insert into voiceovers set type_id=2,id=100,indexed=1,mp3_string="voiceover/english/sean_wellfayer/qey_harbor/100_qst_sean_wellfayer_multhail1_5dca659c.mp3", text_string="I don't think fishing interests you.  Perhaps you should be on your way!", key1=1997164956, key2=747011072, garbled=0,garble_link_id=1;
```

```lua
PlayFlavorID(NPC, 2, 100, 1, nil, 18)
```
