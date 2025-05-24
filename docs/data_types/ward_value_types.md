# Ward Value Types

Each key is supplied to the Lua interface via `SetInt32Value` (integer) or `SetBooleanValue` (boolean).

| Key String                       | Lua Value Type | Description (best guess)                                                        |
|----------------------------------|----------------|---------------------------------------------------------------------------------|
| damageleft                       | int32          | Remaining ward hit-points still able to absorb damage.                          |
| basedamage                       | int32          | Original (maximum) ward value before any damage was applied.                    |
| keepward                         | boolean        | If `true`, the ward persists after reaching zero until its duration expires.    |
| wardtype                         | int32          | Enum ID designating the ward’s category (physical, magical, elemental, etc.).   |
| dmgabsorptionpct                 | int32          | Percentage of incoming damage that the ward can absorb.                         |
| dmgabsorptionmaxhealthpct        | int32          | Maximum damage absorbed per hit, expressed as a percentage of caster’s health.  |
| redirectdamagepercent            | int32          | Percentage of damage redirected from target to ward bearer (or another target). |
| lastredirectdamage               | int32          | Most recent damage amount redirected by the ward.                               |
| lastabsorbeddamage               | int32          | Most recent damage amount absorbed by the ward.                                 |
| hitcount                         | int32          | Number of hits the ward has absorbed so far.                                    |
| maxhitcount                      | int32          | Maximum number of hits the ward can absorb before expiring.                     |
