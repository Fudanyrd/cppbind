from myabc import myadd as add
from myabc import mysum

def test_add():
    assert add(1, 2) == 3
    assert add(-1, 1) == 0
    assert add(0, 0) == 0

    print('1 + 2 =', add(1, 2))

def test_sum():
    assert mysum(1, 2, 3) == 6
    assert mysum(-1, 1, 0) == 0

    lst = [i for i in range(5)]
    assert mysum(*lst) == sum(lst)
    del lst

    print('sum(1, 2, 3) =', mysum(1, 2, 3))

if __name__ == '__main__':
    test_add()
    test_sum()
