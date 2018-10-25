#include "stdafx.h"
#include "Delaunay3D.h"
#include "Common.h"

using Eigen::Vector3d;
using Eigen::Vector2d;
using Eigen::Matrix2d;
using Eigen::Matrix3d;
using std::vector;

#define UNUSED(x) 

namespace delaunay {

	// =============================================================================
	// DECLARATIONS
	// =============================================================================

	enum class KnownVertices : size_t {
		BBOX_LBF = 0,		///< Bounding box left-bottom-front vertex.
		BBOX_RBF,			///< Bounding box right-bottom-front vertex.
		BBOX_RTF,			///< Bounding box right-top-front vertex.
		BBOX_LTF,			///< Bounding box left-top-front vertex.
		BBOX_LBB,			///< Bounding box left-bottom-back vertex.
		BBOX_RBB,			///< Bounding box right-bottom-back vertex.
		BBOX_RTB,			///< Bounding box right-top-back vertex.
		BBOX_LTB,			///< Bounding box left-top-back vertex.
		COUNT				///< Signalizes how many known vertices there are.
	};


	// =============================================================================
	// IMPLEMENTATION
	// =============================================================================

	// OVERLOADED OPERATORS
	// ====================

	// Following operators on Triangle struct are needed so that a collection of triangles can 
	// be sorted, checked for repeating triangles etc.

	inline bool operator==(const BowyerWatson3D::Triangle & lhs, const BowyerWatson3D::Triangle & rhs) {
		return (lhs.m_v0 == rhs.m_v0) && (lhs.m_v1 == rhs.m_v1) && (lhs.m_v2 == rhs.m_v2);
	}

	inline bool operator!=(const BowyerWatson3D::Triangle & lhs, const BowyerWatson3D::Triangle & rhs) {
		return !(lhs == rhs);
	}

	inline bool operator<(const BowyerWatson3D::Triangle & lhs, const BowyerWatson3D::Triangle & rhs) {
		if (lhs.m_v0 < rhs.m_v0)
			return true;

		if (lhs.m_v0 == rhs.m_v0) {
			if (lhs.m_v1 < rhs.m_v1) {
				return true;
			}
			else if (lhs.m_v1 == rhs.m_v1) {
				return (lhs.m_v2 < rhs.m_v2);
			}
			else {
				return false;
			}
		}
		else
			return false;
	}


	// BOWYER WATSON TRIANGLE IMPLEMENTATION
	// =====================================

	BowyerWatson3D::Triangle::Triangle(BowyerWatson3D & UNUSED(ctx), size_t v0, size_t v1, size_t v2)
	{
		std::array<size_t, 3> indices;
		indices[0] = v0;
		indices[1] = v1;
		indices[2] = v2;

		std::sort(indices.begin(), indices.end());
		
		m_v0 = indices[0];
		m_v1 = indices[1];
		m_v2 = indices[2];
	}

	BowyerWatson3D::Tetrahedron BowyerWatson3D::Triangle::formTetrahedron(BowyerWatson3D & ctx, size_t vertex)
	{
		return Tetrahedron(ctx, m_v0, m_v1, m_v2, vertex);
	}


	// BOWYER WATSON TETRAHEDRON IMPLEMENTATION
	// ========================================

	BowyerWatson3D::Tetrahedron::Tetrahedron(BowyerWatson3D & ctx, size_t v0, size_t v1, size_t v2, size_t v3)
		: m_v0(v0), m_v1(v1), m_v2(v2), m_v3(v3)
	{
		const Vector3d vec0 = ctx.m_vertices[v0];
		const Vector3d vec1 = ctx.m_vertices[v1];
		const Vector3d vec2 = ctx.m_vertices[v2];
		const Vector3d vec3 = ctx.m_vertices[v3];

		// The logic behind this is the same as in 2D version. More info can be found in 
		// BowyerWatson2D::Triangle::Triangle() in "Delaunay2D.cpp".

		Matrix3d matrix;
		matrix.row(0) = 2.0 * (vec0 - vec1);
		matrix.row(1) = 2.0 * (vec0 - vec2);
		matrix.row(2) = 2.0 * (vec0 - vec3);

		Vector3d rhs;
		rhs(0) = squareSum(vec0) - squareSum(vec1);
		rhs(1) = squareSum(vec0) - squareSum(vec2);
		rhs(2) = squareSum(vec0) - squareSum(vec3);

		Vector3d circumCenter = matrix.fullPivLu().solve(rhs);
		Vector3d vec0d = vec0 - circumCenter;
		double circumRadiusSquared = squareSum(vec0d);

		m_circumCenter = circumCenter;
		m_circumRadiusSquared = circumRadiusSquared;
	}

