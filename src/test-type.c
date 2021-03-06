/*
 * Tests Type Parser
 *
 * Test the type-parser for correctness and completeness. This includes static
 * tests for known values, as well as automated tests for random types.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"
#include "c-dvar-type.h"

static const char test_array_signature[] = {
        "u"
        "()"
        "(nq)"
        "a{sa(vt)}"
        "({uu}{uu})"
        "(yqut)"
};

static const CDVarType test_array[] = {
        /* "u" */
        C_DVAR_T_INIT(
                C_DVAR_T_u
        ),
        /* "()" */
        C_DVAR_T_INIT(
                C_DVAR_T_TUPLE0
        ),
        /* "(nq)" */
        C_DVAR_T_INIT(
                C_DVAR_T_TUPLE2(
                        C_DVAR_T_n,
                        C_DVAR_T_q
                )
        ),
        /* "a{sa(vt)}" */
        C_DVAR_T_INIT(
                C_DVAR_T_ARRAY(
                        C_DVAR_T_PAIR(
                                C_DVAR_T_s,
                                C_DVAR_T_ARRAY(
                                        C_DVAR_T_TUPLE2(
                                                C_DVAR_T_v,
                                                C_DVAR_T_t
                                        )
                                )
                        )
                )
        ),
        /* "({uu}{uu})" */
        C_DVAR_T_INIT(
                C_DVAR_T_TUPLE2(
                        C_DVAR_T_PAIR(
                                C_DVAR_T_u,
                                C_DVAR_T_u
                        ),
                        C_DVAR_T_PAIR(
                                C_DVAR_T_u,
                                C_DVAR_T_u
                        )
                )
        ),
        /* "(yqut)" */
        C_DVAR_T_INIT(
                C_DVAR_T_TUPLE4(
                        C_DVAR_T_y,
                        C_DVAR_T_q,
                        C_DVAR_T_u,
                        C_DVAR_T_t
                )
        ),
};

static void test_common(void) {
        CDVarType *type = (CDVarType[2]){};
        int r;

        r = c_dvar_type_new_from_string(&type, "()");
        assert(!r);

        assert(!memcmp(c_dvar_type_unit, type, type->length * sizeof(*type)));
        assert(!memcmp(c_dvar_type_unit,
                       (const CDVarType[]){ C_DVAR_T_INIT(C_DVAR_T_TUPLE0) },
                       c_dvar_type_unit->length * sizeof(CDVarType)));
}

static void test_known_types(void) {
        _cleanup_(c_dvar_type_freep) CDVarType *type = NULL;
        const CDVarType *expect;
        const char *signature;
        size_t i, n_signature;
        int r;

        /*
         * This runs c_dvar_type_new_from_signature() across
         * @test_array_signature until its end. It verifies the output is the
         * exact sequence provided by @test_array.
         */

        signature = test_array_signature;
        n_signature = strlen(signature);
        expect = test_array;

        while (n_signature) {
                r = c_dvar_type_new_from_signature(&type, signature, n_signature);
                assert(!r);

                /* this is redunant, but tests the compare operator */
                assert(!c_dvar_type_compare_string(type, signature, type->length));

                for (i = 0; i < type->length; ++i) {
                        assert(expect[i].size == type[i].size);
                        assert(expect[i].alignment == type[i].alignment);
                        assert(expect[i].element == type[i].element);
                        assert(expect[i].length == type[i].length);
                        assert(expect[i].basic == type[i].basic);
                }

                n_signature -= type->length;
                signature += type->length;
                expect += type->length;
                type = c_dvar_type_free(type);
        }

        assert(expect == test_array + sizeof(test_array) / sizeof(*test_array));
}

static void test_valid_types(void) {
        const char *signatures[] = {
                "()",
                "{tv}",
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau",
                "((((((((((((((((((((((((((((((((u))))))))))))))))))))))))))))))))",
                "((((((((((((((((((((((((((((((((aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau))))))))))))))))))))))))))))))))",
                "a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(u))))))))))))))))))))))))))))))))",
        };
        CDVarType *type = NULL;
        size_t i;
        int r;

        /*
         * This parses the signatures provided by @signatures one by one, each
         * of them must be valid. No further validation is done.
         */

        for (i = 0; i < sizeof(signatures) / sizeof(*signatures); ++i) {
                r = c_dvar_type_new_from_string(&type, signatures[i]);
                assert(!r);
                type = c_dvar_type_free(type);
        }
}

static void test_invalid_types(void) {
        const char *signatures[] = {
                "A", /* invalid element */
                "$", /* invalid element */
                "{}", /* invalid pair */
                "{)", /* non-matching brackets */
                "(}", /* non-matching brackets */
                "{()y}", /* invalid pair */
                "{yyy}", /* invalid pair */
                "(yy{))", /* non-matching brackets */
                "(yy{}}", /* invalid pair */
                "(yy{)}", /* non-matching brackets */
                "(", /* unclosed container */
                ")", /* closing wrong container */
                "((", /* unclosed container */
                ")(", /* closing wrong container */
                "a", /* unclosed container */
                "aaa", /* unclosed container */
                "{aau}", /* invalid pair */
                "{vu}", /* invalid pair*/
                "(uu(u())uu{vu}uu)", /* invalid pair */
                "(uu(u())uu(vu}uu)", /* non-matching brackets */
                "(uu(u())uu{uu)uu)", /* non-matching brackets */
                "(uu(u())uuuuuuuu}", /* non-matching brackets */
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau", /* depth exceeded */
                "(((((((((((((((((((((((((((((((((u)))))))))))))))))))))))))))))))))", /* depth exceeded */
                "a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(au))))))))))))))))))))))))))))))))", /* depth exceeded */
        };
        CDVarType *type = NULL;
        size_t i;
        int r;

        /*
         * This parses the signatures provided by @signatures one by one, each
         * of them must be rejected.
         */

        for (i = 0; i < sizeof(signatures) / sizeof(*signatures); ++i) {
                r = c_dvar_type_new_from_string(&type, signatures[i]);
                assert(r != 0);
                assert(r > 0);
        }
}

int main(int argc, char **argv) {
        test_common();
        test_known_types();
        test_valid_types();
        test_invalid_types();
        return 0;
}
