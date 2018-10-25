#pragma once

namespace delaunay {

	/// \brief Interface class for general 3D delaunay algorithm that can be invoked on collection
	/// of vertices.
	class IDelaunay3D {
	public:
		/// \brief Invoke the algorithm. Constructs the tetrahedration that is returned in form of 
		/// Mesh instance. (The caller is responsible for freeing the returned Mesh)
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) = 0;
	};

	/// \brief Implementation class of Bowyer-Watson algorithm for construction of 3D delaunay 
	/// triangulation (tetrahedration).
	class BowyerWatson3D : public IDelaunay3D {
	public:
		struct Tetrahedron;		// forward declaration

		/// Structure representing a triangle formed by three vertices.
		struct Triangle {
			size_t m_v0;	///< Index of the first vertex.
			size_t m_v1;	///< Index of the second vertex.
			size_t m_v2;	///< Index of the third vertex.

			Triangle(BowyerWatson3D & ctx, size_t v0, size_t v1, size_t v2);

			/// Forms a new tetrahedron by connecting this triangle with a vertex.
			Tetrahedron formTetrahedron(BowyerWatson3D & ctx, size_t vertex);
		};

		struct Tetrahedron {
			size_t m_v0;	///< Index of the first vertex.
			size_t m_v1;	///< Index of the second vertex.
			size_t m_v2;	///< Index of the third vertex.
			size_t m_v3;	///< Index of the fourth vertex.

			Eigen::Vector3d m_circumCenter;	///< Cached center of circumscribed sphere.
			double m_circumRadiusSquared;	///< Cached squared radius of circumscribed sphere.
			bool m_isBad = false;			///< A flag that marks to-be-deleted tetrahedrons.

			Tetrahedron(BowyerWatson3D & ctx, size_t v0, size_t v1, size_t v2, size_t v3);

			/// Returns collection of all the triangles of this tetrahedron.
			std::vector<Triangle> getTriangles(BowyerWatson3D & ctx);

			/// \brief Checks whether the given point is contained inside the circumscribed sphere 
			/// of this tetrahedron. 
			bool containsInCircumSphere(const Eigen::Vector3d & point);

			/// \brief Tells if this tetrahedron contains the virtual bounding vertex, i.e. it is 
			/// not a part of the returned tetrahedration.
			bool isBounding(BowyerWatson3D & ctx);
		};

	private:
		using vertexCollection = std::vector<Eigen::Vector3d>;
		using edgeCollection = std::vector<Edge>;
		using triangleCollection = std::vector<Triangle>;
		using tetraCollection = std::vector<Tetrahedron>;

		/// The input vertices that are to be triangulated.
		vertexCollection m_vertices = std::vector<Eigen::Vector3d>();
		/// Current tetrahedration. After each vertex insertion it should hold valid 3D delaunay
		/// tetrahedration.
		tetraCollection m_currentTetrahedration = std::vector<Tetrahedron>();

		/// \brief Construct the starting tetrahedration that contains all the input vertices. 
		///
		/// This is needed because each step of the Bowyer-Watson algorithm needs a correct delaunay
		/// tetrahedration to work on. So the first step must be supplied this artificially created
		/// tetrahedration.
		void makeBoundingTetrahedrons(const vertexCollection & vertices);

		/// Converts the computed triangulation into 3ds Max Mesh structure. 
		Mesh* convertTetrahedrationIntoMesh();

	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) override;
		virtual ~BowyerWatson3D() {}
	};

}