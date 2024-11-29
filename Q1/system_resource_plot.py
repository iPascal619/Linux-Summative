#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Function to read system metrics from the usage file
static PyObject* read_system_metrics(PyObject* self, PyObject* args) {
    const char* filename;
    FILE* file;
    PyObject* metrics_list;

    // Parse filename argument
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        return NULL;
    }

    // Open file
    file = fopen(filename, "r");
    if (!file) {
        PyErr_SetString(PyExc_IOError, "Unable to open file");
        return NULL;
    }

    // Create a Python list to store metrics
    metrics_list = PyList_New(0);
    char line[256];

    // Read file line by line
    while (fgets(line, sizeof(line), file)) {
        // Create a Python string for each line
        PyObject* py_line = Py_BuildValue("s", line);
        PyList_Append(metrics_list, py_line);
        Py_DECREF(py_line);
    }

    fclose(file);
    return metrics_list;
}

// Method definition object for this extension
static PyMethodDef SystemResourceMethods[] = {
    {"read_metrics", read_system_metrics, METH_VARARGS, 
     "Read system resource metrics from a file"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// Module definition
static struct PyModuleDef systemresourcemodule = {
    PyModuleDef_HEAD_INIT,
    "system_resource_extension",   // module name
    NULL,                          // module documentation
    -1,                            // size of per-interpreter state of the module
    SystemResourceMethods
};

// Module initialization function
PyMODINIT_FUNC PyInit_system_resource_extension(void) {
    return PyModule_Create(&systemresourcemodule);
}