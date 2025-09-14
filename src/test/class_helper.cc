//C standard library
#include <cstdlib>
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "class_helper.hh"
#include "memcry_helper.hh"



void _class_helper::str::setup_stub(char *& str) {

    //allocate & populate
    str = (char *) std::malloc(8);
    REQUIRE_NE(str, nullptr);
    strncpy(str, "stub\0", 5);

    return;
}


void _class_helper::vct::setup_stub(cm_vct & vct) {

    int num;


    //allocate a data buffer
    vct.data = malloc(0x10);
    REQUIRE_NE(vct.data, nullptr);

    //set a false number of elements
    num = (rand() % 9) + 8;
    vct.len = num;
    vct.sz = num;

    //set vector as initialised
    vct.is_init = true;

    return;
}


void _class_helper::lst::setup_stub(cm_lst & lst) {

    int num;
    cm_lst_node * node;


    //set a false number of elements
    node = (cm_lst_node *) std::malloc(sizeof(cm_lst_node));
    REQUIRE_NE(node, nullptr);

    //allocate a data area
    node->data = std::malloc(10);
    REQUIRE_NE(node->data, nullptr);

    //setup the node & list
    node->prev = nullptr;
    node->next = nullptr;

    lst.is_init = true;
    lst.head = node;
    lst.len = 1;
    lst.data_sz = 10;

    return;
}


enum cm_rbt_side _class_helper::rbt::_compare(
    const void *, const void *) { return CM_RBT_LESS; }

void _class_helper::rbt::setup_stub(cm_rbt & rbt) {

    int num;
    cm_rbt_node * node;


    //allocate a new node;
    node = (cm_rbt_node *) std::malloc(sizeof(cm_rbt_node));
    REQUIRE_NE(node, nullptr);

    //allocate a key & data area
    node->data = std::malloc(10);
    REQUIRE_NE(node->data, nullptr);

    node->key = std::malloc(10);
    REQUIRE_NE(node->data, nullptr);

    //setup the node & tree
    node->colour = CM_RBT_BLACK;
    node->parent_side = CM_RBT_ROOT;
    node->parent = node->left = node->right = NULL;

    rbt.is_init = true;
    rbt.root = node;
    rbt.size = 1;
    rbt.data_sz = 10;
    rbt.key_sz = 10;

    return;
}


void _class_helper::ma_set::print_set(const sc::map_area_set & ma_set) {

    int ret;

    cm_rbt_node * node;
    cm_lst_node * area_node;
    mc_vm_area * area;


    //fetch the set
    const cm_rbt & set = ma_set.get_set();
    REQUIRE_EQ(set.is_init, true);


    //print each area
    for (int i = 0; i < set.size; ++i) {

        //fetch red-black tree node
        node = cm_rbt_idx_get_n(&set, i);
        REQUIRE_NE(node, nullptr);

        //fetch key & data
        area_node = SC_GET_SET_KEY(node);
        area = SC_GET_SET_DATA(node);
        REQUIRE_EQ(area, MC_GET_NODE_AREA(area_node));

        //print this area
        _memcry_helper::print_area(area);
    }
    

    return;
}
