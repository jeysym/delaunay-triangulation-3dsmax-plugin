#include "stdafx.h"
#include "Delaunay2D.h"

namespace delaunay {

	using Eigen::Vector3d;
	using Eigen::Vector2d;
	using Eigen::Matrix2d;
	using std::vector;

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

	inline static Point3 toPoint3(const Vector3d & vec) {
		return Point3(vec.x(), vec.y(), vec.z());
	}

	//struct Edge {
	//	size_t v0, v1;
	//
	//	Edge(size_t v0, size_t v1) : v0(v0), v1(v1) {
	//		if (v1 < v0)
	//			std::swap(this->v0, this->v1);
	//	}
	//};
	//
	//struct Triangle {
	//	size_t v0, v1, v2;
	//	Vector3d circumCenter;
	//	double circumRadiusSquared;
	//	bool relevant = true;
	//
	//	Triangle(const vector<Vector3d> & vertices, size_t v0, size_t v1, size_t v2)
	//		: v0(v0), v1(v1), v2(v2) {
	//		// TODO : compute the circumcenter and radius here.
	//	}
	//
	//	bool containsInCircumcircle(Vector3d vertex) {
	//		if (relevant == false)
	//			return false;
	//
	//		double difference = circumRadiusSquared - square(vertex.x());
	//		if (difference < 0.0) {
	//			relevant = false;
	//			return false;
	//		}
	//
	//		difference -= square(vertex.y());
	//		if (difference < 0.0) {
	//			return false;
	//		} 
	//		else {
	//			return true;
	//		}
	//	}
	//};

	vector<Triangle> makeBoundingTriangles(vector<Vector3d> & vertices) {
		vector<Triangle> result;

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
		xMin -= dx;
		xMax += dx;
		yMin -= dy;
		yMax += dy;

		size_t i = vertices.size();
		Vector3d A(xMin, yMin, 0.0);
		Vector3d B(xMax, yMin, 0.0);
		Vector3d C(xMax, yMax, 0.0);
		Vector3d D(xMin, yMax, 0.0);

		size_t iA = i;
		size_t iB = i + 1;
		size_t iC = i + 2;
		size_t iD = i + 3;

		vertices.push_back(A);
		vertices.push_back(B);
		vertices.push_back(C);
		vertices.push_back(D);

		Triangle t0(vertices, iA, iB, iC);
		Triangle t1(vertices, iA, iC, iD);
		t0.isBounding = true;
		t1.isBounding = true;

		result.push_back(t0);
		result.push_back(t1);

		return result;
	}

	Mesh* BowyerWatson2D::invoke(const vector<Vector3d> & inputVertices)
	{
		// PREPARATION PHASE
		// =================

		vector<Vector3d> vertices = inputVertices;
		// sort the input vertices by x-coordinate
		std::sort(vertices.begin(), vertices.end(), compareVectorByXCoord);
		// calculate the initial triangulation which bounds the input vertices
		vector<Triangle> triangulation = makeBoundingTriangles(vertices);


		// INSERTING THE VERTICES
		// ======================

		size_t vertexCount = vertices.size() - 4;
		for (size_t iVertex = 0; iVertex < vertexCount; ++iVertex) {
			Vector3d & vertex = vertices[iVertex];

			vector<Edge> badEdges;

			size_t triangleCount = triangulation.size();
			for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle) {
				Triangle & triangle = triangulation[iTriangle];

				if (triangle.containsInCircumCircle(vertex)) {
					// triangle is bad, it must be cut out
					triangle.isBad = true;
					auto triangleEdges = triangle.getEdges(vertices);
					badEdges.insert(badEdges.end(), triangleEdges.begin(), triangleEdges.end());
				}
			}

			triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(), 
				[](Triangle t) { return t.isBad; }
				), triangulation.end());

			std::sort(badEdges.begin(), badEdges.end());
			for_each_nonrepeating(badEdges.begin(), badEdges.end(),
				[&](Edge & edge) {
				triangulation.push_back(edge.connectWithVertex(vertices, iVertex));
			});

		}


		// CONSTRUCTION OF THE 3DS MAX MESH
		// ================================

		size_t triangleCount = std::count_if(
			triangulation.begin(),
			triangulation.end(),
			[](Triangle & triangle) { return triangle.isBounding == false; }
		);

		Mesh* result = new Mesh;
		result->setNumVerts(vertexCount);
		result->setNumFaces(triangleCount);

		for (size_t iVertex = 0; iVertex < vertexCount; ++iVertex) {
			Vector3d & vertex = vertices[iVertex];
			result->setVert(int(iVertex), toPoint3(vertex));
		}

		size_t iFace = 0;
		for (Triangle & triangle : triangulation) {
			if (triangle.isBounding)
				continue;

			result->faces[iFace].v[0] = DWORD(triangle.v0);
			result->faces[iFace].v[1] = DWORD(triangle.v1);
			result->faces[iFace].v[2] = DWORD(triangle.v2);
			++iFace;
		}

		return result;
	}

	Edge::Edge(const vertexCollection & vertices, size_t v0, size_t v1)
		: v0(v0), v1(v1)
	{
		if (v1 < v0)
			std::swap(this->v0, this->v1);
	}

	Triangle Edge::connectWithVertex(const vertexCollection & vertices, size_t v)
	{
		return Triangle(vertices, v0, v1, v);
	}

	Triangle::Triangle(const vertexCollection & vertices, size_t i0, size_t i1, size_t i2)
		: v0(i0), v1(i1), v2(i2)
	{
		const Vector3d & vert0 = vertices[i0];
		const Vector3d & vert1 = vertices[i1];
		const Vector3d & vert2 = vertices[i2];

		const Vector2d v0 = { vert0.x(), vert0.y() };
		const Vector2d v1 = { vert1.x(), vert1.y() };
		const Vector2d v2 = { vert2.x(), vert2.y() };

		Matrix2d matrix;
		matrix <<
			2.0 * (v0 - v1),
			2.0 * (v0 - v2);
		matrix.transposeInPlace();

		Vector2d rhs;
		rhs <<
			squareSum(v0) - squareSum(v1), squareSum(v0) - squareSum(v2);

		Vector2d sol1 = matrix.fullPivHouseholderQr().solve(rhs);
		Vector2d sol2 = matrix.fullPivLu().solve(rhs);
		Vector2d sol3 = matrix.partialPivLu().solve(rhs);
		circumCenter = sol3;
		Vector2d x = v0 - circumCenter;
		circumRadiusSquared = squareSum(x);
	}

	std::vector<Edge> Triangle::getEdges(const vertexCollection & vertices) const
	{
		vector<Edge> result;
		result.push_back(Edge(vertices, v0, v1));
		result.push_back(Edge(vertices, v1, v2));
		result.push_back(Edge(vertices, v2, v0));
		return result;
	}

	bool Triangle::containsInCircumCircle(const Vector3d & vertex) const
	{
		double d = circumRadiusSquared;
		d -= square(vertex.x() - circumCenter.x());
		d -= square(vertex.y() - circumCenter.y());

		if (d > 0.0) {
			return true;
		}

		return false;
	}

}