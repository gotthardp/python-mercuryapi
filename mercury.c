/*
 * Copyright (c) 2016-2019 Petr Gotthard <petr.gotthard@centrum.cz>
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

#define MAX_ANTENNA_COUNT 16
#define MAX_GPIO_COUNT 4
#define MAX_DATA_AREA 258
#define numberof(x) (sizeof((x))/sizeof((x)[0]))

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    TMR_Reader reader;
    TMR_TagOp tagop;
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

    if ((ant_count = PyList_Size(list)) > MAX_ANTENNA_COUNT)
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

        if ((ret = TMR_TagOp_init_GEN2_ReadData(&self->tagop, op, 0, 0)) != TMR_SUCCESS)
            goto fail;

        if ((ret = TMR_RP_set_tagop(&plan, &self->tagop)) != TMR_SUCCESS)
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
Reader_write(Reader *self, PyObject *args, PyObject *kwds)
{
    char* epc_data;
    char* epc_target = NULL;
    TMR_Status ret;
    TMR_TagData data;
    TMR_TagData target;
    TMR_TagFilter tag_filter, *filter;

    // Read call arguments.
    static char *kwlist[] = {"epc_code", "epc_target", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &epc_data, &epc_target))
        return NULL;

    /* build data tag to be writen */
    data.epcByteCount = strlen(epc_data) * sizeof(char) / 2;
    TMR_hexToBytes(epc_data, data.epc, data.epcByteCount, NULL);

    /* build target tag to search */
    if(epc_target != NULL)
    {
        target.epcByteCount = strlen(epc_target) * sizeof(char) / 2;
        TMR_hexToBytes(epc_target, target.epc, target.epcByteCount, NULL);

        filter = &tag_filter;
        TMR_TF_init_tag(filter, &target);
    }
    else
        filter = NULL;

    // Write data tag on target tag.
    ret = TMR_writeTag(&self->reader, filter, &data);
    // In case of not target tag found.
    if (ret == TMR_ERROR_NO_TAGS_FOUND)
        Py_RETURN_FALSE;
    else if(ret != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

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

    Py_BEGIN_ALLOW_THREADS
    ret = TMR_read(&self->reader, timeout, NULL);
    Py_END_ALLOW_THREADS

    /* In case of TAG ID Buffer Full, extract the tags present in buffer. */
    if (ret != TMR_SUCCESS && ret != TMR_ERROR_TAG_ID_BUFFER_FULL)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* create empty list */
    list = PyList_New(0);
    if(list == NULL)
        return PyErr_NoMemory();

    while (TMR_hasMoreTags(&self->reader) == TMR_SUCCESS)
    {
        TagReadData *tag;
        uint8_t dataBuf1[MAX_DATA_AREA];
        uint8_t dataBuf2[MAX_DATA_AREA];
        uint8_t dataBuf3[MAX_DATA_AREA];
        uint8_t dataBuf4[MAX_DATA_AREA];

        tag = PyObject_New(TagReadData, &TagReadDataType);
        if(tag == NULL)
        {
            Py_XDECREF(list);
            return PyErr_NoMemory();
        }
        TMR_TRD_init(&tag->data);
        tag->epcMemData = NULL;
        tag->tidMemData = NULL;
        tag->userMemData = NULL;
        tag->reservedMemData = NULL;

        TMR_TRD_MEMBANK_init_data(&tag->data.epcMemData, MAX_DATA_AREA, dataBuf1);
        TMR_TRD_MEMBANK_init_data(&tag->data.tidMemData, MAX_DATA_AREA, dataBuf2);
        TMR_TRD_MEMBANK_init_data(&tag->data.userMemData, MAX_DATA_AREA, dataBuf3);
        TMR_TRD_MEMBANK_init_data(&tag->data.reservedMemData, MAX_DATA_AREA, dataBuf4);

        if ((ret = TMR_getNextTag(&self->reader, &tag->data)) != TMR_SUCCESS)
        {
            PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
            Py_XDECREF(tag);
            Py_XDECREF(list);
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
        tag->epcMemData = NULL;
        tag->tidMemData = NULL;
        tag->userMemData = NULL;
        tag->reservedMemData = NULL;

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
Reader_read_tag_mem(Reader *self, PyObject *args, PyObject *kwds)
{
    TMR_Status ret;
    char* epc_target = NULL;
    uint32_t bank, address, count;

    TMR_TagData target;
    TMR_TagFilter tag_filter, *filter;
    uint8_t *buf;
    PyObject *result;

    static char *kwlist[] = {"bank", "address", "count", "epc_target", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "III|s", kwlist, &bank, &address, &count, &epc_target))
        return NULL;

    if(epc_target != NULL)
    {
        /* build target tag to search */
        target.epcByteCount = strlen(epc_target) / 2;
        TMR_hexToBytes(epc_target, target.epc, target.epcByteCount, NULL);

        filter = &tag_filter;
        TMR_TF_init_tag(filter, &target);
    }
    else
        filter = NULL;

    buf = malloc(count);
    ret = TMR_readTagMemBytes(&self->reader, filter, bank, address, (uint16_t)count, buf);
    if (ret == TMR_ERROR_NO_TAGS_FOUND)
        Py_RETURN_NONE;
    else if (ret != TMR_SUCCESS)
    {
        free(buf);
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    result = PyByteArray_FromStringAndSize((const char *)buf, count);
    free(buf);

    return result;
}

static PyObject *
Reader_write_tag_mem(Reader *self, PyObject *args, PyObject *kwds)
{
    TMR_Status ret;
    char* epc_target = NULL;
    uint32_t bank, address;
    PyObject *data;

    TMR_TagData target;
    TMR_TagFilter tag_filter, *filter;

    static char *kwlist[] = {"bank", "address", "data", "epc_target", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "IIO!|s", kwlist, &bank, &address, &PyByteArray_Type, &data, &epc_target))
        return NULL;

    if(epc_target != NULL)
    {
        /* build target tag to search */
        target.epcByteCount = strlen(epc_target) / 2;
        TMR_hexToBytes(epc_target, target.epc, target.epcByteCount, NULL);

        filter = &tag_filter;
        TMR_TF_init_tag(filter, &target);
    }
    else
        filter = NULL;

    ret = TMR_writeTagMemBytes(&self->reader, filter, bank, address, PyByteArray_Size(data), (uint8_t *)PyByteArray_AsString(data));
    free(filter);
    if (ret == TMR_ERROR_NO_TAGS_FOUND)
        Py_RETURN_FALSE;
    else if (ret != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }
    else
        Py_RETURN_TRUE;
}

static PyObject *
Reader_gpi_get(Reader *self, PyObject *args)
{
    TMR_GpioPin *gpio;
    uint8_t i, pin, count;
    TMR_Status ret;

    if (!PyArg_ParseTuple(args, "B", &pin))
        return NULL;

    count = pin+1;
    gpio = (TMR_GpioPin*)malloc(count*sizeof(TMR_GpioPin));

    if ((ret = TMR_gpiGet(&self->reader, &count, gpio)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        free(gpio);
        return NULL;
    }

    for(i = 0; i < count; i++)
    {
        if(gpio[i].id == pin)
        {
            PyObject *res = PyBool_FromLong(gpio[i].high);
            free(gpio);
            return res;
        }
    }

    free(gpio);
    Py_RETURN_NONE;
}

static PyObject *
Reader_gpo_set(Reader *self, PyObject *args)
{
    uint8_t pin;
    PyObject *value;
    int intval;
    TMR_GpioPin gpio;
    TMR_Status ret;

    /* to preserve Python 2.x compatibility we cannot use "p"
       also, using "i" is foolish as per https://bugs.python.org/issue14705 */
    if (!PyArg_ParseTuple(args, "BO", &pin, &value))
        return NULL;

    if ((intval = PyObject_IsTrue(value)) < 0)
    {
        PyErr_SetString(PyExc_TypeError, "Boolean value expected");
        return NULL;
    }

    gpio.id = pin;
    gpio.high = !!intval; /* convert to bool */
    gpio.output = true;

    if ((ret = TMR_gpoSet(&self->reader, 1, &gpio)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    Py_RETURN_NONE;
}

/* Functions to get/set the reader parameters */

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
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyUnicode_FromString(model.value);
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
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* Do something interesting here. */
    Py_RETURN_NONE;
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
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    /* create empty list */
    list = PyList_New(0);
    if(list == NULL)
        return PyErr_NoMemory();

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
get_uint32(TMR_Reader *reader, int param)
{
    uint32_t value;
    TMR_Status ret;

    if ((ret = TMR_paramGet(reader, param, &value)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        return NULL;
    }

    return PyLong_FromLong(value);
}

static PyObject *
set_uint32(TMR_Reader *reader, PyObject *args, int param)
{
    uint32_t value;
    TMR_Status ret;

    if (!PyArg_ParseTuple(args, "I", &value))
        return NULL;

    if ((ret = TMR_paramSet(reader, param, &value)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        return NULL;
    }

    return PyLong_FromLong(value);
}

static PyObject *
get_uint8List(TMR_Reader *reader, int param, size_t max)
{
    int i;
    TMR_Status ret;
    PyObject *result;
    TMR_uint8List values;

    values.list = (uint8_t*)malloc(max);
    values.len = 0;
    values.max = max;

    if ((ret = TMR_paramGet(reader, param, &values)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        free(values.list);
        return NULL;
    }

    result = PyList_New(values.len);
    if(result == NULL)
        return PyErr_NoMemory();

    for (i = 0; i < values.len; i++)
        PyList_SetItem(result, i, PyLong_FromUnsignedLong(values.list[i]));

    free(values.list);
    return result;
}

static PyObject *
get_uint32List(TMR_Reader *reader, int param, size_t max)
{
    int i;
    TMR_Status ret;
    PyObject *result;
    TMR_uint32List values;

    values.list = (uint32_t*)malloc(max*sizeof(uint32_t));
    values.len = 0;
    values.max = max;

    if ((ret = TMR_paramGet(reader, param, &values)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        free(values.list);
        return NULL;
    }

    result = PyList_New(values.len);
    if(result == NULL)
        return PyErr_NoMemory();

    for (i = 0; i < values.len; i++)
        PyList_SetItem(result, i, PyLong_FromUnsignedLong(values.list[i]));

    free(values.list);
    return result;
}

static PyObject *
set_uint8List(TMR_Reader *reader, PyObject *args, int param)
{
    int i;
    PyObject *input;
    TMR_Status ret;

    size_t count;
    TMR_uint8List values;

    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input))
        return NULL;

    count = PyList_Size(input);
    values.list = (uint8_t*)malloc(count);
    values.len = count;
    values.max = count;

    for (i = 0; i < values.len; i++)
    {
        values.list[i] = PyLong_AsLong(PyList_GetItem(input, i));
    }

    if ((ret = TMR_paramSet(reader, param, &values)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        free(values.list);
        return NULL;
    }

    free(values.list);
    Py_RETURN_NONE; 
}

static PyObject *
set_uint32List(TMR_Reader *reader, PyObject *args, int param)
{
    int i;
    PyObject *input;
    TMR_Status ret;

    size_t count;
    TMR_uint32List values;

    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input))
        return NULL;

    count = PyList_Size(input);
    values.list = (uint32_t*)malloc(count*sizeof(uint32_t));
    values.len = count;
    values.max = count;

    for (i = 0; i < values.len; i++)
    {
        values.list[i] = PyLong_AsLong(PyList_GetItem(input, i));
    }

    if ((ret = TMR_paramSet(reader, param, &values)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        free(values.list);
        return NULL;
    }

    free(values.list);
    Py_RETURN_NONE; 
}

static PyObject *
Reader_get_hop_table(Reader *self)
{
    return get_uint32List(&self->reader, TMR_PARAM_REGION_HOPTABLE, 64);
}

static PyObject *
Reader_set_hop_table(Reader *self, PyObject *args)
{
    return set_uint32List(&self->reader, args, TMR_PARAM_REGION_HOPTABLE);
}

static PyObject *
Reader_get_hop_time(Reader *self)
{
    return get_uint32(&self->reader, TMR_PARAM_REGION_HOPTIME);
}

static PyObject *
Reader_set_hop_time(Reader *self, PyObject *args)
{
    return set_uint32(&self->reader, args, TMR_PARAM_REGION_HOPTIME);
}

static PyObject *
Reader_get_antennas(Reader *self)
{
    return get_uint8List(&self->reader, TMR_PARAM_ANTENNA_PORTLIST, MAX_ANTENNA_COUNT);
}

static PyObject *
Reader_get_connected_ports(Reader *self)
{
    return get_uint8List(&self->reader, TMR_PARAM_ANTENNA_CONNECTEDPORTLIST, MAX_ANTENNA_COUNT);
}

static PyObject *
Reader_get_antenna_portswitchgpos(Reader *self)
{
    return get_uint8List(&self->reader, TMR_PARAM_ANTENNA_PORTSWITCHGPOS, MAX_GPIO_COUNT);
}

static PyObject *
Reader_set_antenna_portswitchgpos(Reader *self, PyObject *args)
{
    return set_uint8List(&self->reader, args, TMR_PARAM_ANTENNA_PORTSWITCHGPOS);
}

static PyObject *
Reader_get_power_range(Reader *self)
{
    int16_t lim_power;
    TMR_Status ret;
    PyObject *powers;
    powers = PyTuple_New(2);

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_POWERMIN, &lim_power)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }
    PyTuple_SetItem(powers, 0, PyLong_FromLong(lim_power));

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_POWERMAX, &lim_power)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }
    PyTuple_SetItem(powers, 1, PyLong_FromLong(lim_power));
    return powers;
}

static PyObject *
get_PortValueList(TMR_Reader *reader, int param)
{
    int i;
    TMR_Status ret;
    PyObject *result = PyList_New(0);

    TMR_PortValueList value_list;
    TMR_PortValue values[MAX_ANTENNA_COUNT];

    value_list.list = values;
    value_list.max = MAX_ANTENNA_COUNT;
    value_list.len = 0;

    if ((ret = TMR_paramGet(reader, param, &value_list)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
        return NULL;
    }

    for (i = 0; i < value_list.len; i++)
    {
        PyObject *item;
        item = PyTuple_New(2);
        PyTuple_SetItem(item, 0, PyLong_FromUnsignedLong(value_list.list[i].port));
        PyTuple_SetItem(item, 1, PyLong_FromLong(value_list.list[i].value));
        PyList_Append(result, item);
    }

    return result;
}

static PyObject *
set_PortValueList(TMR_Reader *reader, PyObject *args, int param)
{
    TMR_Status ret;
    PyObject *input;
    uint8_t count;

    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input))
        return NULL;
    if ((count = PyList_Size(input)) > MAX_ANTENNA_COUNT)
    {
        PyErr_SetString(PyExc_TypeError, "Too many powers");
        return NULL;
    }

    int i;
    TMR_PortValueList value_list;
    TMR_PortValue values[MAX_ANTENNA_COUNT];

    value_list.list = values;
    value_list.max = MAX_ANTENNA_COUNT;
    value_list.len = count;

    for (i = 0; i < count; i++)
    {
        int antenna;
        int power;

        PyObject *item = PyList_GetItem(input, i);
        if(!PyTuple_Check(item))
        {
            PyErr_SetString(PyExc_TypeError, "expecting list of tuples");
            return NULL;
        }

        antenna = PyLong_AsLong(PyTuple_GetItem(item, 0));
        power = PyLong_AsLong(PyTuple_GetItem(item, 1));

        if(antenna < 0 || antenna > MAX_ANTENNA_COUNT)
        {
            PyErr_SetString(PyExc_TypeError, "wrong antenna index");
            return NULL;
        }

        value_list.list[i].port = antenna;
        value_list.list[i].value = power;
    }

    if (count > 0)
    {
        if ((ret = TMR_paramSet(reader, param, &value_list)) != TMR_SUCCESS)
            goto fail;
    }

    Py_DECREF(input);
    return get_PortValueList(reader, param);
fail:
    PyErr_SetString(PyExc_RuntimeError, TMR_strerr(reader, ret));
    return NULL;
}

static PyObject *
Reader_get_read_powers(Reader *self)
{
    return get_PortValueList(&self->reader, TMR_PARAM_RADIO_PORTREADPOWERLIST);
}

static PyObject *
Reader_set_read_powers(Reader *self, PyObject *args)
{
    return set_PortValueList(&self->reader, args, TMR_PARAM_RADIO_PORTREADPOWERLIST);
}

static PyObject *
Reader_get_write_powers(Reader *self)
{
    return get_PortValueList(&self->reader, TMR_PARAM_RADIO_PORTWRITEPOWERLIST);
}

static PyObject *
Reader_set_write_powers(Reader *self, PyObject *args, PyObject *kwds)
{
    return set_PortValueList(&self->reader, args, TMR_PARAM_RADIO_PORTWRITEPOWERLIST);
}

static PyObject *
Reader_get_gpio_inputs(Reader *self)
{
    return get_uint8List(&self->reader, TMR_PARAM_GPIO_INPUTLIST, 16);
}

static PyObject *
Reader_set_gpio_inputs(Reader *self, PyObject *args)
{
    return set_uint8List(&self->reader, args, TMR_PARAM_GPIO_INPUTLIST);
}

static PyObject *
Reader_get_gpio_outputs(Reader *self)
{
    return get_uint8List(&self->reader, TMR_PARAM_GPIO_OUTPUTLIST, 16);
}

static PyObject *
Reader_set_gpio_outputs(Reader *self, PyObject *args)
{
    return set_uint8List(&self->reader, args, TMR_PARAM_GPIO_OUTPUTLIST);
}

static PyObject *
Reader_get_gen2_q(Reader* self)
{
    TMR_Status ret;
    TMR_SR_GEN2_Q model;
    PyObject *q_value;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_Q, &model)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    q_value = PyTuple_New(2);
    PyTuple_SetItem(q_value, 0, PyLong_FromLong(model.type));
    PyTuple_SetItem(q_value, 1, PyLong_FromLong(model.u.staticQ.initialQ));

    return q_value;
}

static PyObject *
Reader_set_gen2_q(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_SR_GEN2_Q model;
    PyObject *q_value;

    if (!PyArg_ParseTuple(args, "ii", &model.type, &model.u.staticQ.initialQ))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_Q, &model)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    q_value = PyTuple_New(2);
    PyTuple_SetItem(q_value, 0, PyLong_FromLong(model.type));
    PyTuple_SetItem(q_value, 1, PyLong_FromLong(model.u.staticQ.initialQ));

    return q_value;
}

