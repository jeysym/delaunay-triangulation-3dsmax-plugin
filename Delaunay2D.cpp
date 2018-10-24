#include "stdafx.h"
#include "Delaunay2D.h"

using Eigen::Vector3d;
using Eigen::Vector2d;
using Eigen::Matrix2d;
using std::vector;

#define UNUSED(x) 

namespace delaunay {

	// =============================================================================
	// DECLARATIONS
	// =============================================================================

	enum class KnownVertices : size_t {
		BBOX_LB = 0,		///< Bounding box left-bottom vertex.
		BBOX_RB,			///< Bounding box right-bottom vertex.
		BBOX_RT,			///< Bounding box right-top vertex.
		BBOX_LT,			///< Bounding box left-top vertex.
		COUNT				///< Signalizes how many known vertices there are.
	};


	// =============================================================================
	// IMPLEMENTATION
	// =============================================================================

	// OVERLOADED OPERATORS
	// ====================

	// Following operators on Edge struct are needed so that a collection of edges can be sorted,
	// checked for repeating edges etc.

	inline bool operator==(const BowyerWatson2D::Edge & lhs, const BowyerWatson2D::Edge & rhs) {
		return (lhs.m_v0 == rhs.m_v0) && (lhs.m_v1 == rhs.m_v1);
	}

	inline bool operator!=(const BowyerWatson2D::Edge & lhs, const BowyerWatson2D::Edge & rhs) {
		return !(lhs == rhs);
	}

	inline bool operator<(const BowyerWatson2D::Edge & lhs, const BowyerWatson2D::Edge & rhs) {
		if (lhs.m_v0 < rhs.m_v0)
			return true;

		if (lhs.m_v0 == rhs.m_v0)
			return (lhs.m_v1 < rhs.m_v1);

		return false;
	}


	// HELPER FUNCTIONS
	// ================

	/// Iterates over a collection and performs a functor for each element that does not repeat.
	template<typename I, typename F>
	void for_each_nonrepeating(I begin, I end, F func) {
		auto it = begin;
		while (it != end) {
			auto next = it + 1;

			if (next == end) {
				func(*it);
				return;
			}
			else if (*it != *next) {
				func(*it);
				++it;
			}
			else {
				while (*it == *next)
					++it;
			}
		}
	}
	
	inline static double square(double value) {
		return value * value;
	}

	inline static double squareSum(const Vector2d & vector) {
		return (square(vector.x()) + square(vector.y()));
	}

	inline static double squareSum(const Vector3d & vector) {
		return (square(vector.x()) + square(vector.y()) + square(vector.z()));
	}

	inline static bool compareVectorByXCoord(const Vector3d & v0, const Vector3d & v1) {
		return v0.x() < v1.x();
	}

	inline static Vector2d toVector2d(const Vector3d & vec) {
		return Vector2d(vec.x(), vec.y());
	}

	inline static Point3 toPoint3(const Vector3d & vec) {
		return Point3(vec.x(), vec.y(), vec.z());
	}


	// BOWYER WATSON EDGE IMPLEMENTATION
	// =================================

	BowyerWatson2D::Edge::Edge(BowyerWatson2D & UNUSED(ctx), size_t v0, size_t v1)
		: m_v0(v0), m_v1(v1)
	{
		if (m_v0 > m_v1)
			std::swap(m_v0, m_v1);
	}

	BowyerWatson2D::Triangle BowyerWatson2D::Edge::formTriangle(BowyerWatson2D & ctx, size_t vertex)
	{
		return Triangle(ctx, m_v0, m_v1, vertex);
	}


	// BOWYER WATSON TRIANGLE IMPLEMENTATION
	// =====================================

