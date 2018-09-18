/*
 * Copyright (c) 2016-2018 Petr Gotthard <petr.gotthard@centrum.cz>
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
#define MAX_DATA_AREA 258
#define numberof(x) (sizeof((x))/sizeof((x)[0]))

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    TMR_Reader reader;
    uint8_t antennas[MAX_ANTENNA_COUNT];
    TMR_ReadListenerBlock readListener;
    PyObject *readCallback;
} Reader;

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    TMR_TagData tag;
} TagData;

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    TMR_TagReadData data;
    PyObject *epcMemData;
    PyObject *tidMemData;
    PyObject *userMemData;
    PyObject *reservedMemData;
} TagReadData;

static PyTypeObject TagReadDataType;

static void
invoke_read_callback(TMR_Reader *reader, const TMR_TagReadData *tag, void *cookie);

static PyObject *
Reader_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Reader *self;
    char *deviceUri;
    int baudRate = 0;
    TMR_Status ret;

    static char *kwlist[] = {"uri", "baudrate", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &deviceUri, &baudRate))
        return NULL;

    self = (Reader *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    if ((ret = TMR_create(&self->reader, deviceUri)) != TMR_SUCCESS)
        goto fail;

    if (baudRate > 0)
    {
        if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_BAUDRATE, &baudRate)) != TMR_SUCCESS)
            goto fail;
    }

    /* install the callback wrapper for asynchronous reading */
    self->readListener.listener = invoke_read_callback;
    self->readListener.cookie = self;
    if ((ret = TMR_addReadListener(&self->reader, &self->readListener)) != TMR_SUCCESS)
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
    {"IS",   TMR_REGION_IS},
    {"open", TMR_REGION_OPEN},
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
        {
            PyObject *sname = PyUnicode_FromString(name);
            PyList_Append(list, sname);
            Py_DECREF(sname);
        }
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

    if (PyLong_Check(item))
        num = PyLong_AsLong(item);
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(item))
        num = PyInt_AsLong(item);
#endif
    else
        return 255;

    if (num < 0 || num > 255)
        return 255;

    return (uint8_t)num;
}

static int str2bank(PyObject *name)
{
    const char *text;

    if (PyBytes_Check(name))
        text = PyBytes_AsString(name);
#if PY_MAJOR_VERSION >= 3
    else if (PyUnicode_Check(name))
        text = PyUnicode_AsUTF8(name);
#endif
    else
    {
        PyErr_SetString(PyExc_TypeError, "expecting string");
        return 0;
    }

    if(strcmp(text, "reserved") == 0)
        return TMR_GEN2_BANK_RESERVED_ENABLED;
    else if(strcmp(text, "epc") == 0)
        return TMR_GEN2_BANK_EPC | TMR_GEN2_BANK_EPC_ENABLED;
    else if(strcmp(text, "tid") == 0)
        return TMR_GEN2_BANK_TID | TMR_GEN2_BANK_TID_ENABLED;
    else if(strcmp(text, "user") == 0)
        return TMR_GEN2_BANK_USER | TMR_GEN2_BANK_USER_ENABLED;
    else
    {
        PyErr_SetString(PyExc_TypeError, "invalid bank name");
        return 0;
    }
}

static PyObject *
Reader_set_read_plan(Reader *self, PyObject *args, PyObject *kwds)
{
    PyObject *list;
    char *s;
    TMR_TagProtocol protocol;
    TMR_ReadPlan plan;
    TMR_Status ret;
    int i;
    uint8_t ant_count;
    PyObject *bank = NULL;
    int readPower = 0;

    static char *kwlist[] = {"antennas", "protocol", "bank", "read_power", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!s|Oi", kwlist, &PyList_Type, &list, &s, &bank, &readPower))
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

    if (bank != NULL)
    {
        int op = 0;
        TMR_TagOp tagop;

        if(PyList_Check(bank))
        {
            for (i = 0; i < PyList_Size(bank); i++)
            {
                int op2;

                if ((op2 = str2bank(PyList_GetItem(bank, i))) == 0)
                    return NULL;

                op |= op2;
            }
        }
        else
        {
            if ((op = str2bank(bank)) == 0)
                return NULL;
        }

        if ((ret = TMR_TagOp_init_GEN2_ReadData(&tagop, op, 0, 0)) != TMR_SUCCESS)
            goto fail;

        if ((ret = TMR_RP_set_tagop(&plan, &tagop)) != TMR_SUCCESS)
            goto fail;
    }

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_READ_PLAN, &plan)) != TMR_SUCCESS)
        goto fail;

    if (readPower > 0)
    {
        if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_RADIO_READPOWER, &readPower)) != TMR_SUCCESS)
            goto fail;
    }

    Py_RETURN_NONE;
