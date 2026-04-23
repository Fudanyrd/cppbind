from myabc import myadd as add

def test_add():
    assert add(1, 2) == 3
    assert add(-1, 1) == 0
    assert add(0, 0) == 0

    print('1 + 2 =', add(1, 2))

if __name__ == '__main__':
    test_add()