	BowyerWatson2D::Triangle::Triangle(BowyerWatson2D & ctx, size_t v0, size_t v1, size_t v2)
		: m_v0(v0), m_v1(v1), m_v2(v2)
	{
		const Vector2d vec0 = toVector2d(ctx.m_vertices[v0]);
		const Vector2d vec1 = toVector2d(ctx.m_vertices[v1]);
		const Vector2d vec2 = toVector2d(ctx.m_vertices[v2]);

		// To find the parameters of the circumscribed cirle of the triangle we solve system of 
		// two linear equations with two unknowns.
		// These equations are buit this way: 
		//		For each point [x,y] on the circle with center [X,Y] and radius R
		//		(x-X)^2 + (y-Y)^2 = R^2
		//
		//		Each triangle vertex forms a quadratic equation with unknowns X, Y, R.
		//		(x0-X)^2 + (y0-Y)^2 = R^2
		//		(x1-X)^2 + (y1-Y)^2 = R^2
		//		(x2-X)^2 + (y2-Y)^2 = R^2
		//		
		//		By subtracting the first equation from the other we get rid of the degree-2 power.
		//		And we get two linear equations.
		//		X * 2(x0 - x1) + Y * 2(y0 - y1) = (x0^2 + y0^2) - (x1^2 + y1^2)
		//		X * 2(x0 - x2) + Y * 2(y0 - y2) = (x0^2 + y0^2) - (x2^2 + y2^2)
		//
		//		This can be solved by the standard linear algebra methods.

		Matrix2d matrix;
		matrix <<
			2.0 * (vec0 - vec1),
			2.0 * (vec0 - vec2);
		matrix.transposeInPlace();

		Vector2d rhs;
		rhs <<
			squareSum(vec0) - squareSum(vec1), squareSum(vec0) - squareSum(vec2);

		Vector2d circumCenter = matrix.fullPivLu().solve(rhs);
		Vector2d x = vec0 - circumCenter;
		double circumRadiusSquared = squareSum(x);

		m_circumCenter = circumCenter;
		m_circumRadiusSquared = circumRadiusSquared;
	}

	vector<BowyerWatson2D::Edge> BowyerWatson2D::Triangle::getEdges(BowyerWatson2D & ctx)
	{
		vector<BowyerWatson2D::Edge> result;
		result.push_back(Edge(ctx, m_v0, m_v1));
		result.push_back(Edge(ctx, m_v1, m_v2));
		result.push_back(Edge(ctx, m_v2, m_v0));
		return result;
	}

	bool BowyerWatson2D::Triangle::containsInCircumCircle(const Eigen::Vector2d & point)
	{
		double d = m_circumRadiusSquared;
		d -= square(point.x() - m_circumCenter.x());
		d -= square(point.y() - m_circumCenter.y());

		if (d > 0.0) {
			return true;
		}

		return false;
	}

	bool BowyerWatson2D::Triangle::isBounding(BowyerWatson2D & UNUSED(ctx))
	{
		bool v0_bounding = m_v0 < size_t(KnownVertices::COUNT);
		bool v1_bounding = m_v1 < size_t(KnownVertices::COUNT);
		bool v2_bounding = m_v2 < size_t(KnownVertices::COUNT);
		
		if (v0_bounding || v1_bounding || v2_bounding) 
			return true;
		else
			return false;
	}

	void BowyerWatson2D::makeBoundingTriangles(const vertexCollection & vertices)
	{
		double xMin = std::numeric_limits<double>::max();
		double xMax = std::numeric_limits<double>::lowest();
		double yMin = std::numeric_limits<double>::max();
		double yMax = std::numeric_limits<double>::lowest();

		for (const Vector3d & vertex : vertices) {
			if (vertex.x() < xMin) xMin = vertex.x();
			if (vertex.x() > xMax) xMax = vertex.x();

			if (vertex.y() < yMin) yMin = vertex.y();
			if (vertex.y() > yMax) yMax = vertex.y();
		}

		double dx = xMax - xMin;
		double dy = yMax - yMin;
		double maxD = std::max(dx, dy);

		double left = xMin - maxD;
		double right = xMax + maxD;
		double top = yMax + maxD;
		double bottom = yMin - maxD;

		Vector3d lb(left, bottom, 0.0);
		Vector3d rb(right, bottom, 0.0);
		Vector3d rt(right, top, 0.0);
		Vector3d lt(left, top, 0.0);

		size_t lbIndex = size_t(KnownVertices::BBOX_LB);
		size_t rbIndex = size_t(KnownVertices::BBOX_RB);
		size_t rtIndex = size_t(KnownVertices::BBOX_RT);
		size_t ltIndex = size_t(KnownVertices::BBOX_LT);

		m_vertices.resize(size_t(KnownVertices::COUNT));
		m_vertices[lbIndex] = lb;
		m_vertices[rbIndex] = rb;
		m_vertices[rtIndex] = rt;
		m_vertices[ltIndex] = lt;

		m_currentTriangulation.push_back(Triangle(*this, lbIndex, rbIndex, rtIndex));
		m_currentTriangulation.push_back(Triangle(*this, lbIndex, rtIndex, ltIndex));
	}

