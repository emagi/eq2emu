### Function: SetLocationProximityFunction(zone, x, y, z, max_variation, in_range_function, leaving_range_function)

**Description:**
Creates a ZoneScript proximity function for in range and leaving range respectively.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `max_variation` (int32) - Integer value `max_variation`.
- `in_range_function` (int32) - Distance `in_range_function`.
- `leaving_range_function` (int32) - Distance `leaving_range_function`.

**Returns:** None.

**Example:**

```lua
-- From ZoneScripts/Antonica.lua
function init_zone_script(Zone)
	SetLocationProximityFunction(Zone, -2128.93, -28.4328, 614.081, 10, "GriffonTower", "Leave")  -- Steppes from Qeynos
	SetLocationProximityFunction(Zone, -904.472, -7.23051, -610.321, 10, "GriffonTower", "Leave")  -- Oracle from Qeynos
	SetLocationProximityFunction(Zone, 337.246, -17.3142, 537.882, 10, "GriffonTower", "Leave")  -- Qeynos from Steppes
    SetLocationProximityFunction(Zone, -912.659, -7.21881, -599.911, 10, "GriffonTower", "Leave")  -- Oracle from Steppes
	SetLocationProximityFunction(Zone, 327.727, -17.3058, 529.95, 10, "GriffonTower", "Leave")  -- Qeynos from Oracle
	SetLocationProximityFunction(Zone, -2136.6, -28.5276, 608.087, 10, "GriffonTower", "Leave")  -- Steppese from oracle
	
    SetLocationProximityFunction(Zone,127.20, -24.88, 468.20, 60, "CraterLake","LeaveLocation")
    SetLocationProximityFunction(Zone,-102.51, -18.43, 100.64, 95, "CentralFarmlands","LeaveLocation")
    SetLocationProximityFunction(Zone,-123.97, -16.37, 585.85, 35, "QeynosHills","LeaveLocation")
    SetLocationProximityFunction(Zone,-442.50, 2.15, 551.88, 25, "Claymore","LeaveLocation")
    SetLocationProximityFunction(Zone,-503.16, -18.23, 396.36, 95, "QeynosPlains","LeaveLocation")
    SetLocationProximityFunction(Zone,-753.87, -3.03, 592.89, 160, "ArcherWoods","LeaveLocation")
    SetLocationProximityFunction(Zone,-1191.42, -20.26, 261.38, 70, "QeynosHighway","LeaveLocation")
    SetLocationProximityFunction(Zone,-1299.50, 38.29, 44.79, 20, "BellsofVhalen","LeaveLocation")
    SetLocationProximityFunction(Zone,-1506.60, -14.37, -11.08, 55, "GladeoftheCoven","LeaveLocation")
    SetLocationProximityFunction(Zone,-1567.28, 2.28, 609.58, 90, "BrambleWoods","LeaveLocation")
    SetLocationProximityFunction(Zone,424.28, -43.66, 733.90, 50, "ClodwindPoint","LeaveLocation")
    SetLocationProximityFunction(Zone,348.00, -9.39, 809.95, 35, "MarinersIsland","LeaveLocation")
    SetLocationProximityFunction(Zone,188.45, -19.38, 863.55, 35, "CastawayIsland","LeaveLocation")
    SetLocationProximityFunction(Zone,5.57, -9.66, 1028.04, 35, "QeynosSheperdsIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,-796.74, -28.08, 1029.75, 30, "WoodedIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,-917.46, -17.69, 1052.23, 30, "MemorialIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,-327,-14,-501 , 55, "WatchtowerPlains","LeaveLocation")
    SetLocationProximityFunction(Zone,326.08,-35.05,-517.55, 30, "MistyIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,241.24, -29.62, -682.48, 25, "BridgewayIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,334.98, -21.37, -621.86, 30, "AbandonedIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,361.84, -19.69, -664.18, 18, "TombofVarsoon","LeaveLocation")
    SetLocationProximityFunction(Zone,382.58, -31.81, -798.15, 25, "LonelyIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,382.58, -31.81, -798.15, 20, "TravelersIsle","LeaveLocation")
    SetLocationProximityFunction(Zone,-376.53, -0.70, -369.36, 85, "ArdentHills","LeaveLocation")
    SetLocationProximityFunction(Zone,-555.33, -12.85, -319.13, 18, "KeepoftheArdentNeedle","LeaveLocation")
    SetLocationProximityFunction(Zone,-804.55, -32.01, -422.03, 50, "TowerLands","LeaveLocation")
    SetLocationProximityFunction(Zone,-1033.81, 14.12, -630.33, 20, "ToweroftheOracles","LeaveLocation")
    SetLocationProximityFunction(Zone,-1410.68, -13.88, -742.63, 100, "RuinsofCaltorsis","LeaveLocation")
    SetLocationProximityFunction(Zone,-1865, -38, -550, 70, "WindstalkerVillage","LeaveLocation")
    SetLocationProximityFunction(Zone,-2182.37, -11.79, -818.23, 100, "WindstalkerHighlands","LeaveLocation")
    SetLocationProximityFunction(Zone,-2262.04, -24.72, -570.99, 90, "CoastalGrove","LeaveLocation")
    SetLocationProximityFunction(Zone,-2048.99, 15.74, -483.06, 20, "FangbreakerKeep","LeaveLocation")
    SetLocationProximityFunction(Zone,-1851.41, -22.03, -194.27, 90, "NorthernFarmlands","LeaveLocation")
    SetLocationProximityFunction(Zone,-2104, -43, 438, 20, "KeepoftheGnollSlayer","LeaveLocation")
    SetLocationProximityFunction(Zone,-1755, -14, 521, 80, "GnollslayerHighlands","LeaveLocation")
    SetLocationProximityFunction(Zone,-1973.00, -20.08, 640.00, 80, "ScarecrowFields","LeaveLocation")
    SetLocationProximityFunction(Zone,-2082.70, 47.26, 1029.12, 100, "HiddenVale","LeaveLocation")
	
end
```
