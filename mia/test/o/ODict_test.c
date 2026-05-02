#include "o/ODict.h"
#include "o/OMap.h"

#include "o/str.h"

#define test(expr) o_assume(expr, "test failed")


int ODict__test(oobj obj)
{
    oobj dict = ODict_new(obj, 128);
    OMap_remove_able_set(ODict_map(dict), true);
    osize oobjs_a, datas_a, depths_a;
    osize oobjs_b, datas_b, depths_b;

    // first set with new hash creates an internal OArray
    ODict_set(dict, "foo", "bar");
    ODict_remove(dict, "foo");

    // we redo the set, and check if after removing the key, the sizes match
    OObj_tree_metrics(dict, &oobjs_a, &datas_a, &depths_a);
    ODict_set(dict, "foo", "bar");
    ODict_remove(dict, "foo");
    OObj_tree_metrics(dict, &oobjs_b, &datas_b, &depths_b);
    test(oobjs_a == oobjs_b);
    test(datas_a == datas_b);
    test(depths_a == depths_b);

    // this test checks if overriding changes the internal size
    ODict_set(dict, "foo", "bar");
    OObj_tree_metrics(dict, &oobjs_a, &datas_a, &depths_a);
    ODict_set(dict, "foo", "baz");
    OObj_tree_metrics(dict, &oobjs_b, &datas_b, &depths_b);
    test(oobjs_a == oobjs_b);
    test(datas_a == datas_b);
    test(depths_a == depths_b);

    // check content of the dict
    ODict_set(dict, "bar", "goo");
    test(o_num(dict) == 2);
    test(o_str_equals(ODict_get(dict, "foo"), "baz"));
    test(o_str_equals(ODict_get(dict, "bar"), "goo"));

    return 0;
}

