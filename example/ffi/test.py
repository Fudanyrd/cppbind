import _ffi as cxxstd
rc = cxxstd.debug_refcnt

def test_cint():
    a = cxxstd.cint()
    assert int(a) == 0
    init_rc = rc(a)

    # test addition
    a += 1
    assert int(a) == 1
    a += a
    assert int(a) == 2

    # test correct reference counting
    assert rc(a) == init_rc

    # do not divide by zero (Floating point exception on X86)
    # a /= 0


def test_map():
    a = cxxstd.map()
    a[0] = 1
    assert a[0] == 1

    # check value overwriting.
    magic = 0x12345678
    a[magic] = None
    assert a[magic] is None
    a[magic] = magic
    assert magic == a[magic]

    # check size
    assert a.size() == 2

    keys: list = []
    for f in a:
        keys.append(f[0])
    assert keys == [0, magic]


def test_vector():
    a = cxxstd.vector()
    count = 12
    for i in range(count):
        a.append(i + 1)
    assert len(a) == count

    for i in range(count):
        assert a[i] == i + 1


if __name__ == "__main__":
    test_cint()
    test_map()
    test_vector()
    del rc
    del cxxstd
