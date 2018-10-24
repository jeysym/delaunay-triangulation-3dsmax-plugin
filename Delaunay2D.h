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

		/// Structure representing an edge between two vertices.
		struct Edge {
			size_t m_v0;	///< Index of the first vertex.
			size_t m_v1;	///< Index of the second vertex.

			Edge(BowyerWatson2D & ctx, size_t v0, size_t v1);

			/// Forms a new triangle by connecting this edge with a vertex.
			Triangle formTriangle(BowyerWatson2D & ctx, size_t vertex);
		};

		/// Structure representing a triangle formed by three vertices.
		struct Triangle {
			size_t m_v0;	///< Index of the first vertex.
			size_t m_v1;	///< Index of the second vertex.
			size_t m_v2;	///< Index of the third vertex.

			Eigen::Vector2d m_circumCenter;	///< Cached center of circumscribed circle.
			double m_circumRadiusSquared;	///< Cached squared radius of circumscribed circle.
			bool m_isBad = false;			///< A flag that marks to-be-deleted triangles.

			Triangle(BowyerWatson2D & ctx, size_t v0, size_t v1, size_t v2);

			/// Returns collection of all the edges of this triangle.
			std::vector<Edge> getEdges(BowyerWatson2D & ctx);

			/// \brief Checks whether the given point is contained inside the circumscribed circle 
			/// of this triangle. 
			bool containsInCircumCircle(const Eigen::Vector2d & point);

			/// \brief Tells if this triangle contains the virtual bounding vertex, i.e. it is not
			/// a part of the returned triangulation.
			bool isBounding(BowyerWatson2D & ctx);
		};

	private:
		using vertexCollection = std::vector<Eigen::Vector3d>;
		using edgeCollection = std::vector<Edge>;
		using triangleCollection = std::vector<Triangle>;

		/// The input vertices that are to be triangulated.
		vertexCollection m_vertices = std::vector<Eigen::Vector3d>();
		/// Current triangulation. After each vertex insertion it should hold valid 2D delaunay
		/// triangulation.
		triangleCollection m_currentTriangulation = std::vector<Triangle>();

		/// \brief Construct the starting triangulation that contains all the input vertices. 
		///
		/// This is needed because each step of the Bowyer-Watson algorithm needs a correct delaunay
		/// triangulation to work on. So the first step must be supplied this artificially created
		/// triangulation.
		void makeBoundingTriangles(const vertexCollection & vertices);

		/// Converts the computed triangulation into 3ds Max Mesh structure. 
		Mesh* convertTriangulationIntoMesh();

	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) override;
		virtual ~BowyerWatson2D() {}
	};

}