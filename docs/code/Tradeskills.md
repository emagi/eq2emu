# File: `Tradeskills.h`

## Classes

- `Player`
- `Spawn`
- `Recipe`
- `Client`
- `TradeskillEvent`
- `Tradeskill`
- `TradeskillMgr`
- `MasterTradeskillEventsList`

## Functions

- `void Process();`
- `void BeginCrafting(Client* client, vector<pair<int32, int16>> components);`
- `void StopCrafting(Client* client, bool lock = true);`
- `bool IsClientCrafting(Client* client);`
- `void CheckTradeskillEvent(Client* client, int16 icon);`
- `void ReadLock(const char* function = (const char*)0, int32 line = 0) { m_tradeskills.readlock(function, line); }`
- `void ReleaseReadLock(const char* function = (const char*)0, int32 line = 0) { m_tradeskills.releasereadlock(function, line); }`
- `int32 GetTechniqueSuccessAnim(int16 version, int32 technique);`
- `int32 GetTechniqueFailureAnim(int16 version, int32 technique);`
- `int32 GetTechniqueIdleAnim(int16 version, int32 technique);`
- `int32 GetMissTargetAnim(int16 version);`
- `int32 GetKillMissTargetAnim(int16 version);`
- `void SetClientIdleVisualState(Client* client, Tradeskill* ts);`
- `void SendItemCreationUI(Client* client, Recipe* recipe);`
- `void AddEvent(TradeskillEvent* tradeskillEvent);`
- `int32 Size();`

## Notable Comments

- /*
- */
- /// <summary>Determines if an update is needed if so send one and stop crafting if finished</summary>
- /// <summary>Starts the actual crafting process</summary>
- /// <param name='client'>Client that is crafting</param>
- /// <param name='components'>List of items the player is using to craft</param>
- /// <summary>Stops the crafting process</summary>
- /// <param name='client'>Client that stopped crafting</param>
- /// <param name='lock'>Does the list need a mutex lock? default = true</param>
- /// <summary>Checks to see if the given client is crafting</summary>
