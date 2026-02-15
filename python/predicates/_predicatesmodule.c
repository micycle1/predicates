#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "predicates.h"

static int parse_point(PyObject *obj, double *out, Py_ssize_t expected_len)
{
  PyObject *seq = PySequence_Fast(obj, "point must be a sequence");
  if (seq == NULL) {
    return 0;
  }

  Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
  if (len != expected_len) {
    Py_DECREF(seq);
    PyErr_Format(PyExc_ValueError,
                 "point must have length %zd, got %zd",
                 expected_len,
                 len);
    return 0;
  }

  for (Py_ssize_t i = 0; i < expected_len; i++) {
    PyObject *item = PySequence_Fast_GET_ITEM(seq, i);
    double value = PyFloat_AsDouble(item);
    if (PyErr_Occurred()) {
      Py_DECREF(seq);
      return 0;
    }
    out[i] = value;
  }

  Py_DECREF(seq);
  return 1;
}

static PyObject *py_orient2d(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc;
  double a[2], b[2], c[2];

  if (!PyArg_ParseTuple(args, "OOO", &pa, &pb, &pc)) {
    return NULL;
  }
  if (!parse_point(pa, a, 2) || !parse_point(pb, b, 2) || !parse_point(pc, c, 2)) {
    return NULL;
  }
  return PyFloat_FromDouble(orient2d(a, b, c));
}

static PyObject *py_orient3d(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd;
  double a[3], b[3], c[3], d[3];

  if (!PyArg_ParseTuple(args, "OOOO", &pa, &pb, &pc, &pd)) {
    return NULL;
  }
  if (!parse_point(pa, a, 3) ||
      !parse_point(pb, b, 3) ||
      !parse_point(pc, c, 3) ||
      !parse_point(pd, d, 3)) {
    return NULL;
  }
  return PyFloat_FromDouble(orient3d(a, b, c, d));
}

static PyObject *py_incircle(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd;
  double a[2], b[2], c[2], d[2];

  if (!PyArg_ParseTuple(args, "OOOO", &pa, &pb, &pc, &pd)) {
    return NULL;
  }
  if (!parse_point(pa, a, 2) ||
      !parse_point(pb, b, 2) ||
      !parse_point(pc, c, 2) ||
      !parse_point(pd, d, 2)) {
    return NULL;
  }
  return PyFloat_FromDouble(incircle(a, b, c, d));
}

static PyObject *py_insphere(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd, *pe;
  double a[3], b[3], c[3], d[3], e[3];

  if (!PyArg_ParseTuple(args, "OOOOO", &pa, &pb, &pc, &pd, &pe)) {
    return NULL;
  }
  if (!parse_point(pa, a, 3) ||
      !parse_point(pb, b, 3) ||
      !parse_point(pc, c, 3) ||
      !parse_point(pd, d, 3) ||
      !parse_point(pe, e, 3)) {
    return NULL;
  }
  return PyFloat_FromDouble(insphere(a, b, c, d, e));
}

static PyObject *py_orient2dfast(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc;
  double a[2], b[2], c[2];

  if (!PyArg_ParseTuple(args, "OOO", &pa, &pb, &pc)) {
    return NULL;
  }
  if (!parse_point(pa, a, 2) || !parse_point(pb, b, 2) || !parse_point(pc, c, 2)) {
    return NULL;
  }
  return PyFloat_FromDouble(orient2dfast(a, b, c));
}

static PyObject *py_orient3dfast(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd;
  double a[3], b[3], c[3], d[3];

  if (!PyArg_ParseTuple(args, "OOOO", &pa, &pb, &pc, &pd)) {
    return NULL;
  }
  if (!parse_point(pa, a, 3) ||
      !parse_point(pb, b, 3) ||
      !parse_point(pc, c, 3) ||
      !parse_point(pd, d, 3)) {
    return NULL;
  }
  return PyFloat_FromDouble(orient3dfast(a, b, c, d));
}

static PyObject *py_incirclefast(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd;
  double a[2], b[2], c[2], d[2];

  if (!PyArg_ParseTuple(args, "OOOO", &pa, &pb, &pc, &pd)) {
    return NULL;
  }
  if (!parse_point(pa, a, 2) ||
      !parse_point(pb, b, 2) ||
      !parse_point(pc, c, 2) ||
      !parse_point(pd, d, 2)) {
    return NULL;
  }
  return PyFloat_FromDouble(incirclefast(a, b, c, d));
}

static PyObject *py_inspherefast(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc, *pd, *pe;
  double a[3], b[3], c[3], d[3], e[3];

  if (!PyArg_ParseTuple(args, "OOOOO", &pa, &pb, &pc, &pd, &pe)) {
    return NULL;
  }
  if (!parse_point(pa, a, 3) ||
      !parse_point(pb, b, 3) ||
      !parse_point(pc, c, 3) ||
      !parse_point(pd, d, 3) ||
      !parse_point(pe, e, 3)) {
    return NULL;
  }
  return PyFloat_FromDouble(inspherefast(a, b, c, d, e));
}

static PyMethodDef predicates_methods[] = {
  {"orient2d", py_orient2d, METH_VARARGS, "Robust orientation test in 2D."},
  {"orient3d", py_orient3d, METH_VARARGS, "Robust orientation test in 3D."},
  {"incircle", py_incircle, METH_VARARGS, "Robust in-circle test in 2D."},
  {"insphere", py_insphere, METH_VARARGS, "Robust in-sphere test in 3D."},
  {"orient2dfast", py_orient2dfast, METH_VARARGS, "Fast orientation test in 2D."},
  {"orient3dfast", py_orient3dfast, METH_VARARGS, "Fast orientation test in 3D."},
  {"incirclefast", py_incirclefast, METH_VARARGS, "Fast in-circle test in 2D."},
  {"inspherefast", py_inspherefast, METH_VARARGS, "Fast in-sphere test in 3D."},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef predicates_module = {
  PyModuleDef_HEAD_INIT,
  "_predicates",
  "Python bindings for robust geometric predicates.",
  -1,
  predicates_methods
};

PyMODINIT_FUNC PyInit__predicates(void)
{
  return PyModule_Create(&predicates_module);
}
