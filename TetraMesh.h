#pragma once

struct AxisAlignedBox {
	double xMin, xMax;
	double yMin, yMax;
	double zMin, zMan;

	AxisAlignedBox(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);

	void enlargeBy(double amount);

	static AxisAlignedBox getBoundingBox(std::vector<Eigen::Vector3d> vertices);
};

class TetraMesh {
public:
	Mesh * convertTo3dsMaxMesh();

	static TetraMesh constructTetraMeshCoveringBox(const AxisAlignedBox & box)
};