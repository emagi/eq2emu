# File: `HeroicOp.h`

## Classes

- `HeroicOPStarter`
- `HeroicOPWheel`
- `HeroicOP`
- `MasterHeroicOPList`

## Functions

- `void SetComplete(int8 val) { m_complete = val; }`
- `void SetStage(int8 val) { m_currentStage = val; }`
- `void SetWheel(HeroicOPWheel* val);`
- `void SetStartTime(int32 val) { m_startTime = val; }`
- `void SetTotalTime(float val) { m_totalTime = val; }`
- `void SetTarget(int32 val);`
- `int8 GetComplete() { return m_complete; }`
- `int8 GetStage() { return m_currentStage; }`
- `int32 GetStartTime() { return m_startTime; }`
- `float GetTotalTime() { return m_totalTime; }`
- `int32 GetTarget() { return m_target; }`
- `bool HasShifted() { return m_shifted; }`
- `bool UpdateHeroicOP(int16 icon);`
- `void ResetStage() { m_currentStage = 0; }`
- `void AddStarterChain(HeroicOPStarter* starter);`
- `bool ShiftWheel();`
- `void AddStarter(int8 start_class, HeroicOPStarter* starter);`
- `void AddWheel(int32 starter_id, HeroicOPWheel* wheel);`

## Notable Comments

- /*
- */
- /// <summary>Sets the complete flag for this Heroic OP</summary>
- /// <param name='val'>The value to set the complete flag to, 1 = failed 2 = finished</param>
- /// <summary>Sets the current stage of the starter chain or the wheel chain is at</summary>
- /// <param name='val'>The stage to set this Heroic OP to</param>
- /// <summary>Sets the wheel for this Heroic OP</summary>
- /// <param name='val'>The wheel we are setting the Heroic OP to</param>
- /// <summary>Sets the start time for the wheel</summary>
- /// <param name='val'>Value to set the start time to</param>
