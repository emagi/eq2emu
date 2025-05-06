# File: `Widget.h`

## Classes

- `Widget`

## Functions

- `bool	IsWidget(){ return true; }`
- `int32	GetWidgetID();`
- `void	SetWidgetID(int32 val);`
- `void	SetWidgetX(float val);`
- `float	GetWidgetX();`
- `void	SetWidgetY(float val);`
- `float	GetWidgetY();`
- `void	SetWidgetZ(float val);`
- `float	GetWidgetZ();`
- `void	SetIncludeLocation(bool val);`
- `bool	GetIncludeLocation();`
- `void	SetIncludeHeading(bool val);`
- `bool	GetIncludeHeading();`
- `void	SetWidgetIcon(int8 val);`
- `void	HandleTimerUpdate();`
- `void	OpenDoor();`
- `void	CloseDoor();`
- `void	HandleUse(Client* client, string command, int8 overrideWidgetType=0xFF);`
- `float	GetOpenHeading();`
- `void	SetOpenHeading(float val);`
- `float	GetClosedHeading();`
- `void	SetClosedHeading(float val);`
- `float	GetOpenY();`
- `void	SetOpenY(float val);`
- `float	GetCloseY();`
- `void	SetCloseY(float val);`
- `float GetOpenX(){return open_x;}`
- `float GetOpenZ(){return open_z;}`
- `float GetCloseX(){return close_x;}`
- `float GetCloseZ(){return close_z;}`
- `void SetOpenX(float x){open_x = x;}`
- `void SetOpenZ(float z){open_z = z;}`
- `void SetCloseX(float x){close_x = x;}`
- `void SetCloseZ(float z){close_z = z;}`
- `int8	GetWidgetType();`
- `void	SetWidgetType(int8 val);`
- `bool	IsOpen();`
- `int32	GetActionSpawnID();`
- `void	SetActionSpawnID(int32 id);`
- `int32	GetLinkedSpawnID();`
- `void	SetLinkedSpawnID(int32 id);`
- `void	SetOpenSound(const char* name);`
- `void	SetCloseSound(const char* name);`
- `void	SetOpenDuration(int16 val);`
- `int16	GetOpenDuration();`
- `void	ProcessUse(Spawn* caller=nullptr);`
- `void	SetHouseID(int32 val) { m_houseID = val; }`
- `int32	GetHouseID() { return m_houseID; }`
- `void	SetMultiFloorLift(bool val) { multi_floor_lift = val; }`
- `bool	GetMultiFloorLift() { return multi_floor_lift; }`
- `return string("Door");`
- `return string("Lift");`
- `return string("Generic");`

## Notable Comments

- /*
- */
