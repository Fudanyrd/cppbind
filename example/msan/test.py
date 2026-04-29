from msan import play
from msan import Resource_New


def test_create():
    """ Check that one and only one instance can be created. """
    b = Resource_New()
    del b
    a = Resource_New()
    print(a)
    try:
        b = Resource_New()
        assert False, "two instances allocated"
    except RuntimeError:
        pass
    del a


def test_invoke_cpp_function():
    """ Check that C++ functions manages referece count correctly. """
    a = Resource_New()
    play(1, 2, kv=a) #  test when Resource is in kwargs.
    play(1, a, a, foo=1) # test when Resource is in args.
    play(a, a, b=1, c=2, other=a) # test mixed call
    del a

    # try create an instance again.
    a = Resource_New()
    del a


def test_get_count():
    """ Check that C++ method manages referece count correctly. """
    a = Resource_New()
    meth = a.getcount
    print(meth())
    del meth
    del a
    
    # try create again.
    b = Resource_New()
    del b


if __name__ == "__main__":
    test_create()
    test_invoke_cpp_function()
    test_get_count()
