### Command: /castspell [spellid|spellname] [tier] [self]

**Handler Macro:** COMMAND_CASTSPELL

**Handler Value:** 509

**Required Status:** 200

**Arguments:**
- `arg[0]`: `int spellid`
- `arg[0]`: `string spellname`
- `arg[1]`: `int tier`
- `arg[2]`: `int self`

**Notes:**
- /castspell spellid and /castspell spellname supported.  `spellid` casts the spell where `spellname` is a search function to get a list of spell ids by spell name wildcard.
- Casts the spellid presented on yourself or target based on the target of the spell and your current target.
- Tier and self are optional, both numerical, Third argument `self` = 1 means self spell.  Eg. /castspell 1234 1 `1`
