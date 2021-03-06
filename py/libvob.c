#include <Python.h>
#include <glib.h>
#include "structmember.h"

#include "util/sigsegv.h"
#include "util/array.h"
#include "gfx/gfx_api.h"
#include "gfx/animation.h"
#include "lob/api.h"
#include "lob/lobs.h"


static Region *region;


#include "gen_protos.c"
#include "gen_structs.c"



/* *********************
 * PyRegion type 
 ************************ */

typedef struct {
    PyObject_HEAD
    Region *_reg;
} PyRegion;

static void
PyRegion_dealloc(PyRegion* self)
{
	self->ob_type->tp_free((PyObject*)self);
}


static PyTypeObject PyRegionType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "libvob.Region",             /*tp_name*/
    sizeof(PyRegion),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    "Region object",         /* tp_doc */
};


/* *********************
 * GfxWindow type 
 ************************ */

typedef struct {
	PyObject_HEAD
	PyObject *create_lob;
	struct gfx_window *win;
} GfxWindow;

/** Global window instance.
 */
static GfxWindow *win_instance = NULL;


static void
GfxWindow_dealloc(GfxWindow* self)
{
	Py_XDECREF(self->create_lob);
	self->ob_type->tp_free((PyObject*)self);
}




static PyObject *
GfxWindow_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	if (win_instance != NULL) {
		PyErr_SetString(PyExc_NotImplementedError, 
				"Only one window supported.");
		return NULL;
	}

	GfxWindow *self = (GfxWindow *)type->tp_alloc(type, 0);
	win_instance = self;
	return (PyObject *)self;
}

static
Lob *cb_create_lob(Region *lobs_reg)
{
	PyObject *result;
	PyRegion *reg = PyType_GenericNew(&PyRegionType, NULL, NULL);

	// push region should be called XXX

	reg->_reg = lobs_reg;
	region = lobs_reg;

	//printf("eval\n", result);
	result = PyEval_CallFunction(win_instance->create_lob, "(O)", reg);
	if (PyErr_Occurred()) {
		PyErr_Print();
		abort();
	}
	Py_XDECREF(reg);

	//printf("create lob %d\n", result);
	Lob *ret = NULL;
	if (result != NULL /*&& PyType_isSubType(result, PyLob)*/) {
		PyLob *py_lob = (PyLob *) result;
		ret = py_lob->obj;
	}
	Py_XDECREF(result);
	//printf("create lob done\n");
	return ret;
}

static int
GfxWindow_init(GfxWindow *self, PyObject *args, PyObject *kwds)
{

	self->win = NULL;
	int 
		x = 0,
		y = 0,
		w = 200,
		h = 200;
	static char *kwlist[] = {"x", "y", "w", "h", NULL};
	if (! PyArg_ParseTupleAndKeywords(args, kwds, "|iiii", kwlist, 
					  &x, &y, &w, &h))
		return -1;
	
	self->win = gfx_create_window(x,y,w,h);
	struct gfx_callbacks *cb = gfx_callbacks(self->win);
	cb->create_lob = &cb_create_lob;
	
	return 0;
}


static PyMemberDef GfxWindow_members[] = {
    {"create_lob", T_OBJECT, offsetof(GfxWindow, create_lob), 0,
     "Callback method to create lob"},
    {NULL}  /* Sentinel */
};

static PyObject *
GfxWindow_name(GfxWindow* self)
{
    static PyObject *format = NULL;
    PyObject *args, *result;

    if (format == NULL) {
        format = PyString_FromString("%s %s");
        if (format == NULL)
            return NULL;
    }
    //result = PyString_Format(format, args);
    //Py_DECREF(args);
    
    return result;
}

static PyMethodDef GfxWindow_methods[] = {
    {"name", (PyCFunction)GfxWindow_name, METH_NOARGS,
     "Return the name, combining the first and last name"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject GfxWindowType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "libvob.GfxWindow",             /*tp_name*/
    sizeof(GfxWindow),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)GfxWindow_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "GfxWindow objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    GfxWindow_methods,             /* tp_methods */
    GfxWindow_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)GfxWindow_init,      /* tp_init */
    0,                         /* tp_alloc */
    GfxWindow_new,                 /* tp_new */
};





static PyObject *
main_loop(PyObject *self, PyObject *win)
{
	GfxWindow *w = (GfxWindow *)win;

	gfx_main_loop(w->win);
	return NULL;
}

static PyMethodDef module_methods[] = {
    {"main_loop", (PyCFunction)main_loop, METH_O,
     "Starts the main loop and never returns."
    },
    {NULL}
};
static PyMethodDef non_methods[] = {
    {NULL}
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initlibvob(void) 
{
    PyObject *libvob, *lob, *vob, *m;

#include "gen_init_types.c"

    if (PyType_Ready(&GfxWindowType) < 0)
        return;

    PyRegionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyRegionType) < 0)
        return;


    libvob = Py_InitModule3("libvob", module_methods,
			    "Python bindings for libvob-c library.");
    lob = Py_InitModule3("libvob.lob", non_methods,
			 "Layoutable objects module.");
    vob = Py_InitModule3("libvob.vob", non_methods,
			 "Visual objects module.");

    if (libvob == NULL || lob == NULL || vob == NULL)
      return;

    // Add lob and vob submodules to this.
    PyModule_AddObject(libvob, "lob", lob);
    PyModule_AddObject(libvob, "vob", vob);

#include "gen_add_obs.c"

    Py_INCREF(&GfxWindowType);
    PyModule_AddObject(libvob, "Window", (PyObject *)&GfxWindowType);

    Py_INCREF(&PyRegionType);
    PyModule_AddObject(libvob, "Region", (PyObject *)&PyRegionType);

    setup_sigsegv();
}
