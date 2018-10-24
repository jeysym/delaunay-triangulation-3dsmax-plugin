#pragma once

namespace delaunay {

	/// \brief Interface class for general 2D delaunay algorithm that can be invoked on collection
	/// of vertices.
	class IDelaunay2D {
	public:
		/// \brief Invoke the algorithm. Constructs the triangulation that is returned in form of 
		/// Mesh instance. (The caller is responsible for freeing the returned Mesh)
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) = 0;
	};

	/// \brief Implementation class of Bowyer-Watson algorithm for construction of 2D delaunay 
	/// triangulation.
	class BowyerWatson2D : public IDelaunay2D {
	public:
		struct Triangle;		// forward declaration

		struct Edge {
			size_t m_v0, m_v1;

			Edge(BowyerWatson2D & ctx, size_t v0, size_t v1);
			Triangle formTriangle(BowyerWatson2D & ctx, size_t vertex);
		};

		struct Triangle {
			size_t m_v0, m_v1, m_v2;
			Eigen::Vector2d m_circumCenter;
			double m_circumRadiusSquared;
			bool m_isBad = false;

			Triangle(BowyerWatson2D & ctx, size_t v0, size_t v1, size_t v2);
			std::vector<Edge> getEdges(BowyerWatson2D & ctx);
			bool containsInCircumCircle(const Eigen::Vector2d & point);
			bool isBounding(BowyerWatson2D & ctx);
		};

	private:
		using vertexCollection = std::vector<Eigen::Vector3d>;
		using edgeCollection = std::vector<Edge>;
		using triangleCollection = std::vector<Triangle>;

		vertexCollection m_vertices = std::vector<Eigen::Vector3d>();
		triangleCollection m_currentTriangulation = std::vector<Triangle>();

		void makeBoundingTriangles(const vertexCollection & vertices);
		Mesh* convertTriangulationIntoMesh();

	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) override;
		virtual ~BowyerWatson2D() {}
	};

}