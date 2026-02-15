import predicates
import pytest


def test_orient2d_sign():
    assert predicates.orient2d((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)) > 0.0
    assert predicates.orient2d_xy(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) > 0.0


def test_invalid_point_length():
    with pytest.raises(ValueError):
        predicates.orient2d((0.0, 0.0), (1.0, 0.0), (0.0, 1.0, 2.0))
    with pytest.raises(TypeError):
        predicates.orient2d_xy(0.0, 0.0, 1.0, 0.0, 0.0)


def test_all_exports_smoke():
    assert predicates.orient3d((0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)) < 0.0
    assert predicates.incircle((0.0, 0.0), (1.0, 0.0), (0.0, 1.0), (0.2, 0.2)) > 0.0
    assert predicates.insphere((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0), (-1.0, 0.0, 0.0), (0.0, 0.0, 0.0)) > 0.0
    assert predicates.orient2dfast((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)) > 0.0
    assert predicates.orient3dfast((0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)) < 0.0
    assert predicates.incirclefast((0.0, 0.0), (1.0, 0.0), (0.0, 1.0), (0.2, 0.2)) > 0.0
    assert predicates.inspherefast((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0), (-1.0, 0.0, 0.0), (0.0, 0.0, 0.0)) > 0.0


def test_orient2d_batch():
    import array

    pa = array.array("d", [0.0, 0.0, 0.0, 0.0])
    pb = array.array("d", [1.0, 0.0, 1.0, 0.0])
    pc = array.array("d", [0.0, 1.0, 0.0, -1.0])
    out = predicates.orient2d_batch(pa, pb, pc)
    assert isinstance(out, memoryview)
    assert out.format == "d"
    assert out.tolist() == pytest.approx([1.0, -1.0])