	std::vector<BowyerWatson3D::Triangle> BowyerWatson3D::Tetrahedron::getTriangles(BowyerWatson3D & ctx)
	{
		vector<BowyerWatson3D::Triangle> result;
		result.push_back(Triangle(ctx, m_v0, m_v1, m_v2));
		result.push_back(Triangle(ctx, m_v0, m_v1, m_v3));
		result.push_back(Triangle(ctx, m_v0, m_v2, m_v3));
		result.push_back(Triangle(ctx, m_v1, m_v2, m_v3));
		return result;
	}

	bool BowyerWatson3D::Tetrahedron::containsInCircumSphere(const Eigen::Vector3d & point)
	{
		double d = m_circumRadiusSquared;
		d -= square(point.x() - m_circumCenter.x());
		d -= square(point.y() - m_circumCenter.y());
		d -= square(point.z() - m_circumCenter.z());

		if (d > 0.0) {
			return true;
		}

		return false;
	}

	bool BowyerWatson3D::Tetrahedron::isBounding(BowyerWatson3D & UNUSED(ctx))
	{
		bool v0_bounding = m_v0 < size_t(KnownVertices::COUNT);
		bool v1_bounding = m_v1 < size_t(KnownVertices::COUNT);
		bool v2_bounding = m_v2 < size_t(KnownVertices::COUNT);
		bool v3_bounding = m_v3 < size_t(KnownVertices::COUNT);

		if (v0_bounding || v1_bounding || v2_bounding || v3_bounding)
			return true;
		else
			return false;
	}


	// BOWYER WATSON ALGORITHM IMPLEMENTATION
	// ======================================

	void BowyerWatson3D::makeBoundingTetrahedrons(const vertexCollection & vertices)
	{
		// This function does this:
		// * Finds the bounding box of the input vertices.
		// * Enlarges the bounding box as it may play a role in some edge cases.
		// * Construct a valid 3D delaunay tetrahedration of this box.

		double xMin = std::numeric_limits<double>::max();
		double xMax = std::numeric_limits<double>::lowest();
		double yMin = std::numeric_limits<double>::max();
		double yMax = std::numeric_limits<double>::lowest();
		double zMin = std::numeric_limits<double>::max();
		double zMax = std::numeric_limits<double>::lowest();

		for (const Vector3d & vertex : vertices) {
			if (vertex.x() < xMin) xMin = vertex.x();
			if (vertex.x() > xMax) xMax = vertex.x();

			if (vertex.y() < yMin) yMin = vertex.y();
			if (vertex.y() > yMax) yMax = vertex.y();

			if (vertex.z() < zMin) zMin = vertex.z();
			if (vertex.z() > zMax) zMax = vertex.z();
		}

		double dx = xMax - xMin;
		double dy = yMax - yMin;
		double dz = zMax - zMin;
		double maxD = std::max(dx, std::max(dy, dz));

		double left = xMin - maxD;
		double right = xMax + maxD;
		double top = yMax + maxD;
		double bottom = yMin - maxD;
		double front = zMax + maxD;
		double back = zMin - maxD;

		Vector3d lbf(left, bottom, front);
		Vector3d rbf(right, bottom, front);
		Vector3d rtf(right, top, front);
		Vector3d ltf(left, top, front);
		Vector3d lbb(left, bottom, back);
		Vector3d rbb(right, bottom, back);
		Vector3d rtb(right, top, back);
		Vector3d ltb(left, top, back);

		size_t lbfIndex = size_t(KnownVertices::BBOX_LBF);
		size_t rbfIndex = size_t(KnownVertices::BBOX_RBF);
		size_t rtfIndex = size_t(KnownVertices::BBOX_RTF);
		size_t ltfIndex = size_t(KnownVertices::BBOX_LTF);
		size_t lbbIndex = size_t(KnownVertices::BBOX_LBB);
		size_t rbbIndex = size_t(KnownVertices::BBOX_RBB);
		size_t rtbIndex = size_t(KnownVertices::BBOX_RTB);
		size_t ltbIndex = size_t(KnownVertices::BBOX_LTB);

		m_vertices.resize(size_t(KnownVertices::COUNT));
		m_vertices[lbfIndex] = lbf;
		m_vertices[rbfIndex] = rbf;
		m_vertices[rtfIndex] = rtf;
		m_vertices[ltfIndex] = ltf;
		m_vertices[lbbIndex] = lbb;
		m_vertices[rbbIndex] = rbb;
		m_vertices[rtbIndex] = rtb;
		m_vertices[ltbIndex] = ltb;

		m_currentTetrahedration.push_back(Tetrahedron(*this, lbfIndex, rbbIndex, lbbIndex, ltbIndex));
		m_currentTetrahedration.push_back(Tetrahedron(*this, lbfIndex, rbbIndex, rbfIndex, rtfIndex));
		m_currentTetrahedration.push_back(Tetrahedron(*this, ltbIndex, rtfIndex, ltfIndex, lbfIndex));
		m_currentTetrahedration.push_back(Tetrahedron(*this, ltbIndex, rtfIndex, rtbIndex, rbbIndex));
		m_currentTetrahedration.push_back(Tetrahedron(*this, ltbIndex, rtfIndex, lbfIndex, rbbIndex));
	}