static PyObject *
Reader_get_gen2_tagencoding(Reader* self)
{
    TMR_Status ret;
    TMR_GEN2_TagEncoding tagencoding_val;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_TAGENCODING, &tagencoding_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(tagencoding_val);
}

static PyObject *
Reader_set_gen2_tagencoding(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_GEN2_TagEncoding tagencoding_val;

    if (!PyArg_ParseTuple(args, "i", &tagencoding_val))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_TAGENCODING, &tagencoding_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(tagencoding_val);
}

static PyObject *
Reader_get_gen2_session(Reader* self)
{
    TMR_Status ret;
    TMR_GEN2_Session session_val;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_SESSION, &session_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(session_val);
}

static PyObject *
Reader_set_gen2_session(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_GEN2_Session session_val;

    if (!PyArg_ParseTuple(args, "i", &session_val))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_SESSION, &session_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(session_val);
}

static PyObject *
Reader_get_gen2_target(Reader* self)
{
    TMR_Status ret;
    TMR_GEN2_Target target_val;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_TARGET, &target_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(target_val);
}

static PyObject *
Reader_set_gen2_target(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_GEN2_Target target_val;

    if (!PyArg_ParseTuple(args, "i", &target_val))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_TARGET, &target_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(target_val);
}

static PyObject *
Reader_get_gen2_blf(Reader* self)
{
    TMR_Status ret;
    TMR_GEN2_LinkFrequency blf_val;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_BLF, &blf_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(blf_val);
}

