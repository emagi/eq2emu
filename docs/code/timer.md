# File: `timer.h`

## Classes

- `Timer`
- `BenchTimer`

## Functions

- `int gettimeofday (timeval *tp, ...);`
- `bool Check(bool iReset = true);`
- `void Enable();`
- `void Disable();`
- `void Start(int32 set_timer_time=0, bool ChangeResetTimer = true);`
- `void SetTimer(int32 set_timer_time=0);`
- `int32 GetRemainingTime();`
- `int32 GetElapsedTime();`
- `void Trigger();`
- `void SetAtTrigger(int32 set_at_trigger, bool iEnableIfDisabled = false);`
- `void reset() { start_time = clock::now(); }`
- `double elapsed() { return std::chrono::duration<double>(clock::now() - start_time).count(); }`

## Notable Comments

- /*
- */
- // Disgrace: for windows compile
- // Tells the timer to be more acurate about happening every X ms.
- // Instead of Check() setting the start_time = now,
- // it it sets it to start_time += timer_time
- //	static int32 current_time;
- //	static int32 last_time;
- // this is seconds
