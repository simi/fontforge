#include "Python.h"
#include "../inc/dynamic.h"

PyMODINIT_FUNC ENTRY_POINT(void) {
    DL_CONST void *lib;
    void (*initer)(void);

    if ( (lib = dlopen("libgunicode" SO_EXT,RTLD_LAZY))==NULL ) {
#ifdef PREFIX
	lib = dlopen( PREFIX "/lib/" "libgunicode" SO_EXT,RTLD_LAZY);
#endif
    }
    if ( lib==NULL ) {
	PyErr_Format(PyExc_SystemError,"Missing library: %s", "libgunicode");
return;
    }

    if ( (lib = dlopen("libgdraw" SO_EXT,RTLD_LAZY))==NULL ) {
#ifdef PREFIX
	lib = dlopen( PREFIX "/lib/" "libgdraw" SO_EXT,RTLD_LAZY);
#endif
    }
    if ( lib==NULL ) {
	PyErr_Format(PyExc_SystemError,"Missing library: %s", "libgdraw");
return;
    }

    if ( (lib = dlopen("libfontforge" SO_EXT,RTLD_LAZY))==NULL ) {
#ifdef PREFIX
	lib = dlopen( PREFIX "/lib/" "libfontforge" SO_EXT,RTLD_LAZY);
#endif
    }
    if ( lib==NULL ) {
	PyErr_Format(PyExc_SystemError,"Missing library: %s", "libfontforge");
return;
    }
    initer = dlsym(lib,"ff_init");
    if ( initer==NULL ) {
	PyErr_Format(PyExc_SystemError,"No initialization function in fontforge library");
return;
    }
    (*initer)();
}
