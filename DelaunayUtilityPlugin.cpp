#include "stdafx.h"
#include "DelaunayUtilityPlugin.h"

#define DelaunayUtilityPlugin_CLASS_ID	Class_ID(0x73620695, 0x651f0af4)
#define DelaunayUtilityPlugin_FP_INTERFACE_ID Interface_ID(0x789b0621, 0xa5ef0346)

using std::vector;
using Eigen::Vector3d;
using std::unique_ptr;
using std::make_unique;

class DelaunayUtilityPlugin;

vector<Vector3d> makeVector(Tab<Point3*>* vertices) {
	vector<Vector3d> result;
	Tab<Point3*> & verticesTab = *vertices;
	size_t vertexCount = (size_t)verticesTab.Count();

	result.reserve(vertexCount);
	for (size_t i = 0; i < vertexCount; ++i) {
		Point3 & vertex = *verticesTab[i];
		result.push_back(Vector3d(vertex.x, vertex.y, vertex.z));
	}

	return result;
}

class DelaunayUtilityPlugin : public UtilityObj 
{
public:
		
	//Constructor/Destructor
	DelaunayUtilityPlugin();
	virtual ~DelaunayUtilityPlugin();

	virtual void DeleteThis() { }
	
	virtual void BeginEditParams(Interface *ip,IUtil *iu);
	virtual void EndEditParams(Interface *ip,IUtil *iu);

	virtual void Init(HWND hWnd);
	virtual void Destroy(HWND hWnd);
	
	// Singleton access
	static DelaunayUtilityPlugin* GetInstance();

	Mesh* triangulate2D(Tab<Point3*>* vertices) {
		unique_ptr<delaunay::IDelaunay2D> algorithm = make_unique<delaunay::BowyerWatson2D>();
		return algorithm->invoke(makeVector(vertices));
	}

	Mesh* triangulate3D(Tab<Point3*>* vertices) {
		unique_ptr<delaunay::IDelaunay3D> algorithm =  make_unique<delaunay::BowyerWatson3D>();
		return algorithm->invoke(makeVector(vertices));
	}

private:

	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND   hPanel;
	IUtil* iu;
};

class DelaunayUtilityPluginClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 	{ return DelaunayUtilityPlugin::GetInstance(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return UTILITY_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return DelaunayUtilityPlugin_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("DelaunayUtilityPlugin"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};

class DelaunayFpImplementation : public DelaunayFpInterface {
	DECLARE_DESCRIPTOR(DelaunayFpImplementation)
	BEGIN_FUNCTION_MAP
		FN_1((int)DelaunayFpFunctions::DELAUNAY2D, TYPE_MESH, delaunay2D, TYPE_POINT3_TAB)
		FN_1((int)DelaunayFpFunctions::DELAUNAY3D, TYPE_MESH, delaunay3D, TYPE_POINT3_TAB)
	END_FUNCTION_MAP

	virtual Mesh* delaunay2D(Tab<Point3*>* vertices) {
		return DelaunayUtilityPlugin::GetInstance()->triangulate2D(vertices);
	}

	virtual Mesh* delaunay3D(Tab<Point3*>* vertices) {
		return DelaunayUtilityPlugin::GetInstance()->triangulate3D(vertices);
	}
};

static DelaunayUtilityPlugin theDelaunayUtilityPlugin;
static DelaunayUtilityPluginClassDesc delaunayUtilityPluginDesc;
static DelaunayFpImplementation delaunayFpImplementationDesc(
	// INTERFACE ID | INTERNAL NAME | DESCRIPTION | CLASS DESCRIPTOR | FLAGS
	DelaunayUtilityPlugin_FP_INTERFACE_ID, _T("Algorithms"), IDS_FPI_ALGO, &delaunayUtilityPluginDesc, 0,
	// Here starts the var-args magic.
	// FUNCTION ID | INTERNAL NAME | LOCALIZABLE DESCRIPTION | RETURN TYPE | FLAGS | PARAMETER COUNT
	// for each parameter: INTERNAL PARAMETER NAME | LOCALIZABLE DESCRIPTION | TYPE
	(int)DelaunayFpFunctions::DELAUNAY2D, _T("delaunay2D"), IDS_FN_DELAUNAY2D, TYPE_MESH, 0, 1,
	_T("vertices"), IDS_FNP_VERTICES, TYPE_POINT3_TAB,

	(int)DelaunayFpFunctions::DELAUNAY3D, _T("delaunay3D"), IDS_FN_DELAUNAY3D, TYPE_MESH, 0, 1,
	_T("vertices"), IDS_FNP_VERTICES, TYPE_POINT3_TAB,
	p_end
);

ClassDesc2* GetDelaunayUtilityPluginDesc() { 
	return &delaunayUtilityPluginDesc; 
}

DelaunayUtilityPlugin* DelaunayUtilityPlugin::GetInstance() {
	return &theDelaunayUtilityPlugin;
}






//--- DelaunayUtilityPlugin -------------------------------------------------------
DelaunayUtilityPlugin::DelaunayUtilityPlugin()
	: hPanel(nullptr)
	, iu(nullptr)
{
	
}

DelaunayUtilityPlugin::~DelaunayUtilityPlugin()
{

}

void DelaunayUtilityPlugin::BeginEditParams(Interface* ip,IUtil* iu) 
{
	this->iu = iu;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		DlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void DelaunayUtilityPlugin::EndEditParams(Interface* ip,IUtil*)
{
	this->iu = nullptr;
	ip->DeleteRollupPage(hPanel);
	hPanel = nullptr;
}

void DelaunayUtilityPlugin::Init(HWND /*handle*/)
{

}

void DelaunayUtilityPlugin::Destroy(HWND /*handle*/)
{

}

INT_PTR CALLBACK DelaunayUtilityPlugin::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
			DelaunayUtilityPlugin::GetInstance()->Init(hWnd);
			break;

		case WM_DESTROY:
			DelaunayUtilityPlugin::GetInstance()->Destroy(hWnd);
			break;

		case WM_COMMAND:
			#pragma message(TODO("React to the user interface commands.  A utility plug-in is controlled by the user from here."))
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			GetCOREInterface()->RollupMouseMessage(hWnd,msg,wParam,lParam);
			break;

		default:
			return 0;
	}
	return 1;
}
