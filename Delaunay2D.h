#pragma once

namespace delaunay {

	class IDelaunay2D {
	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) = 0;
	};

	class BowyerWatson2D : public IDelaunay2D {
	public:
		virtual Mesh* invoke(const std::vector<Eigen::Vector3d> & vertices) override;
	};


	using vertexCollection = std::vector<Eigen::Vector3d>;

	struct Triangle;

	struct Edge {
		size_t v0, v1;

		Edge(const vertexCollection & vertices, size_t v0, size_t v1);

		Triangle connectWithVertex(const vertexCollection & vertices, size_t v);
	};

	struct Triangle {
		Eigen::Vector2d circumCenter;
		double circumRadiusSquared;
		size_t v0, v1, v2;
		bool isBounding = false;
		bool isBad = false;

		Triangle(const vertexCollection & vertices, size_t v0, size_t v1, size_t v2);

		std::vector<Edge> getEdges(const vertexCollection & vertices) const;
		bool containsInCircumCircle(const Eigen::Vector3d & vertex) const;
	};

	inline bool operator==(const Edge & lhs, const Edge & rhs) {
		return (lhs.v0 == rhs.v0) && (lhs.v1 == rhs.v1);
	}

	inline bool operator!=(const Edge & lhs, const Edge & rhs) {
		return !(lhs == rhs);
	}

	inline bool operator<(const Edge & lhs, const Edge & rhs) {
		if (lhs.v0 < rhs.v0)
			return true;

		if (lhs.v0 == rhs.v0)
			return (lhs.v1 == rhs.v1);

		return false;
	}

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

}