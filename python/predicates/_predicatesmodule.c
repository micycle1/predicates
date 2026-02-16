#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>

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

static int as_double(PyObject *obj, double *out)
{
  if (PyFloat_CheckExact(obj)) {
    *out = PyFloat_AS_DOUBLE(obj);
    return 1;
  }
  *out = PyFloat_AsDouble(obj);
  return !PyErr_Occurred();
}

static int parse_2d_points_buffer(PyObject *obj, Py_buffer *view, const double **data, Py_ssize_t *n)
{
  if (PyObject_GetBuffer(obj, view, PyBUF_FORMAT | PyBUF_ND | PyBUF_STRIDES | PyBUF_C_CONTIGUOUS) != 0) {
    return 0;
  }

  if (view->itemsize != (Py_ssize_t) sizeof(double) ||
      view->format == NULL ||
      strcmp(view->format, "d") != 0) {
    PyErr_SetString(PyExc_TypeError, "buffer must have float64 ('d') format");
    PyBuffer_Release(view);
    return 0;
  }

  if (view->ndim == 2) {
    if (view->shape[1] != 2) {
      PyErr_SetString(PyExc_ValueError, "buffer must have shape (N, 2)");
      PyBuffer_Release(view);
      return 0;
    }
    *n = view->shape[0];
  } else if (view->ndim == 1) {
    if (view->shape[0] % 2 != 0) {
      PyErr_SetString(PyExc_ValueError, "flat buffer length must be divisible by 2");
      PyBuffer_Release(view);
      return 0;
    }
    *n = view->shape[0] / 2;
  } else {
    PyErr_SetString(PyExc_ValueError, "buffer must be 1D (2*N) or 2D (N,2)");
    PyBuffer_Release(view);
    return 0;
  }

  *data = (const double *) view->buf;
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

static PyObject *py_orient2d_xy(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
  double ax, ay, bx, by, cx, cy;

  if (nargs != 6) {
    return PyErr_Format(PyExc_TypeError,
                        "orient2d_xy() takes 6 positional arguments (%zd given)",
                        nargs);
  }

  if (!as_double(args[0], &ax) || !as_double(args[1], &ay) ||
      !as_double(args[2], &bx) || !as_double(args[3], &by) ||
      !as_double(args[4], &cx) || !as_double(args[5], &cy)) {
    return NULL;
  }

  double a[2] = {ax, ay};
  double b[2] = {bx, by};
  double c[2] = {cx, cy};
  return PyFloat_FromDouble(orient2d(a, b, c));
}

static PyObject *py_orient2d_batch(PyObject *self, PyObject *args)
{
  PyObject *pa, *pb, *pc;
  Py_buffer va, vb, vc;
  const double *a, *b, *c;
  Py_ssize_t na, nb, nc;
  Py_ssize_t i;
  PyObject *out = NULL, *view = NULL, *casted = NULL;
  double *out_data;

  va.obj = NULL; vb.obj = NULL; vc.obj = NULL;

  if (!PyArg_ParseTuple(args, "OOO", &pa, &pb, &pc)) {
    return NULL;
  }

  if (!parse_2d_points_buffer(pa, &va, &a, &na) ||
      !parse_2d_points_buffer(pb, &vb, &b, &nb) ||
      !parse_2d_points_buffer(pc, &vc, &c, &nc)) {
    goto cleanup;
  }

  if (na != nb || na != nc) {
    PyErr_SetString(PyExc_ValueError, "all inputs must contain the same number of points");
    goto cleanup;
  }

  out = PyByteArray_FromStringAndSize(NULL, na * (Py_ssize_t) sizeof(double));
  if (out == NULL) {
    goto cleanup;
  }

  out_data = (double *) PyByteArray_AS_STRING(out);
  for (i = 0; i < na; i++) {
    out_data[i] = orient2d(a + 2 * i, b + 2 * i, c + 2 * i);
  }

  view = PyMemoryView_FromObject(out);
  if (view == NULL) {
    goto cleanup;
  }
  casted = PyObject_CallMethod(view, "cast", "s(n)", "d", na);

cleanup:
  if (va.obj != NULL) {
    PyBuffer_Release(&va);
  }
  if (vb.obj != NULL) {
    PyBuffer_Release(&vb);
  }
  if (vc.obj != NULL) {
    PyBuffer_Release(&vc);
  }
  Py_XDECREF(view);
  Py_XDECREF(out);
  return casted;
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
  {"orient2d_xy", (PyCFunction) py_orient2d_xy, METH_FASTCALL, "Fast scalar orientation test in 2D using flat coordinates."},
  {"orient2d_batch", py_orient2d_batch, METH_VARARGS, "Batched orientation test in 2D for float64 buffers shaped (N,2) or flat (2*N)."},
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
