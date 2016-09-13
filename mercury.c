/*
 * Copyright (c) 2016 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <tm_reader.h>

#include <Python.h>
#include <structmember.h>

#define MAX_ANTENNA_COUNT 4

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    TMR_Reader reader;
    uint8_t antennas[MAX_ANTENNA_COUNT];
} Reader;

static PyObject *
Reader_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Reader *self;
    char *deviceUri;
    int baudRate = 115200;
    TMR_Status ret;

    static char *kwlist[] = {"uri", "baudrate", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &deviceUri, &baudRate))
        return NULL;

    self = (Reader *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    if ((ret = TMR_create(&self->reader, deviceUri)) != TMR_SUCCESS)
        goto fail;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_BAUDRATE, &baudRate)) != TMR_SUCCESS)
        goto fail;

    if ((ret = TMR_connect(&self->reader)) != TMR_SUCCESS)
        goto fail;

    return (PyObject *)self;
fail:
    PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

static void
Reader_dealloc(Reader* self)
{
    TMR_destroy(&self->reader);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef Reader_members[] = {
    {NULL}  /* Sentinel */
};

typedef struct {
    const char* name;
    TMR_Region region;
} Regions;

static Regions Reader_regions[] = {
    {"NA",   TMR_REGION_NA},
    {"EU",   TMR_REGION_EU},
    {"KR",   TMR_REGION_KR},
    {"IN",   TMR_REGION_IN},
    {"JP",   TMR_REGION_JP},
    {"PRC",  TMR_REGION_PRC},
    {"EU2",  TMR_REGION_EU2},
    {"EU3",  TMR_REGION_EU3},
    {"KR2",  TMR_REGION_KR2},
    {"PRC2", TMR_REGION_PRC2},
    {"AU",   TMR_REGION_AU},
    {"NZ",   TMR_REGION_NZ},
    {"NA2",  TMR_REGION_NA2},
    {"NA3",  TMR_REGION_NA3},
    {NULL,   TMR_REGION_NONE}
};

static TMR_Region str2region(const char *name)
{
    Regions *reg;
    for(reg = Reader_regions; reg->name != NULL; reg++)
    {
        if(strcmp(reg->name, name) == 0)
            return reg->region;
    }

    return TMR_REGION_NONE;
}

static const char* region2str(TMR_Region region)
{
    Regions *reg;
    for(reg = Reader_regions; reg->name != NULL; reg++)
    {
        if(reg->region == region)
            return reg->name;
    }

    return NULL;
}

static PyObject *
Reader_get_supported_regions(Reader* self)
{
    int i;
    TMR_RegionList regions;
    TMR_Region regionStore[32];
    TMR_Status ret;
    PyObject *list;

    regions.list = regionStore;
    regions.max = sizeof(regionStore)/sizeof(regionStore[0]);
    regions.len = 0;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* create empty list */
    list = PyList_New(0);

    for (i = 0; i < regions.len; i++)
    {
        const char* name = region2str(regions.list[i]);
        if (name != NULL)
            PyList_Append(list, PyUnicode_FromString(name));
    }

    return list;
}

static PyObject *
Reader_set_region(Reader *self, PyObject *args)
{
    char *s;
    TMR_Region region;
    TMR_Status ret;

    if (!PyArg_ParseTuple(args, "s", &s))
        return NULL;

    if ((region = str2region(s)) == TMR_REGION_NONE)
    {
        PyErr_SetString(PyExc_TypeError, "Unknown region");
        return NULL;
    }

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_REGION_ID, &region)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* Do something interesting here. */
    Py_RETURN_NONE;
}

typedef struct {
    const char* name;
    TMR_TagProtocol protocol;
} Protocols;

static Protocols Reader_protocols[] = {
    {"ISO180006B", TMR_TAG_PROTOCOL_ISO180006B},
    {"GEN2", TMR_TAG_PROTOCOL_GEN2},
    {"UCODE", TMR_TAG_PROTOCOL_ISO180006B_UCODE},
    {"IPX64", TMR_TAG_PROTOCOL_IPX64},
    {"IPX256", TMR_TAG_PROTOCOL_IPX256},
    {"ATA", TMR_TAG_PROTOCOL_ATA},
    {NULL, TMR_TAG_PROTOCOL_NONE}
};

