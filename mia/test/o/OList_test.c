#include "o/OList.h"
#include "o/str.h"

#define test(expr) o_assume(expr, "test failed")

int OList__test(oobj obj)
{
    oobj list = OList_new(obj, (void**) o_list_compound(char*, "Hello", "World"), -1);
    test(o_num(list) == 2);
    test(o_str_equals(o_at(list, 0), "Hello"));
    test(o_str_equals(o_at(list, 1), "World"));

    OList_push(list, "Foo");
    char *pop = OList_pop_front(list);
    test(o_str_equals(pop, "Hello"));
    test(o_num(list) == 2);
    test(o_str_equals(o_at(list, 0), "World"));
    test(o_str_equals(o_at(list, 1), "Foo"));

    OList_append_at(list, 1, (void**) o_list_compound(char*, "Bar", "Baz"), -1);
    test(o_num(list) == 4);
    test(o_str_equals(o_at(list, 0), "World"));
    test(o_str_equals(o_at(list, 1), "Bar"));
    test(o_str_equals(o_at(list, 2), "Baz"));
    test(o_str_equals(o_at(list, 3), "Foo"));

    return 0;
}

