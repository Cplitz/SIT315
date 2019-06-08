#pragma once
// Vector3 class representing a 3D vector with useful operations
class Vector3 {
public:
	float x, y, z;

	// Constructors
	Vector3() : x(0.0), y(0.0), z(0.0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	// Return the size of the vector
	float mod() const;

	// Get the unit vector in the same direction
	Vector3 unit();

	// + overload
	Vector3 operator+(const Vector3& rhs) const;

	// - overload
	Vector3 operator-(const Vector3& rhs) const;

	// * overload
	Vector3 operator*(float s) const;

	// / overload
	Vector3 operator/(float s) const;

	// == overload
	bool operator==(const Vector3& rhs) const;
};

// Zero vector
const Vector3 ZERO{ 0.0, 0.0, 0.0 };