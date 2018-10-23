#pragma once
/// Precompiled headers file. 
/// \author Jan Bryda
#include <Eigen/Dense>	// Eigen is math/linear algebra library

// 3ds Max SDK includes
// ====================
#include <maxtypes.h>
#include <point3.h>		// Point3
#include <tab.h>		// Tab<>
#include <istdplug.h>	
#include <iparamb2.h>
#include <iparamm2.h>
#include <ifnpub.h>		// Function publishing: FPStaticInterface
#include <utilapi.h>

// undef the "min" and "max" macro that is defined in the 3ds Max SDK
#undef min
#undef max


// C++ STD library includes
// ========================
#include <vector>			// vector
#include <memory>			// shared_ptr, unique_ptr, ...
#include <string>			// string
#include <algorithm>		// max
#include <initializer_list>	// {}
#include <tuple>			// tuple
#include <map>
#include <set>


// Other includes
// ==============
