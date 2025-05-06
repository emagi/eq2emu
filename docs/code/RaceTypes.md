# File: `RaceTypes.h`

## Classes

- `RaceTypeStructure`
- `MasterRaceTypeList`

## Functions

- `bool AddRaceType(int16 model_id, int16 race_type_id, const char* category, const char* subcategory, const char* modelname, bool allow_override = false);`
- `int16 GetRaceType(int16 model_id);`
- `int16 GetRaceBaseType(int16 model_id);`

## Notable Comments

- */
- //FLYINGSNAKE  Defined in natural as well, think is a better fit there then here
- /// <summary>Add a race type define to the list</summary>
- /// <param name='model_id'>The id of the model</param>
- /// <param name=race_type_id'>The id of the race type</param>
- /// <param name=category'>The category of the race type</param>
- /// <param name=subcategory'>The subcategory of the race type</param>
- /// <param name=modelname'>The model name of the model id</param>
- /// <summary>Gets the race type for the given model</summary>
- /// <param name='model_id'>The model id to get the race type for</param>