fail:
    PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
    return NULL;
}

static PyObject *
Reader_get_temperature(Reader *self)
{
    TMR_Status ret;
    uint8_t temp;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_TEMPERATURE, &temp)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, "Error getting temperature");
        return NULL;
    }

    return PyLong_FromLong(temp);
}

static PyObject *
Reader_get_antennas(Reader *self)
{
    int i;
    TMR_Status ret;
    PyObject *antennas;
    TMR_uint8List port_list;
    uint8_t value_list[MAX_ANTENNA_COUNT];

    port_list.list = value_list;
    port_list.max = numberof(value_list);

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_ANTENNA_PORTLIST, &port_list)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, "Error getting antennas");
        return NULL;
    }

    antennas = PyList_New(0);
    for (i = 0; i < port_list.len && i < port_list.max; i++)
    {
        PyList_Append(antennas, PyLong_FromLong((long) port_list.list[i]));
    }
    return antennas;
}

static PyObject *
Reader_get_power_range(Reader *self)
{
    int lim_power;
    TMR_Status ret;
    PyObject *powers;
    powers = PyTuple_New(2);

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_POWERMIN, &lim_power)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }
    PyTuple_SetItem(powers, 0, PyLong_FromLong((long) lim_power));

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_POWERMAX, &lim_power)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }
    PyTuple_SetItem(powers, 1, PyLong_FromLong((long) lim_power));
    return powers;
}

static PyObject *
Reader_get_read_powers(Reader *self)
{
    int row;
    TMR_Status ret;
    PyObject *antenna_power;
    PyObject *antenna_powers;
    antenna_powers = PyList_New(0);
    TMR_PortValueList ant_pow_list;
    TMR_PortValue pow_value_list[MAX_ANTENNA_COUNT];

    ant_pow_list.list = pow_value_list;
    ant_pow_list.max = numberof(pow_value_list);

    TMR_uint8List port_list;
    uint8_t port_value_list[MAX_ANTENNA_COUNT];

    port_list.list = port_value_list;
    port_list.max = numberof(port_value_list);

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_ANTENNA_PORTLIST, &port_list)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, "Error getting antennas");
        return NULL;
    }

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_PORTREADPOWERLIST, &ant_pow_list)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    for (row = 0; row < port_list.len; row++)
    {
        antenna_power = PyTuple_New(2);
        PyTuple_SetItem(antenna_power, 0, PyLong_FromLong((long) ant_pow_list.list[row].port));
        PyTuple_SetItem(antenna_power, 1, PyLong_FromLong((long) ant_pow_list.list[row].value));
        PyList_Append(antenna_powers, antenna_power);
    }

    return antenna_powers;
}

static PyObject *
Reader_set_read_powers(Reader *self, PyObject *args, PyObject *kwds)
{
    int length;
    TMR_Status ret;
    uint8_t ant_count, pow_count;
    PyObject *power_list, *antenna_list;
    static char *kwlist[] = {"antennas", "powers", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!O!", kwlist, &PyList_Type, &antenna_list, &PyList_Type, &power_list))
        return NULL;
    if ((ant_count = PyList_Size(antenna_list)) > MAX_ANTENNA_COUNT)
    {
        PyErr_SetString(PyExc_TypeError, "Too many antennas");
        return NULL;
    }
    if ((pow_count = PyList_Size(power_list)) > MAX_ANTENNA_COUNT)
    {
        PyErr_SetString(PyExc_TypeError, "Too many powers");
        return NULL;
    }
    if (pow_count != ant_count)
    {
        PyErr_SetString(PyExc_TypeError, "Number of antennas and powers not matching");
        return NULL;
    }
    length = (int) ant_count;

    int row;
    int power;
    int antenna;
    TMR_PortValueList ant_pow_list;
    TMR_PortValue value_list[MAX_ANTENNA_COUNT];

    ant_pow_list.len = length;
    ant_pow_list.max = numberof(value_list);
    ant_pow_list.list = value_list;
    for (row = 0; row < length; row++)
    {
        power = (int) PyLong_AsLong(PyList_GetItem(power_list, row));
        antenna = (int) PyLong_AsLong(PyList_GetItem(antenna_list, row));

        if ((ant_pow_list.list[row].port = antenna) == 255)
        {
            PyErr_SetString(PyExc_TypeError, "antennas expecting a list of integers");
            return NULL;
        }
        if ((ant_pow_list.list[row].value = power) == 255)
        {
            PyErr_SetString(PyExc_TypeError, "powers expecting a list of integers");
            return NULL;
        }
    }

    if (length > 0)
    {
        if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_RADIO_PORTREADPOWERLIST, &ant_pow_list)) != TMR_SUCCESS)
            goto fail;
    }

    return Reader_get_read_powers(self);
