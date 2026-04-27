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


def _compare(a: int, b: int) -> bool:
    ":returns: True if a < b else False"
    # print(a, b)
    return True if a < b else False


def test_cppmap():
    table = CppMap_New(_compare)
    for i in range(4):
        table.put(i, i * i)

    for i in range(4):
        assert table.get(i) == i * i
    assert table.size() == 4
    del table

    table = CppMap_New(_compare)
    table.put(0, "😀")
    method = table.get
    del table
    assert method(0) == "😀"
    del method


if __name__ == "__main__":
    test_new_del()
    test_new_del2()
    test_cppmap()
    del _ffi