static PyObject *
Reader_set_gen2_blf(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_GEN2_LinkFrequency blf_val;

    if (!PyArg_ParseTuple(args, "i", &blf_val))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_BLF, &blf_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(blf_val);
}

static PyObject *
Reader_get_gen2_tari(Reader* self)
{
    TMR_Status ret;
    TMR_GEN2_Tari tari_val;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_GEN2_TARI, &tari_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(tari_val);
}

static PyObject *
Reader_set_gen2_tari(Reader* self, PyObject *args)
{
    TMR_Status ret;
    TMR_GEN2_Tari tari_val;

    if (!PyArg_ParseTuple(args, "i", &tari_val))
        return NULL;

    if ((ret = TMR_paramSet(&self->reader, TMR_PARAM_GEN2_TARI, &tari_val)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromLong(tari_val);
}

static PyObject *
Reader_get_temperature(Reader *self)
{
    TMR_Status ret;
    uint8_t temp;

    if ((ret = TMR_paramGet(&self->reader, TMR_PARAM_RADIO_TEMPERATURE, &temp)) != TMR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, TMR_strerr(&self->reader, ret));
        return NULL;
    }

    return PyLong_FromUnsignedLong(temp);
}

static PyMethodDef Reader_methods[] = {
    {"set_read_plan", (PyCFunction)Reader_set_read_plan, METH_VARARGS | METH_KEYWORDS,
     "Set the read plan"
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
    {"read_tag_mem", (PyCFunction)Reader_read_tag_mem, METH_VARARGS | METH_KEYWORDS,
     "Read bytes from the memory bank of a tag"
    },
    {"write_tag_mem", (PyCFunction)Reader_write_tag_mem, METH_VARARGS | METH_KEYWORDS,
     "Write bytes to the memory bank of a tag"
    },
    {"gpi_get", (PyCFunction)Reader_gpi_get, METH_VARARGS,
     "Gets GPIO pin value"
    },
    {"gpo_set", (PyCFunction)Reader_gpo_set, METH_VARARGS,
     "Sets GPIO pin value"
    },
    /* Reader parameters */
    {"get_model", (PyCFunction)Reader_get_model, METH_NOARGS,
     "Returns the model name"
    },
    {"set_region", (PyCFunction)Reader_set_region, METH_VARARGS,
     "Set the reader region"
    },
    {"get_supported_regions", (PyCFunction)Reader_get_supported_regions, METH_NOARGS,
     "Returns a list of regions supported by the reader"
    },
    {"get_hop_table", (PyCFunction)Reader_get_hop_table, METH_NOARGS,
     "Gets the frequencies used by the reader, in kHz"
    },
    {"set_hop_table", (PyCFunction)Reader_set_hop_table, METH_VARARGS,
     "Sets the frequencies used by the reader, in kHz"
    },
    {"get_hop_time", (PyCFunction)Reader_get_hop_time, METH_NOARGS,
     "Gets the frequency hop time, in milliseconds"
    },
    {"set_hop_time", (PyCFunction)Reader_set_hop_time, METH_VARARGS,
     "Sets the frequency hop time, in milliseconds"
    },
    {"get_antennas", (PyCFunction)Reader_get_antennas, METH_NOARGS,
     "Lists available antennas."
    },
    {"get_connected_ports", (PyCFunction)Reader_get_connected_ports, METH_NOARGS,
     "Lists connected antennas."
    },
    {"get_antenna_portswitchgpos", (PyCFunction)Reader_get_antenna_portswitchgpos, METH_NOARGS,
     "Lists the GPO pins used for antenna port switching."
    },
    {"set_antenna_portswitchgpos", (PyCFunction)Reader_set_antenna_portswitchgpos, METH_VARARGS,
     "Set the GPO pins used for antenna port switching."
    },
    {"get_power_range", (PyCFunction)Reader_get_power_range, METH_NOARGS,
     "Lists supported radio power range."
    },
    {"get_read_powers", (PyCFunction)Reader_get_read_powers, METH_NOARGS,
     "Lists configured read powers for each antenna."
    },
    {"get_write_powers", (PyCFunction)Reader_get_write_powers, METH_NOARGS,
     "Lists configured write powers for each antenna."
    },
    {"set_read_powers", (PyCFunction)Reader_set_read_powers, METH_VARARGS,
     "Set the read power for each listed antenna and return the real setted values."
    },
    {"set_write_powers", (PyCFunction)Reader_set_write_powers, METH_VARARGS,
     "Set the write power for each listed antenna and return the real setted values."
    },
    {"get_gpio_inputs", (PyCFunction)Reader_get_gpio_inputs, METH_NOARGS,
     "Get numbers of the GPIO pins available as input pins on the device."
    },
    {"set_gpio_inputs", (PyCFunction)Reader_set_gpio_inputs, METH_VARARGS,
     "Set numbers of the GPIO pins available as input pins on the device."
    },
    {"get_gpio_outputs", (PyCFunction)Reader_get_gpio_outputs, METH_NOARGS,
     "Get numbers of the GPIO pins available as output pins on the device."
    },
    {"set_gpio_outputs", (PyCFunction)Reader_set_gpio_outputs, METH_VARARGS,
     "Set numbers of the GPIO pins available as output pins on the device."
    },
    {"get_gen2_q", (PyCFunction)Reader_get_gen2_q, METH_NOARGS,
     "Returns the current Gen2 Q setting"
    },
    {"set_gen2_q", (PyCFunction)Reader_set_gen2_q, METH_VARARGS,
     "Sets the Gen2 Q"
    },
    {"get_gen2_tagencoding", (PyCFunction)Reader_get_gen2_tagencoding, METH_NOARGS,
     "Returns the current Gen2 TagEncoding setting"
    },
    {"set_gen2_tagencoding", (PyCFunction)Reader_set_gen2_tagencoding, METH_VARARGS,
     "Sets the Gen2 TagEncoding"
    },
    {"get_gen2_session", (PyCFunction)Reader_get_gen2_session, METH_NOARGS,
     "Returns the current Gen2 Session setting"
    },
    {"set_gen2_session", (PyCFunction)Reader_set_gen2_session, METH_VARARGS,
     "Sets the Gen2 Session"
    },
    {"get_gen2_target", (PyCFunction)Reader_get_gen2_target, METH_NOARGS,
     "Returns the current Gen2 Target setting"
    },
    {"set_gen2_target", (PyCFunction)Reader_set_gen2_target, METH_VARARGS,
     "Sets the Gen2 Target"
    },
    {"get_gen2_blf", (PyCFunction)Reader_get_gen2_blf, METH_NOARGS,
     "Returns the current Gen2 BLF setting"
    },
    {"set_gen2_blf", (PyCFunction)Reader_set_gen2_blf, METH_VARARGS,
     "Sets the Gen2 BLF"
    },
    {"get_gen2_tari", (PyCFunction)Reader_get_gen2_tari, METH_NOARGS,
     "Returns the current Gen2 Tari setting"
    },
    {"set_gen2_tari", (PyCFunction)Reader_set_gen2_tari, METH_VARARGS,
     "Sets the Gen2 Tari"
    },
    {"get_temperature", (PyCFunction)Reader_get_temperature, METH_NOARGS,
     "Returns the chip temperature"
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
    {"phase", T_USHORT, offsetof(TagReadData, data.phase), READONLY,
     "Tag response phase."},
    {"antenna", T_UBYTE, offsetof(TagReadData, data.antenna), READONLY,
     "Antenna where the tag was read."},
    {"read_count", T_UINT, offsetof(TagReadData, data.readCount), READONLY,
     "Number of times the tag was read."},
    {"rssi", T_INT, offsetof(TagReadData, data.rssi), READONLY,
     "Strength of the signal recieved from the tag."},
    {"frequency", T_UINT, offsetof(TagReadData, data.frequency), READONLY,
     "RF carrier frequency the tag was read with."},
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
