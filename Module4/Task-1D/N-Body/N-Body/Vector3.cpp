#include "pch.h"
#include "Vector3.h"
#include <cmath>


// Return the size of the vector
float Vector3::mod() const {
	return sqrt(x*x + y * y + z * z);
}

// Get the unit vector in the same direction
Vector3 Vector3::unit() {
	float mag = mod();
	return Vector3(x /= mag, y /= mag, z /= mag);
}

// + overload
Vector3 Vector3::operator+(const Vector3& rhs) const {
	return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

// - overload
Vector3 Vector3::operator-(const Vector3& rhs) const {
	return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
}

// * overload
Vector3 Vector3::operator*(float s) const {
	return Vector3(x*s, y*s, z*s);
}

Vector3 Vector3::operator/(float s) const {
	return Vector3(x / s, y / s, z / s);
}

// == overload
bool Vector3::operator==(const Vector3& rhs) const {
	return x == rhs.x
		&& y == rhs.y
		&& z == rhs.z;
}

