#include "stdafx.h"
#include "TetraMesh.h"

AxisAlignedBox::AxisAlignedBox(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
	: xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax)
{
}

double AxisAlignedBox::getLongestEdge() const
{
	double lenX = xMax - xMin;
	double lenY = yMax - yMin;
	double lenZ = zMax - zMin;
	return std::max({ lenX, lenY, lenZ });
}

void AxisAlignedBox::enlargeBy(double amount)
{
	xMin -= amount;
	xMax += amount;

	yMin -= amount;
	yMax += amount;

	zMin -= amount;
	zMax += amount;
}

AxisAlignedBox AxisAlignedBox::getBoundingBox(std::vector<Eigen::Vector3d> vertices)
{
	double xMin, xMax;
	double yMin, yMax;
	double zMin, zMax;

	xMin = xMax = yMin = yMax = zMin = zMax = 0.0;

	for (auto it = vertices.begin(); it != vertices.end(); ++it) {
		double x = it->x();
		double y = it->y();
		double z = it->z();

		if (x < xMin) xMin = x;
		if (x > xMax) xMax = x;
		if (y < yMin) yMin = y;
		if (y > yMax) yMax = y;
		if (z < zMin) zMin = z;
		if (z > zMax) zMax = z;
	}

	return AxisAlignedBox(xMin, xMax, yMin, yMax, zMin, zMax);
}