static TMR_TagProtocol str2protocol(const char *name)
{
    Protocols *prot;
    for(prot = Reader_protocols; prot->name != NULL; prot++)
    {
        if(strcmp(prot->name, name) == 0)
            return prot->protocol;
    }

    return TMR_TAG_PROTOCOL_NONE;
}

static
uint8_t as_uint8(PyObject *item)
{
    long num;

    if (!PyLong_Check(item))
        return 255;

    num = PyLong_AsLong(item);
    if (num < 0 || num > 255)
        return 255;

    return (uint8_t)num;
}

static PyObject *
Reader_set_read_plan(Reader *self, PyObject *args)
{
    PyObject *list;
    char *s;
    TMR_TagProtocol protocol;
    TMR_ReadPlan plan;
    TMR_Status ret;
    int i;
    uint8_t ant_count;

    if (!PyArg_ParseTuple(args, "O!s", &PyList_Type, &list, &s))
        return NULL;

    if ((protocol = str2protocol(s)) == TMR_TAG_PROTOCOL_NONE)
    {
        PyErr_SetString(PyExc_TypeError, "unknown protocol");
        return NULL;
    }

    if ((ant_count = PyList_Size(list)) > 4)
    {
        PyErr_SetString(PyExc_TypeError, "Too many antennas");
        return NULL;
    }

    for (i = 0; i < ant_count; i++)
    {
        uint8_t num;
        if ((num = as_uint8(PyList_GetItem(list, i))) == 255)
        {
            PyErr_SetString(PyExc_TypeError, "expecting a list of 8-bit integers");
            return NULL;
        }

        self->antennas[i] = num;
    }

    if ((ret = TMR_RP_init_simple(&plan, ant_count, self->antennas, protocol, 1000)) != TMR_SUCCESS)
        goto fail;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_READ_PLAN, &plan)) != TMR_SUCCESS)
        goto fail;

    Py_RETURN_NONE;
fail:
    PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
    return NULL;
}

static PyObject *
Reader_read(Reader *self, PyObject *args)
{
    PyObject *list;
    TMR_Status ret;

    ret = TMR_read(&self->reader, 500, NULL);
    /* In case of TAG ID Buffer Full, extract the tags present in buffer. */
    if (ret != TMR_SUCCESS && ret != TMR_ERROR_TAG_ID_BUFFER_FULL)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* create empty list */
    list = PyList_New(0);

    while (TMR_hasMoreTags(&self->reader) == TMR_SUCCESS)
    {
        TMR_TagReadData data;
        char epcStr[128];

        if ((ret = TMR_getNextTag(&self->reader, &data)) != TMR_SUCCESS)
        {
            PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
            return NULL;
        }

        TMR_bytesToHex(data.tag.epc, data.tag.epcByteCount, epcStr);
        PyList_Append(list, Py_BuildValue("(Oi)", PyBytes_FromString(epcStr), data.readCount));
    }

    return list;
}

static PyObject *
Reader_get_model(Reader* self)
{
    TMR_String model;
    char str[64];
    TMR_Status ret;

    model.value = str;
    model.max = sizeof(str);

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_VERSION_MODEL, &model)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyUnicode_FromString(model.value);
}

static PyMethodDef Reader_methods[] = {
    {"get_supported_regions", (PyCFunction)Reader_get_supported_regions, METH_NOARGS,
     "Returns a list of regions supported by the reader"
    },
    {"set_region", (PyCFunction)Reader_set_region, METH_VARARGS,
     "Set the reader region"
    },
    {"set_read_plan", (PyCFunction)Reader_set_read_plan, METH_VARARGS,
     "Set the read plan"
    },
    {"read", (PyCFunction)Reader_read, METH_VARARGS,
     "Read the tags"
    },
    {"get_model", (PyCFunction)Reader_get_model, METH_NOARGS,
     "Returns the model name"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject ReaderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mercury.Reader",          /* tp_name */
    sizeof(Reader),            /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Reader_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Reader object",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Reader_methods,            /* tp_methods */
    Reader_members,            /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    Reader_new,                /* tp_new */
};

static PyModuleDef mercurymodule = {
    PyModuleDef_HEAD_INIT,
    "mercury",
    "ThingMagic Mercury API to control ThingMagic RFID readers.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_mercury(void)
{
    PyObject* m;

    if (PyType_Ready(&ReaderType) < 0)
        return NULL;

    m = PyModule_Create(&mercurymodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ReaderType);
    PyModule_AddObject(m, "Reader", (PyObject *)&ReaderType);
    return m;
}

/* end of file */