	Mesh* BowyerWatson2D::convertTriangulationIntoMesh()
	{
		// CONSTRUCTION OF THE 3DS MAX MESH
		// ================================

		size_t boundingVerticesCount = size_t(KnownVertices::COUNT);
		size_t totalVerticesCount = m_vertices.size();
		size_t verticesCount = totalVerticesCount - boundingVerticesCount;

		size_t triangleCount = std::count_if(
			m_currentTriangulation.begin(),
			m_currentTriangulation.end(),
			[this](Triangle & triangle) { return (triangle.isBounding(*this) == false); }
		);

		Mesh* result = new Mesh;
		result->setNumVerts(int(verticesCount));
		result->setNumFaces(int(2 * triangleCount));

		for (size_t iVertex = 0; iVertex < verticesCount; ++iVertex) {
			Vector3d & vertex = m_vertices[iVertex + boundingVerticesCount];
			result->setVert(int(iVertex), toPoint3(vertex));
		}

		size_t iFace = 0;
		for (Triangle & triangle : m_currentTriangulation) {
			if (triangle.isBounding(*this))
				continue;

			DWORD index0 = DWORD(triangle.m_v0 - boundingVerticesCount);
			DWORD index1 = DWORD(triangle.m_v1 - boundingVerticesCount);
			DWORD index2 = DWORD(triangle.m_v2 - boundingVerticesCount);

			result->faces[iFace].v[0] = index0;
			result->faces[iFace].v[1] = index1;
			result->faces[iFace].v[2] = index2;

			result->faces[triangleCount + iFace].v[0] = index2;
			result->faces[triangleCount + iFace].v[1] = index1;
			result->faces[triangleCount + iFace].v[2] = index0;

			++iFace;
		}

		return result;
	}

	Mesh* BowyerWatson2D::invoke(const vector<Vector3d> & inputVertices)
	{
		// PREPARATION PHASE
		// =================

		// Calculate the initial triangulation which bounds the input vertices.
		makeBoundingTriangles(inputVertices);

		// Insert the input vertices.
		m_vertices.insert(m_vertices.end(), inputVertices.begin(), inputVertices.end());

		// Sort the input data vertices by x-coordinate.
		size_t firstVertexIndex = size_t(KnownVertices::COUNT);
		auto beginIt = m_vertices.begin() + firstVertexIndex;
		std::sort(beginIt, m_vertices.end(), compareVectorByXCoord);


		// INSERTING THE VERTICES
		// ======================

		size_t totalVertexCount = m_vertices.size();
		for (size_t iVertex = firstVertexIndex; iVertex < totalVertexCount; ++iVertex) {
			Vector3d & vertex = m_vertices[iVertex];
			Vector2d vertex2D = toVector2d(vertex);

			vector<Edge> badEdges;

			size_t triangleCount = m_currentTriangulation.size();
			for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle) {
				Triangle & triangle = m_currentTriangulation[iTriangle];

				if (triangle.containsInCircumCircle(vertex2D)) {
					// triangle is bad, it must be cut out
					triangle.m_isBad = true;
					auto triangleEdges = triangle.getEdges(*this);
					badEdges.insert(badEdges.end(), triangleEdges.begin(), triangleEdges.end());
				}
			}

			// simply remove all the bad triangles
			m_currentTriangulation.erase(
				std::remove_if(
					m_currentTriangulation.begin(),
					m_currentTriangulation.end(),
					[this](const Triangle & t) { return t.m_isBad; }
				), 
				m_currentTriangulation.end()
			);

			// construct the polygon that forms the boundary of the bad triangles
			// and create new triangles from this polygon
			std::sort(badEdges.begin(), badEdges.end());
			for_each_nonrepeating(
				badEdges.begin(), 
				badEdges.end(),
				[this, iVertex](Edge & edge) {
					m_currentTriangulation.push_back(edge.formTriangle(*this, iVertex));
				}
			);
		}

		return convertTriangulationIntoMesh();
	}

}