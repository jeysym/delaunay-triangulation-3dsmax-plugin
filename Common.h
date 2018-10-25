#pragma once
#include "stdafx.h"

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

inline static double squareSum(const Eigen::Vector2d & vector) {
	return (square(vector.x()) + square(vector.y()));
}

inline static double squareSum(const Eigen::Vector3d & vector) {
	return (square(vector.x()) + square(vector.y()) + square(vector.z()));
}

inline static bool compareVectorByXCoord(const Eigen::Vector3d & v0, const Eigen::Vector3d & v1) {
	return v0.x() < v1.x();
}

inline static Eigen::Vector2d toVector2d(const Eigen::Vector3d & vec) {
	return Eigen::Vector2d(vec.x(), vec.y());
}

inline static Point3 toPoint3(const Eigen::Vector3d & vec) {
	return Point3(vec.x(), vec.y(), vec.z());
}