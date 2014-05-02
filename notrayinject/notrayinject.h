// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NOTRAYINJECT_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NOTRAYINJECT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NOTRAYINJECT_EXPORTS
#define NOTRAYINJECT_API __declspec(dllexport)
#else
#define NOTRAYINJECT_API __declspec(dllimport)
#endif



NOTRAYINJECT_API void inject(void);
