import _ffi
from _ffi import CInt_New, CInt_FromInt
from _ffi import CppMap_New

def test_new_del():
    a = CInt_New()
    b = a.getvalue()
    assert b == 0
    del b
    del a 

def test_new_del2():
    magic = 0xc0ffee
    a = CInt_FromInt(magic)
    b = a.getvalue()
    assert b == magic
    del b
    del a


def test_nonexist_attr():
    magic = 0xc0ffee
    a = CInt_FromInt(magic)
    try:
        b = a.value() # TYPO
        assert False, "Expected AttributeError"
    except AttributeError as ex:
        print(ex)
    del a


def _compare(a: int, b: int) -> bool:
    ":returns: True if a < b else False"
    # print(a, b)
    return True if a < b else False


def test_cppmap():
    table = CppMap_New(_compare)
    for i in range(4):
        table[i] = i * i

    for i in range(4):
        assert table[i] == i * i
    assert table.get(4) is None
    assert table.get(4, 'foo') == 'foo' # call with default key
    try:
        table[4]
        assert False, "Expected KeyError"
    except KeyError:
        pass

    # check that CppMap handles non-comparable keys properly
    try:
        table['not comparable']
        assert False, "Expected Some Error because cannot compare int and str"
    except Exception as ex:
        print(ex)
    try:
        table.put('not comparable', 'value')
        assert False, "Expected Some Error because cannot compare int and str"
    except Exception as ex:
        print(ex)
    assert table.size() == 4
    del table

    table = CppMap_New(_compare)
    table.put(0, "😀")
    # starting from 0c1e2e13, putting `None` is allowed
    table.put(1, None)
    method = table.get
    del table
    assert method(0) == "😀"
    assert method(1) is None
    del method


if __name__ == "__main__":
    test_new_del()
    test_new_del2()
    test_nonexist_attr()
    test_cppmap()
    del _ffi
    print("All tests passed.")

