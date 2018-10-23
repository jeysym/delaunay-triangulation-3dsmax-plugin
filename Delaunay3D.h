#pragma once

namespace delaunay {

	class IDelaunay3D {
	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) = 0;
	};

	class BowyerWatson3D : public IDelaunay3D {
	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) override;
	};

}