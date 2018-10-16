#include "stdafx.h"
#include "Delaunay3D.h"

using std::vector;
using Eigen::Vector3d;

TetraMesh BowyerWatson3D::invoke(const vector<Vector3d> & vertices)
{
	AxisAlignedBox box = AxisAlignedBox::getBoundingBox(vertices);
	box.enlargeBy(1000.0);

	TetraMesh tetrahedration = TetraMesh::constructTetraMeshCoveringBox(box);

	for (auto it = vertices.begin(); it != vertices.end(); ++it) {
		for (tetrahedration.)
	}

	return TetraMesh();	// TODO: implement this
}