	Mesh* BowyerWatson3D::convertTetrahedrationIntoMesh()
	{
		// CONSTRUCTION OF THE 3DS MAX MESH
		// ================================

		size_t tetraCount = std::count_if(
			m_currentTetrahedration.begin(),
			m_currentTetrahedration.end(),
			[this](Tetrahedron & tetra) { return (tetra.isBounding(*this) == false); }
		);

		size_t verticesCount = 4 * tetraCount;
		size_t facesCount = 4 * tetraCount;

		Mesh* result = new Mesh;
		result->setNumVerts(int(verticesCount));
		result->setNumFaces(int(facesCount));

		size_t iTetra = 0;
		for (Tetrahedron & tetra : m_currentTetrahedration) {
			if (tetra.isBounding(*this))
				continue;

			Vector3d vec0 = m_vertices[tetra.m_v0];
			Vector3d vec1 = m_vertices[tetra.m_v1];
			Vector3d vec2 = m_vertices[tetra.m_v2];
			Vector3d vec3 = m_vertices[tetra.m_v3];

			int i0 = int(iTetra) * 4 + 0;
			int i1 = int(iTetra) * 4 + 1;
			int i2 = int(iTetra) * 4 + 2;
			int i3 = int(iTetra) * 4 + 3;

			result->setVert(i0, toPoint3(vec0));
			result->setVert(i1, toPoint3(vec1));
			result->setVert(i2, toPoint3(vec2));
			result->setVert(i3, toPoint3(vec3));

			result->faces[i0].v[0] = i0;
			result->faces[i0].v[1] = i1;
			result->faces[i0].v[2] = i2;
			
			result->faces[i1].v[0] = i0;
			result->faces[i1].v[1] = i1;
			result->faces[i1].v[2] = i3;

			result->faces[i2].v[0] = i0;
			result->faces[i2].v[1] = i2;
			result->faces[i2].v[2] = i3;

			result->faces[i3].v[0] = i1;
			result->faces[i3].v[1] = i2;
			result->faces[i3].v[2] = i3;

			++iTetra;
		}


		result->InvalidateGeomCache();
		return result;
	}

	Mesh* BowyerWatson3D::invoke(const std::vector<Eigen::Vector3d>& inputVertices)
	{
		// PREPARATION PHASE
		// =================

		// Calculate the initial triangulation which bounds the input vertices.
		makeBoundingTetrahedrons(inputVertices);

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

			vector<Triangle> badTriangles;

			size_t tetraCount = m_currentTetrahedration.size();
			for (size_t iTetra = 0; iTetra < tetraCount; ++iTetra) {
				Tetrahedron & tetra = m_currentTetrahedration[iTetra];

				if (tetra.containsInCircumSphere(vertex)) {
					// Tetrahedron is bad, it must be cut out.
					tetra.m_isBad = true;
					auto tetraTriangles = tetra.getTriangles(*this);
					badTriangles.insert(badTriangles.end(), tetraTriangles.begin(), tetraTriangles.end());
				}
			}

			// Simply remove all the bad tetrahedrons.
			m_currentTetrahedration.erase(
				std::remove_if(
					m_currentTetrahedration.begin(),
					m_currentTetrahedration.end(),
					[this](const Tetrahedron & t) { return t.m_isBad; }
				),
				m_currentTetrahedration.end()
			);

			// Construct the polytope that forms the boundary of the bad tetrahedrons
			// and create new tetrahedrons from this polytope.
			std::sort(badTriangles.begin(), badTriangles.end());
			for_each_nonrepeating(
				badTriangles.begin(),
				badTriangles.end(),
				[this, iVertex](Triangle & triangle) {
					m_currentTetrahedration.push_back(triangle.formTetrahedron(*this, iVertex));
				}
			);
		}

		return convertTetrahedrationIntoMesh();
	}

}
