# File: `Traits.h`

## Classes

- `Client`
- `TraitData`
- `MasterTraitList`

## Functions

- `bool IdentifyNextTrait(Client* client, map <int8, vector<TraitData*> >* traitList, vector<TraitData*>* collectTraits, vector<TraitData*>* tieredTraits, std::map<int32, int8>* previousMatchedSpells, bool omitFoundMatches = false);`
- `bool ChooseNextTrait(Client* client);`
- `int16 GetSpellCount(Client* client, map <int8, vector<TraitData*> >* traits, bool onlyCharTraits = false);`
- `bool IsPlayerAllowedTrait(Client* client, TraitData* trait);`
- `void AddTrait(TraitData* data);`
- `int Size();`
- `void DestroyTraits();`

## Notable Comments

- /*
- */
- /// <summary>Sorts the traits for the given client and creats and sends the trait packet.</summary>
- /// <param name='client'>The Client calling this function</param>
- /// <returns>EQ2Packet*</returns>
- /// <summary>Add trait data to the global list.</summary>
- /// <param name='data'>The trait data to add.</param>
- /// <summary>Get the total number of traits in the global list.</summary>
- /// <summary>Get the trait data for the given spell.</summary>
- /// <param name='spellID'>Spell ID to get trait data for.</param>
