import predicates


def test_orient2d_sign():
    assert predicates.orient2d((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)) > 0.0


def test_invalid_point_length():
    try:
        predicates.orient2d((0.0, 0.0), (1.0, 0.0), (0.0, 1.0, 2.0))
    except ValueError:
        return
    assert False, "expected ValueError"
