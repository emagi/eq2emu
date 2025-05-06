# File: `position.h`

## Classes

_None detected_

## Functions

- `std::string to_string(const glm::vec4 &position);`
- `std::string to_string(const glm::vec3 &position);`
- `std::string to_string(const glm::vec2 &position);`
- `bool IsWithinAxisAlignedBox(const glm::vec3 &position, const glm::vec3 &minimum, const glm::vec3 &maximum);`
- `bool IsWithinAxisAlignedBox(const glm::vec2 &position, const glm::vec2 &minimum, const glm::vec2 &maximum);`
- `bool IsOrigin(const glm::vec2 &position);`
- `bool IsOrigin(const glm::vec3 &position);`
- `bool IsOrigin(const glm::vec4 &position);`
- `float DistanceSquared(const glm::vec2& point1, const glm::vec2& point2);`
- `float Distance(const glm::vec2& point1, const glm::vec2& point2);`
- `float DistanceSquared(const glm::vec3& point1, const glm::vec3& point2);`
- `float Distance(const glm::vec3& point1, const glm::vec3& point2);`
- `float DistanceNoZ(const glm::vec3& point1, const glm::vec3& point2);`
- `float DistanceSquaredNoZ(const glm::vec3& point1, const glm::vec3& point2);`
- `float DistanceSquared(const glm::vec4& point1, const glm::vec4& point2);`
- `float Distance(const glm::vec4& point1, const glm::vec4& point2);`
- `float DistanceNoZ(const glm::vec4& point1, const glm::vec4& point2);`
- `float DistanceSquaredNoZ(const glm::vec4& point1, const glm::vec4& point2);`
- `float GetReciprocalHeading(const glm::vec4& point1);`
- `float GetReciprocalHeading(const float heading);`
- `bool IsHeadingEqual(const float h1, const float h2);`
- `bool IsPositionEqual(const glm::vec2 &p1, const glm::vec2 &p2);`
- `bool IsPositionEqual(const glm::vec3 &p1, const glm::vec3 &p2);`
- `bool IsPositionEqual(const glm::vec4 &p1, const glm::vec4 &p2);`
- `bool IsPositionEqualWithinCertainZ(const glm::vec3 &p1, const glm::vec3 &p2, float z_eps);`
- `bool IsPositionEqualWithinCertainZ(const glm::vec4 &p1, const glm::vec4 &p2, float z_eps);`
- `bool IsPositionWithinSimpleCylinder(const glm::vec3 &p1, const glm::vec3 &cylinder_center, float cylinder_radius, float cylinder_height);`
- `bool IsPositionWithinSimpleCylinder(const glm::vec4 &p1, const glm::vec4 &cylinder_center, float cylinder_radius, float cylinder_height);`
- `float CalculateHeadingAngleBetweenPositions(float x1, float y1, float x2, float y2);`

## Notable Comments

- /*	EQEMu: Everquest Server Emulator
- */
