# File: `mob_movement_manager.h`

## Classes

- `Mob`
- `Client`
- `RotateCommand`
- `MovementCommand`
- `MobMovementEntry`
- `PlayerPositionUpdateServer_Struct`
- `MobMovementManager`
- `Implementation`

## Functions

- `void Process();`
- `void AddMob(Entity *mob);`
- `void RemoveMob(Entity *mob);`
- `void AddClient(Client *client);`
- `void RemoveClient(Client *client);`
- `void RotateTo(Entity *who, float to, MobMovementMode mob_movement_mode = MovementRunning);`
- `void Teleport(Entity *who, float x, float y, float z, float heading);`
- `void NavigateTo(Entity *who, float x, float y, float z, MobMovementMode mode = MovementRunning, bool overrideDistance=false);`
- `void StopNavigation(Entity *who);`
- `void DisruptNavigation(Entity* who);`
- `float FixHeading(float in);`
- `void ClearStats();`
- `bool IsRunningCommandProcess() {`
- `bool SetCommandProcess(bool status) {`
- `void UpdatePath(Entity *who, float x, float y, float z, MobMovementMode mob_movement_mode);`
- `void UpdatePathGround(Entity *who, float x, float y, float z, MobMovementMode mode);`
- `void UpdatePathUnderwater(Entity *who, float x, float y, float z, MobMovementMode movement_mode);`
- `void UpdatePathBoat(Entity *who, float x, float y, float z, MobMovementMode mode);`
- `void PushTeleportTo(MobMovementEntry &ent, float x, float y, float z, float heading);`
- `void PushMoveTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode);`
- `void PushSwimTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode);`
- `void PushRotateTo(MobMovementEntry &ent, Entity *who, float to, MobMovementMode mob_movement_mode);`
- `void PushStopMoving(MobMovementEntry &mob_movement_entry);`
- `void PushEvadeCombat(MobMovementEntry &mob_movement_entry);`
- `void HandleStuckBehavior(Entity *who, float x, float y, float z, MobMovementMode mob_movement_mode);`

## Notable Comments

- /*
