#pragma once
#include <memory>
#include "Vector3.h"
#include <vector>
#include <algorithm>

struct Element
{
	Vector3 pos;
	float mass;
};

class Octree
{
	// Geometry
	Vector3 far_bottom_left;
	Vector3 near_top_right;
	Vector3 middle;

	// Limit to tree depth
	unsigned int level;

	// subtree objects - any or all may be null
	// index using (left=4, right=0) + (bottom=2, top=0) + (far=1, near=0)
	std::unique_ptr<Octree> children[8] = {};

	// directly held elements - empty unless all children are null
	std::vector<Element> objects = {};

	// Mass
	Vector3 CoM = ZERO;
	float msum = 0;

	// Constants
	const float GC = 0.1;
	const float SOFTENING_FACTOR = 0.1;
	const float THETA = 1;

public:
	// Constructor
	Octree(Vector3 fbl, Vector3 ntr, unsigned int max_depth);

	// Insert an element into the tree
	void add(Element e);

	// Calculate the acceleration on element e as a result of the sum of gravitational forces
	Vector3 calcTreeAccel(Element e);

	// Calculate the center of mass for the octree
	void calculateCenterOfMass();

	Vector3 getFBL();
	Vector3 getNTR();
	float getLevel();

private:
	// Checks if the tree is a leaf node
	bool is_leaf();

	// Calculates an acceleration given two elements
	Vector3 calcAccel(Element e1, Element e2);

	// Calculates the center of mass of a given vector of elements
	Vector3 calculateCenterOfMass(std::vector<Element> objects);

	// Calculates the sum of all masses of a given vector of elements
	float sumMasses(std::vector<Element> objects);
};