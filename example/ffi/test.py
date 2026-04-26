import _ffi
from _ffi import CInt_New, CInt_FromInt

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

if __name__ == "__main__":
    test_new_del()
    test_new_del2()
    del _ffi

