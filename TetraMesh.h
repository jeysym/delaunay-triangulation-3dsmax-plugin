#pragma once

struct AxisAlignedBox {
	double xMin, xMax;
	double yMin, yMax;
	double zMin, zMax;

	AxisAlignedBox(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);

	double getLongestEdge() const;
	void enlargeBy(double amount);

	static AxisAlignedBox getBoundingBox(std::vector<Eigen::Vector3d> vertices);
};


class TetraMesh {
public:
	struct VertexDesc {
		Eigen::Vector3d coordinate;
	};

	struct TriangleDesc {
		size_t v0, v1, v2;

		size_t tetrahedronOut;
		size_t tetrahedronIn;
	};

	struct TetrahedronDesc {
		size_t t0, t1, t2, t3;

		Eigen::Vector3d circumsphereCenter;
		double circumsphereRadius;
	};

	std::vector<VertexDesc> vertices;
	std::vector<TriangleDesc> triangles;
	std::vector<TetrahedronDesc> tetrahedrons;


public:
	size_t getNumberOfTetrahedrons() const;
	void deleteTetrahedrons(std::vector<size_t> indices); // TODO: Make this accept every iterable collection.
	std::tuple<double, double> getCircumCenterAndRadius(size_t tetrahedronIndex) const;
	

	Mesh * convertTo3dsMaxMesh();

	static TetraMesh constructTetraMeshCoveringBox(const AxisAlignedBox & box);
};