fail:
    PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
    return NULL;
}

static PyObject *
Reader_write(Reader *self, PyObject *args, PyObject *kwds)
{
    char* epc_data;
    char* epc_target;
    TMR_Status ret;
    TMR_TagData data;
    TMR_TagData target;
    TMR_TagFilter filter;
    // Read call arguments.
    static char *kwlist[] = {"epc_target", "epc_code", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &epc_target, &epc_data))
        return NULL;
    /* build target tag to search */
    target.epcByteCount = strlen(epc_target) * sizeof(char) / 2;
    TMR_hexToBytes(epc_target, target.epc, target.epcByteCount, NULL);
    /* build data tag to be writen */
    data.epcByteCount = strlen(epc_data) * sizeof(char) / 2;
    TMR_hexToBytes(epc_data, data.epc, data.epcByteCount, NULL);
    // Build filter target tag to be replaced.
    TMR_TF_init_tag(&filter, &target);
    // Write data tag on target tag.
    ret = TMR_writeTag(&self->reader, &filter, &data);
    // In case of not target tag found.
    if (ret == TMR_ERROR_NO_TAGS_FOUND)
        Py_RETURN_FALSE;
    Py_RETURN_TRUE;
}

static PyObject *
PyByteArray_FromUInt8List(const TMR_uint8List *array)
{
    if(array->len > 0)
        return PyByteArray_FromStringAndSize((const char *)array->list, array->len < array->max ? array->len : array->max);
    else
        return NULL;
}

static PyObject *
Reader_read(Reader *self, PyObject *args, PyObject *kwds)
{
    int timeout = 500;
    PyObject *list;
    TMR_Status ret;

    static char *kwlist[] = {"timeout", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &timeout))
        return NULL;

    ret = TMR_read(&self->reader, timeout, NULL);
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
        TagReadData *tag;
        uint8_t dataBuf1[MAX_DATA_AREA];
        uint8_t dataBuf2[MAX_DATA_AREA];
        uint8_t dataBuf3[MAX_DATA_AREA];
        uint8_t dataBuf4[MAX_DATA_AREA];

        tag = PyObject_New(TagReadData, &TagReadDataType);
        TMR_TRD_init(&tag->data);

        TMR_TRD_MEMBANK_init_data(&tag->data.epcMemData, MAX_DATA_AREA, dataBuf1);
        TMR_TRD_MEMBANK_init_data(&tag->data.tidMemData, MAX_DATA_AREA, dataBuf2);
        TMR_TRD_MEMBANK_init_data(&tag->data.userMemData, MAX_DATA_AREA, dataBuf3);
        TMR_TRD_MEMBANK_init_data(&tag->data.reservedMemData, MAX_DATA_AREA, dataBuf4);

        if ((ret = TMR_getNextTag(&self->reader, &tag->data)) != TMR_SUCCESS)
        {
            PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
            return NULL;
        }

        tag->epcMemData = PyByteArray_FromUInt8List(&tag->data.epcMemData);
        tag->tidMemData = PyByteArray_FromUInt8List(&tag->data.tidMemData);
        tag->userMemData = PyByteArray_FromUInt8List(&tag->data.userMemData);
        tag->reservedMemData = PyByteArray_FromUInt8List(&tag->data.reservedMemData);

        PyList_Append(list, (PyObject *)tag);
        Py_XDECREF(tag);
    }

    return list;
}

