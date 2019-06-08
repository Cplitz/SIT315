#include "pch.h"
#include "Octree.h"

// Constructor
Octree::Octree(Vector3 fbl, Vector3 ntr, unsigned int max_depth) {
	far_bottom_left = fbl;
	near_top_right = ntr;
	middle = far_bottom_left * 0.5 + near_top_right * 0.5;
	level = max_depth;
}

Vector3 Octree::getFBL() {
	return far_bottom_left;
}

Vector3 Octree::getNTR() {
	return near_top_right;
}

float Octree::getLevel() {
	return level;
}

// Calculates the sum of all masses of a given vector of elements
float Octree::sumMasses(std::vector<Element> objects) {
	float sum = 0;
	for (int i = 0; i < objects.size(); i++) {
		sum += objects[i].mass;
	}

	return sum;
}

// Calculates the center of mass of a given vector of elements
Vector3 Octree::calculateCenterOfMass(std::vector<Element> objects) {
	Vector3 centerOfMass = ZERO;
	float mass = 0;
	for (int i = 0; i < objects.size(); i++) {
		centerOfMass = centerOfMass + (objects[i].pos * objects[i].mass);
		mass += objects[i].mass;
	}

	return centerOfMass / mass;
}

// Calculates the center of mass recursively for each child node
void Octree::calculateCenterOfMass() {
	CoM = ZERO;
	msum = 0;

	if (objects.size() > 0) {
		CoM = calculateCenterOfMass(objects);
		msum = sumMasses(objects);
	}
	else {
		for (int i = 0; i < 8; i++) {
			if (children[i]) {
				children[i]->calculateCenterOfMass();
				msum += children[i]->msum;
				CoM = CoM + (children[i]->CoM * children[i]->msum);
			}
		}
		CoM = CoM / msum;
	}
}

// Calculates an acceleration given two elements
Vector3 Octree::calcAccel(Element e1, Element e2) {
	if (e1.pos == e2.pos)
		return ZERO;

	Vector3 accel = ZERO;

	double temp = GC * e2.mass / pow((e1.pos - e2.pos).mod(), 3);
	accel = accel + (e2.pos - e1.pos) * temp;

	return accel;
}

// Calculate the acceleration on element e as a result of the sum of gravitational forces
Vector3 Octree::calcTreeAccel(Element e) {

	if (CoM == ZERO || msum <= 0) {
		return ZERO;
	}

	// Calculate normally using objects if this is a leaf node (objects.size() > 0)
	Vector3 accel = ZERO;
	if (objects.size() > 0) {
		for (int i = 0; i < objects.size(); i++) {
			accel = accel + calcAccel(e, objects[i]);
		}
	}
	else {
		// Size of box
		float s = near_top_right.x - far_bottom_left.x;

		// Distance to CoM
		float d = (CoM - e.pos).mod();

		// THETA is a constant, determines whether or not to treat the tree as a center of mass or whether to continue to recursively call calcTreeAccel on children
		if (s / d < THETA) {
			accel = accel + calcAccel(e, { CoM, msum });
		}
		else {
			for (int i = 0; i < 8; i++) {
				if (children[i])
					accel = accel + children[i]->calcTreeAccel(e);
			}
		}
	}


	return accel;
}

// Insert an element into the tree
void Octree::add(Element e) {
	if (level == 0 || objects.empty() && is_leaf()) {
		objects.emplace_back(e);
		return;
	}

	// Determine child to place in
	const bool left = e.pos.x < middle.x;
	const bool down = e.pos.y < middle.y;
	const bool far = e.pos.z < middle.z;
	auto & child = children[4 * left + 2 * down + far];

	// If the child is not set yet, divide the box into another octree and re-insert the objects
	if (!child) {

		Vector3 fbl = far_bottom_left;
		Vector3 ntr = near_top_right;
		(left ? ntr : fbl).x = middle.x;
		(down ? ntr : fbl).y = middle.y;
		(far ? ntr : fbl).z = middle.z;

		child = std::make_unique<Octree>(fbl, ntr, level - 1);

		// move any directly-held object
		std::vector<Element> copyObjects = objects;
		objects.clear();
		for (int i = 0; i < copyObjects.size(); i++) {
			add(copyObjects[i]);
		}
	}
	child->add(e);
}

bool Octree::is_leaf() {
	// we're a leaf node if all children are null
	return std::all_of(std::begin(children), std::end(children),
		[](auto& x) {return !x; });
}