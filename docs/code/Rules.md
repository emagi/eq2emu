# File: `Rules.h`

## Classes

- `Rule`
- `RuleSet`
- `RuleManager`

## Functions

- `void SetValue(const char *value) {strncpy(this->value, value, sizeof(this->value));}`
- `int32 GetCategory() {return category;}`
- `int32 GetType() {return type;}`
- `int8 GetInt8() {return (int8)atoul(value);}`
- `int16 GetInt16() {return (int16)atoul(value);}`
- `int32 GetInt32() {return (int32)atoul(value);}`
- `int64 GetInt64() {return (int64)atoi64(value);}`
- `sint8 GetSInt8() {return (sint8)atoi(value);}`
- `sint16 GetSInt16() {return (sint16)atoi(value);}`
- `sint32 GetSInt32() {return (sint32)atoi(value);}`
- `sint64 GetSInt64() {return (sint64)atoi64(value);}`
- `bool GetBool() {return atoul(value) > 0 ? true : false;}`
- `float GetFloat() {return atof(value);}`
- `char GetChar() {return value[0];}`
- `void CopyRulesInto(RuleSet *in_rule_set);`
- `void SetID(int32 id) {this->id = id;}`
- `void SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}`
- `int32 GetID() {return id;}`
- `void AddRule(Rule *rule);`
- `void ClearRules();`
- `void Init();`
- `void Flush(bool reinit=false);`
- `void LoadCodedDefaultsIntoRuleSet(RuleSet *rule_set);`
- `bool AddRuleSet(RuleSet *rule_set);`
- `int32 GetNumRuleSets();`
- `void ClearRuleSets();`
- `bool SetGlobalRuleSet(int32 rule_set_id);`
- `bool SetZoneRuleSet(int32 zone_id, int32 rule_set_id);`
- `void ClearZoneRuleSets();`

## Notable Comments

- /*
- */
- /* CLIENT */
- /* FACTION */
- /* GUILD */
- /* PLAYER */
- /* PVP */
- /* COMBAT */
- /* SPAWN */
- //SpeedRatio,