static PyObject *
Reader_start_reading(Reader *self, PyObject *args, PyObject *kwds)
{
    PyObject *temp;
    int onTime = 250;
    int offTime = 0;
    TMR_Status ret;

    static char *kwlist[] = {"callback", "on_time", "off_time", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii", kwlist, &temp, &onTime, &offTime))
        return NULL;

    if (!PyCallable_Check(temp))
    {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return NULL;
    }

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_READ_ASYNCONTIME, &onTime)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_READ_ASYNCOFFTIME, &offTime)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_TypeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    Py_XDECREF(self->readCallback);
    Py_XINCREF(temp);
    self->readCallback = temp;

    if ((ret = TMR_startReading(&self->reader)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    Py_RETURN_NONE;
}

static void
invoke_read_callback(TMR_Reader *reader, const TMR_TagReadData *pdata, void *cookie)
{
    Reader *self = (Reader *)cookie;

    if(self && self->readCallback)
    {
        TagReadData *tag;
        PyObject *arglist;
        PyObject *result;
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();

        tag = PyObject_New(TagReadData, &TagReadDataType);
        /* make a hard-copy */
        memcpy(&tag->data, pdata, sizeof(TMR_TagReadData));

        arglist = Py_BuildValue("(O)", tag);
        result = PyObject_CallObject(self->readCallback, arglist);
        if(result != NULL)
            Py_DECREF(result);
        else
            PyErr_Print();
        Py_DECREF(arglist);
        Py_DECREF(tag);

        PyGILState_Release(gstate);
    }
}

static PyObject *
Reader_stop_reading(Reader* self)
{
    PyObject *temp = self->readCallback;
    TMR_Status ret;

    /* avoid deadlock as calling stopReading will invoke the callback */
    self->readCallback = NULL;

    if ((ret = TMR_stopReading(&self->reader)) != TMR_SUCCESS)
    {
        self->readCallback = temp; /* revert back as the function will fail */

        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    Py_XDECREF(temp);
    Py_RETURN_NONE;
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
    {"get_temperature", (PyCFunction)Reader_get_temperature, METH_NOARGS,
     "Returns the chip temperature"
    },
    {"get_antennas", (PyCFunction)Reader_get_antennas, METH_NOARGS,
     "Lists available antennas."
    },
    {"get_power_range", (PyCFunction)Reader_get_power_range, METH_NOARGS,
     "Lists supported radio power range."
    },
    {"get_read_powers", (PyCFunction)Reader_get_read_powers, METH_NOARGS,
     "Lists configured read powers for each antenna."
    },
    {"get_supported_regions", (PyCFunction)Reader_get_supported_regions, METH_NOARGS,
     "Returns a list of regions supported by the reader"
    },
    {"set_region", (PyCFunction)Reader_set_region, METH_VARARGS,
     "Set the reader region"
    },
    {"set_read_plan", (PyCFunction)Reader_set_read_plan, METH_VARARGS | METH_KEYWORDS,
     "Set the read plan"
    },
    {"set_read_powers", (PyCFunction)Reader_set_read_powers, METH_VARARGS | METH_KEYWORDS,
     "Set the read power for each listed antenna and return the real setted values."
    },
    {"write", (PyCFunction)Reader_write, METH_VARARGS | METH_KEYWORDS,
     "Write the epc_target tag with the given epc_code"
    },
    {"read", (PyCFunction)Reader_read, METH_VARARGS | METH_KEYWORDS,
     "Read the tags"
    },
    {"start_reading", (PyCFunction)Reader_start_reading, METH_VARARGS | METH_KEYWORDS,
     "Start reading tags asynchronously"
    },
    {"stop_reading", (PyCFunction)Reader_stop_reading, METH_NOARGS,
     "Stop asynchronous reading"
    },
    {"get_model", (PyCFunction)Reader_get_model, METH_NOARGS,
     "Returns the model name"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef Reader_members[] = {
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

static void
TagData_dealloc(TagData* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
TagData_getepc(TagData *self, void *closure)
{
    char epcStr[128];

    TMR_bytesToHex(self->tag.epc, self->tag.epcByteCount, epcStr);
    return PyBytes_FromString(epcStr);
}

static int
TagData_setepc(TagData *self, PyObject *value, void *closure)
{
    char *buffer;
    Py_ssize_t length;

    PyBytes_AsStringAndSize(value, &buffer, &length);
    if (TMR_hexToBytes(buffer, self->tag.epc, length, NULL) != TMR_SUCCESS)
        return -1;
    self->tag.protocol = TMR_TAG_PROTOCOL_NONE;
    self->tag.epcByteCount = length;
    return 0;
}

static PyObject *
TagData_repr(TagData *self)
{
    PyObject *epc;
    PyObject *repr;

    epc = TagData_getepc(self, NULL);
    repr = PyObject_Repr(epc);
    Py_XDECREF(epc);

    return repr;
}

static PyMethodDef TagData_methods[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef TagData_members[] = {
    {NULL}  /* Sentinel */
};

static PyGetSetDef TagData_getseters[] = {
    {"epc",
     (getter)TagData_getepc, (setter)TagData_setepc,
     "Tag EPC", NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject TagDataType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mercury.TagData",         /* tp_name */
    sizeof(TagData),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)TagData_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)TagData_repr,    /* tp_repr */
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
    "TagData object",          /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    TagData_methods,           /* tp_methods */
    TagData_members,           /* tp_members */
    TagData_getseters,         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};


static void
TagReadData_dealloc(TagReadData* self)
{
    Py_XDECREF(self->epcMemData);
    Py_XDECREF(self->tidMemData);
    Py_XDECREF(self->userMemData);
    Py_XDECREF(self->reservedMemData);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMethodDef TagReadData_methods[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef TagReadData_members[] = {
    {"antenna", T_UBYTE, offsetof(TagReadData, data.antenna), READONLY,
     "Antenna where the tag was read."},
    {"read_count", T_UINT, offsetof(TagReadData, data.readCount), READONLY,
     "Number of times the tag was read."},
    {"rssi", T_INT, offsetof(TagReadData, data.rssi), READONLY,
     "Strength of the signal recieved from the tag."},
    {"epc_mem_data", T_OBJECT, offsetof(TagReadData, epcMemData), READONLY,
     "EPC bank data bytes"},
    {"tid_mem_data", T_OBJECT, offsetof(TagReadData, tidMemData), READONLY,
     "TID bank data bytes"},
    {"user_mem_data", T_OBJECT, offsetof(TagReadData, userMemData), READONLY,
     "User bank data bytes"},
    {"reserved_mem_data", T_OBJECT, offsetof(TagReadData, reservedMemData), READONLY,
     "Reserved bank data bytes"},
    {NULL}  /* Sentinel */
};

static PyGetSetDef TagReadData_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyTypeObject TagReadDataType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mercury.TagReadData",     /* tp_name */
    sizeof(TagReadData),       /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)TagReadData_dealloc, /* tp_dealloc */
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
    "TagReadData object",      /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    TagReadData_methods,       /* tp_methods */
    TagReadData_members,       /* tp_members */
    TagReadData_getseters,     /* tp_getset */
    &TagDataType,              /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};

#if PY_MAJOR_VERSION >= 3
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
#else
void
initmercury(void)
{
#endif
    PyObject* m;
    PyEval_InitThreads();

    if (PyType_Ready(&ReaderType) < 0
        || PyType_Ready(&TagDataType) < 0
        || PyType_Ready(&TagReadDataType) < 0)
#if PY_MAJOR_VERSION >= 3
        return NULL;

    m = PyModule_Create(&mercurymodule);
    if (m == NULL)
        return NULL;
#else
        return;

    m = Py_InitModule("mercury", NULL);
    if (m == NULL)
        return;
#endif
    Py_INCREF(&ReaderType);
    PyModule_AddObject(m, "Reader", (PyObject *)&ReaderType);
    Py_INCREF(&TagDataType);
    PyModule_AddObject(m, "TagData", (PyObject *)&TagDataType);
    Py_INCREF(&TagReadDataType);
    PyModule_AddObject(m, "TagReadData", (PyObject *)&TagReadDataType);
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}

/* end of file */
