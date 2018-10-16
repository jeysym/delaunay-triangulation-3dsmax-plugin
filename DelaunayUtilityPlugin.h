#pragma once
#include "3dsmaxsdk_preinclude.h"
#include "resource.h"
#include "Delaunay3D.h"


enum class DelaunayFpFunctions {
	DELAUNAY
};

class DelaunayFpInterface : public FPStaticInterface {
	virtual Mesh* delaunay(Tab<Point3*>* vertices) = 0;
};

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;
