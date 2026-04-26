from _ffi import CInt_New

def test_new_del():
    a = CInt_New()
    b = a.getvalue()
    assert b == 0
    del b
    del a 


if __name__ == "__main__":
    test_new_del()
