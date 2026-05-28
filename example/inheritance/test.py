import ext

if __name__ == "__main__":
    a = ext.make_animal()
    c = ext.make_cat()

    assert c.make_noise() == "meow"

    assert isinstance(a, type(a))
    # c is an instance of Cat (inherited from Animal), 
    # so isinstance(c, type(a)) should return True.
    assert isinstance(c, type(a))

    assert not isinstance(a, type(c))
