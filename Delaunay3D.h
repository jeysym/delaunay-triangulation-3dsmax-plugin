#pragma once
#include "TetraMesh.h"

class IDelaunay3D {
public:
	virtual TetraMesh invoke(const std::vector<Eigen::Vector3d> & vertices) = 0;
};

class BowyerWatson3D : public IDelaunay3D {
public:
	virtual TetraMesh invoke(const std::vector<Eigen::Vector3d> & vertices) override;
};