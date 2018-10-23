#pragma once
#include "3dsmaxsdk_preinclude.h"
#include "resource.h"
#include "Delaunay3D.h"
#include "Delaunay2D.h"


enum class DelaunayFpFunctions {
	DELAUNAY2D, DELAUNAY3D
};

class DelaunayFpInterface : public FPStaticInterface {
	virtual Mesh* delaunay2D(Tab<Point3*>* vertices) = 0;
	virtual Mesh* delaunay3D(Tab<Point3*>* vertices) = 0;
};

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;
