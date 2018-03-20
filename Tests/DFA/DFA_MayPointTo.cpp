int foo(unsigned a) {
    int *ptr;
    if (a < 10) {
        ptr = new int(a * 3);
    } else {
        ptr = new int(a * 2);
    }
    return *ptr;
}

