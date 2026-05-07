import _ffi
rc = _ffi.debug_refcnt

def test_cint():
    a = _ffi.cint()
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
    a = _ffi.map()
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

if __name__ == "__main__":
    test_cint()
    test_map()
    del rc
    del _ffi
