from myabc import myadd as add
from myabc import mysum, mysum_vec # fastcall
from myabc import kwarg_names
from myabc import len_args_kwargs
from myabc import always_throw

def test_add():
    assert add(1, 2) == 3
    assert add(-1, 1) == 0
    assert add(0, 0) == 0

    try:
        add(1)
        assert False, "add should raise TypeError when given too few arguments"
    except TypeError:
        pass

    print('1 + 2 =', add(1, 2))


def test_kwarg_names():
    print(kwarg_names.__doc__)
    assert kwarg_names(1, 2) == []
    assert kwarg_names() == []

    assert kwarg_names(None, a = 1, b = 2) == ['a', 'b']
    assert kwarg_names(None, b = 2, a = 1) == ['b', 'a']

    d = {
        "a": 1,
        "b": 2,
    }
    assert list(d.keys()) == kwarg_names(d, **d)


def test_sum():
    assert mysum(1, 2, 3) == 6
    assert mysum(-1, 1, 0) == 0

    lst = [i for i in range(5)]
    assert mysum(*lst) == sum(lst)
    del lst

    print('sum(1, 2, 3) =', mysum(1, 2, 3))


def test_sum_vec():
    assert mysum_vec(1, 2, 3) == 6
    assert mysum_vec(-1, 1, 0) == 0

    lst = [i for i in range(5)]
    assert mysum_vec(*lst) == sum(lst)
    del lst


def test_len_args_kwargs():
    assert len_args_kwargs() == (0, 0)
    assert len_args_kwargs(1, 2) == (2, 0)
    assert len_args_kwargs(a = 1, b = 2) == (0, 2)
    assert len_args_kwargs(1, 2, a = 1, b = 2) == (2, 2)


def test_always_throw():
    try:
        always_throw()
    except ValueError:
        pass
    else:
        assert False, "always_throw should raise ValueError"


if __name__ == '__main__':
    test_add()
    test_kwarg_names()
    test_sum()
    test_sum_vec()
    test_len_args_kwargs()
    test_always_throw()
