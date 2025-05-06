# File: `raycast_mesh.h`

## Classes

- `RaycastMesh`

## Functions

- `void serializeRaycastMesh(RaycastMesh* rm, std::vector<char>& rm_buffer);`

## Notable Comments

- // This code snippet allows you to create an axis aligned bounding volume tree for a triangle mesh so that you can do
- // high-speed raycasting.
- //
- // There are much better implementations of this available on the internet.  In particular I recommend that you use
- // OPCODE written by Pierre Terdiman.
- // @see: http://www.codercorner.com/Opcode.htm
- //
- // OPCODE does a whole lot more than just raycasting, and is a rather significant amount of source code.
- //
- // I am providing this code snippet for the use case where you *only* want to do quick and dirty optimized raycasting.
