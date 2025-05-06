# File: `Achievements.h`

## Classes

- `AchievementRewards`
- `AchievementRequirements`
- `AchievementUpdateItems`
- `Achievement`
- `AchievementUpdate`
- `MasterAchievementList`
- `PlayerAchievementList`
- `PlayerAchievementUpdateList`

## Functions

- `void SetID(int32 id) {this->id = id;}`
- `void SetTitle(const char *title) {strncpy(this->title, title, sizeof(this->title));}`
- `void SetUncompletedText(const char *uncompleted_text) {strncpy(this->uncompleted_text, uncompleted_text, sizeof(this->uncompleted_text));}`
- `void SetCompletedText(const char *completed_text) {strncpy(this->completed_text, completed_text, sizeof(this->completed_text));}`
- `void SetCategory(const char *category) {strncpy(this->category, category, sizeof(this->category));}`
- `void SetExpansion(const char *expansion) {strncpy(this->expansion, expansion, sizeof(this->expansion));}`
- `void SetIcon(int16 icon) {this->icon = icon;}`
- `void SetPointValue(int32 point_value) {this->point_value = point_value;}`
- `void SetQtyReq(int32 qty_req) {this->qty_req = qty_req;}`
- `void SetHide(bool hide) {this->hide = hide;}`
- `void SetUnknown3a(int32 unknown3a) {this->unknown3a = unknown3a;}`
- `void SetUnknown3b(int32 unknown3b) {this->unknown3b = unknown3b;}`
- `void AddAchievementRequirement(struct AchievementRequirements *requirements);`
- `void AddAchievementReward(struct AchievementRewards *reward);`
- `int32 GetID() {return id;}`
- `int16 GetIcon() {return icon;}`
- `int32 GetPointValue() {return point_value;}`
- `int32 GetQtyReq() {return qty_req;}`
- `bool GetHide() {return hide;}`
- `int32 GetUnknown3a() {return unknown3a;}`
- `int32 GetUnknown3b() {return unknown3b;}`
- `void SetID(int32 id) {this->id = id;}`
- `void SetCompletedDate(int32 completed_date) {this->completed_date = completed_date;}`
- `void AddAchievementUpdateItems(struct AchievementUpdateItems *update_items);`
- `int32 GetID() {return id;}`
- `int32 GetCompletedDate() {return completed_date;}`
- `bool AddAchievement(Achievement *achievement);`
- `void ClearAchievements();`
- `int32 Size();`
- `void CreateMasterAchievementListPacket();`
- `bool AddAchievement(Achievement *achievement);`
- `void ClearAchievements();`
- `int32 Size();`
- `bool AddAchievementUpdate(AchievementUpdate *achievement_update);`
- `void ClearAchievementUpdates();`
- `int32 Size();`

## Notable Comments

- /*
- */
