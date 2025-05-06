# File: `opcodemgr.h`

## Classes

- `OpcodeManager`
- `OpcodeSetStrategy`
- `MutableOpcodeManager`
- `SharedOpcodeManager`
- `SharedMemStrategy`
- `RegularOpcodeManager`
- `NormalMemStrategy`
- `NullOpcodeManager`
- `EmptyOpcodeManager`

## Functions

- `EmuOpcode NameSearch(const char *name);`
- `void Set(EmuOpcode emu_op, uint16 eq_op);`
- `void Set(EmuOpcode emu_op, uint16 eq_op);`

## Notable Comments

- /*
- */
- //This has to be public for stupid visual studio
- //in a shared manager, this dosent protect others
- //keeps opcodes in shared memory
- //keeps opcodes in regular heap memory
- //implement our editing interface
- //always resolves everything to 0 or OP_Unknown
- //fake it, just used for testing anyways
- //starts as NullOpcodeManager, but remembers any mappings set
