#pragma once
#include "3dsmaxsdk_preinclude.h"
#include "resource.h"
#include "Delaunay3D.h"
#include "Delaunay2D.h"

/// Function Publishing IDs for functions.
enum class DelaunayFpFunctions {
	DELAUNAY2D,		///< Funciton that provides user with 2D delaunay triangulation capability.
	DELAUNAY3D		///< Function that provides user with 3D delaunay tetrahedration capability.
};

/// Abstract interface class that serves as FP interface.
class DelaunayFpInterface : public FPStaticInterface {
	/// Call the 2D delaunay triangulation algorithm on the vertices from the mesh. 
	virtual Mesh* delaunay2D(Mesh* mesh) = 0;

	/// Call the 3D delaunay tetrahedration algorithm on the vertices from the mesh.
	virtual Mesh* delaunay3D(Mesh* mesh) = 0;
};

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;
