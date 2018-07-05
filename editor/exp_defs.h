#ifndef EXPORT_IMPORT_DEFINES
#define EXPORT_IMPORT_DEFINES



#ifdef BUILDING_DLL
	#define DLLIMPEXP __declspec((dllexport))
#else
	#define DLLIMPEXP __declspec((dllimport))
#endif


#define MASAPI		



#endif
