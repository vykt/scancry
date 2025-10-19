//C++ builtin
#include <new>

//C standard library
#include <cstring>
#include <unistd.h>
#ifdef SC_TRACE
#include <cstdio>
#endif

//system headers
#include <limits.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "ptrscan.hh"
#include "common.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [POINTER CHAIN NODE | PRIVATE] ---
 */

void sc::ptr_chain_node::do_copy(
    const sc::ptr_chain_node & p_chain_node) noexcept {

    //copy shallow analysis data
    this->off         = p_chain_node.get_off();
    this->obj_tbl_idx = p_chain_node.get_obj_tbl_idx();

    //copy deep analysis data
    this->addr      = p_chain_node.get_addr();
    this->obj_node  = p_chain_node.get_obj_node();
    this->area_node = p_chain_node.get_area_node();
    this->obj_off   = p_chain_node.get_obj_off();
    this->area_off  = p_chain_node.get_area_off();

    return;
}



/*
 *  --- [POINTER CHAIN NODE | INTERNAL] ---
 */

[[nodiscard]] int sc::ptr_chain_node::_follow(
    const mc_vm_map * map,
    mc_session * sess,
    uintptr_t & cur_addr) noexcept {

    int ret;


    //read the target address
    ret = mc_read(sess, cur_addr + this->off,
                  (cm_byte *) &cur_addr, sizeof(cur_addr));
    if (ret != 0) {
        sc_errno = SC_ERR_MEMCRY;
        return -1;
    }

    return 0;
}


[[nodiscard]] int sc::ptr_chain_node::_update_live_data(
    const mc_vm_map * map,
    mc_session * sess,
    uintptr_t & cur_addr) noexcept {

    int ret;
    mc_vm_area * area;


    //update the current address
    cur_addr = cur_addr + this->off;

    //set this node's address
    this->addr = cur_addr;

    //determine which area this address belongs to
    this->area_node = mc_get_area_by_addr(map, cur_addr, &this->area_off);
    if (this->area_node == nullptr) {
        sc_errno = SC_ERR_MEMCRY;
        return -1;
    }

    //extrapolate which object this address belongs to
    area = MC_GET_NODE_AREA(this->area_node);
    this->obj_node = (area->obj_node_p == nullptr)
                     ? area->last_obj_node_p : area->obj_node_p;

    //determine the offset from the object
    this->obj_off = mc_get_obj_off(this->obj_node, this->addr);

    //read the target address
    ret = mc_read(sess, cur_addr,
                  (cm_byte *) &cur_addr, sizeof(cur_addr));
    if (ret != 0) {
        sc_errno = SC_ERR_MEMCRY;
        return -1;
    }

    return 0;
}



/*
 *  --- [POINTER CHAIN NODE | PUBLIC] ---
 */

//constructor
sc::ptr_chain_node::ptr_chain_node(
 const off_t off,
 const int obj_tbl_idx,
 const off_t obj_tbl_off) noexcept
 : off(off),
   obj_tbl_idx(obj_tbl_idx),
   obj_tbl_off(obj_tbl_off),
   addr(0x0),
   obj_node(nullptr),
   area_node(nullptr),
   obj_off(0x0),
   area_off(0x0) {}


//copy constructor
sc::ptr_chain_node::ptr_chain_node(
    const sc::ptr_chain_node & p_chain_node) noexcept {

    this->do_copy(p_chain_node);
    return;
}


//destructor
sc::ptr_chain_node::~ptr_chain_node() noexcept {}


//copy assignment operator
sc::ptr_chain_node & sc::ptr_chain_node::operator=(
    const sc::ptr_chain_node & p_chain_node) noexcept {

    if (this != &p_chain_node) this->do_copy(p_chain_node);
    return *this;
}


//getters
[[nodiscard]] off_t sc::ptr_chain_node::get_off() const noexcept {
    return this->off;
}

[[nodiscard]] int
    sc::ptr_chain_node::get_obj_tbl_idx() const noexcept {
    return this->obj_tbl_idx;
}

[[nodiscard]] uintptr_t sc::ptr_chain_node::get_addr() const noexcept {
    return this->addr;
}

[[nodiscard]] const cm_lst_node *
    sc::ptr_chain_node::get_obj_node() const noexcept {
    return this->obj_node;
}

[[nodiscard]] const cm_lst_node *
    sc::ptr_chain_node::get_area_node() const noexcept {
    return this->area_node;
}

[[nodiscard]] off_t sc::ptr_chain_node::get_obj_off() const noexcept {
    return this->obj_off;
}

[[nodiscard]] off_t sc::ptr_chain_node::get_area_off() const noexcept {
    return this->area_off;
}



/*
 *  --- [POINTER CHAIN | PRIVATE] ---
 */

//perform a copy
void sc::ptr_chain::do_copy(
    const sc::ptr_chain & p_chain) noexcept {

    int ret;


    //call parent's assignment operator
    sc::_ctor_failable::operator=(p_chain);

    //copy static flag
    this->is_static = p_chain.get_static();

    //copy nodes
    _CTOR_VCT_COPY_IF_INIT(this->nodes, p_chain.get_nodes());

    return;
}


[[nodiscard]] uintptr_t sc::ptr_chain::resolv_start_addr(
    const mc_vm_map * map,
    const sc::obj_table & obj_tbl) noexcept {

    sc::ptr_chain_node * node;

    int ser_obj_pathname_idx;
    const char * obj_pathname;
    const cm_lst_node * obj_node;
    mc_vm_obj * obj;


    //if the chain is empty, return NULL
    if (this->nodes.len == 0) return 0x0;

    //get the starting node
    node = (sc::ptr_chain_node *) cm_vct_get_p(&this->nodes, 0);    

    //get the object
    obj_node = obj_tbl.resolv_obj_node(node->get_obj_tbl_idx(), map);
    if (obj_node == nullptr) return 0;
    obj = MC_GET_NODE_OBJ(obj_node);

    //return the start address
    return obj->start_addr;
}


#if 0
//(re)build a pointer chain
[[nodiscard]] int sc::ptr_chain::build(
    const char * pathname,
    const mc_vm_map & map,
    const mc_session & session,
    const sc::addr_width a_width,
    const sc::map_area_set & static_set) noexcept {

    int ret;
    int ret_val = -1;

    uintptr_t addr;
    uintptr_t first_addr;
    uint64_t rd_addr;
    off_t off;

    const cm_rbt * set;
    const cm_lst_node * obj_node;
    const cm_lst_node * area_node;
    const mc_vm_obj * obj;
    const mc_vm_area * area;

    void * alloc_addr;
    cm_byte _placeholder[sizeof(sc::ptr_chain_node)] = {0};


    //read lock the static set
    ret = static_set._lock_read();
    if (ret != 0) return -1;

    //get the set & assert it is initialised
    set = &static_set.get_set();
    if (set->is_init == false) {
        sc_errno = SC_ERR_OPT_EMPTY;
        goto _ptr_chain_build_cleanup;
    }

    //find the starting object
    obj_node = mc_get_obj_by_pathname(&map, pathname);
    if (obj_node == nullptr) {
        sc_errno = SC_ERR_MEMCRY;
        goto _ptr_chain_build_cleanup;
    }
    obj = MC_GET_NODE_OBJ(obj_node);

    //bootstrap iteration
    addr = obj->start_addr;

    //traverse each offset to initialise nodes
    for (int i = 0; i < this->offs.len; ++i) {

        //get the next offset
        ret = cm_vct_get(&this->offs, i, &off);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            goto _ptr_chain_build_cleanup;
        }

        //first offset is special
        if (i == 0) {

            //find the area of the start of the pointer chain
            first_addr = addr + off;
            area_node = mc_get_area_by_addr(&map, first_addr, nullptr);
            if (area_node == nullptr) {
                sc_errno = SC_ERR_MEMCRY;
                goto _ptr_chain_build_cleanup;
            }

            //determine if 
            
        }

        //allocate space for a new pointer chain node
        ret = cm_vct_apd(&this->nodes, _placeholder);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            goto _ptr_chain_build_cleanup;
        }

        //construct a new pointer chain node in the new allocation
        alloc_addr = cm_vct_get_p(&this->nodes, -1);
        new (alloc_addr) sc::ptr_chain_node(off, )




        //read the target addr
        
        
    }


    //release the read lock on the static set
    _ptr_chain_build_cleanup:
    static_set._unlock();
    return ret_val;
}
#endif



/*
 *  --- [POINTER CHAIN | PUBLIC] ---
 */

//constructor
sc::ptr_chain::ptr_chain(
    const cm_vct /* <off_t> */ & off_vct,
    const cm_vct /* <int> */ & obj_tbl_idx_vct,
    const cm_vct /* <off_t> */ & obj_tbl_off_vct,
    const bool is_static) noexcept
 : _ctor_failable(),
   is_static(is_static) {

    int ret;

    void * new_elem;
    cm_byte _placeholder[sizeof(sc::ptr_chain_node)] = {0};

    off_t * off;
    int * tbl_idx;
    off_t * tbl_off;


    //assert vector lengths match
    if (off_vct.len != obj_tbl_idx_vct.len) {
        goto _ptr_chain_ctor_fail_1;
    }

    //initialise nodes
    ret = cm_new_vct(&this->nodes, sizeof(sc::ptr_chain_node));
    if (ret != 0) goto _ptr_chain_ctor_fail_1;
    
    //construct pointer chain nodes
    for (int i = off_vct.len - 1; i >= 0; --i) {

        //get the offset and pathname index for the next node
        off = (off_t *) cm_vct_get_p(&off_vct, i);
        tbl_idx = (int *) cm_vct_get_p(&obj_tbl_idx_vct, i);
        tbl_off = (off_t *) cm_vct_get_p(&obj_tbl_idx_vct, i);

        //construct the next node
        new_elem = cm_vct_apd(&this->nodes, _placeholder);
        if (new_elem == nullptr) goto _ptr_chain_ctor_fail_2;
        new (new_elem) sc::ptr_chain_node(*off, *tbl_idx, *tbl_off);
    }

    return;

    _ptr_chain_ctor_fail_2:
    cm_del_vct(&this->nodes);

    _ptr_chain_ctor_fail_1:
    _set_ctor_failed(true);

    return;
}


//copy constructor
sc::ptr_chain::ptr_chain(
    const sc::ptr_chain & p_chain) noexcept {

    this->do_copy(p_chain);
    return;
}


//destructor
sc::ptr_chain::~ptr_chain() noexcept {

    //delete the nodes & offs vectors
    _CTOR_VCT_DELETE_IF_INIT(this->nodes);

    return;
}


//copy assignment operator
sc::ptr_chain & sc::ptr_chain::operator=(
    const sc::ptr_chain & p_chain) noexcept {

    if (this != &p_chain) this->do_copy(p_chain);
    return *this;
}


/*
 *  NOTE: This function is unique; a memcry related failure does not
 *        indicate a error in scancry itself, rather that following
 *        this chain is not possible.
 */

//follow & verify
/* -1 = can't follow chain, presume invalid, 0 = success */
[[nodiscard]] int sc::ptr_chain::verify(
    const mc_vm_map * map,
    mc_session * sess,
    const sc::obj_table & obj_tbl,
    const uintptr_t tgt_addr) noexcept {

    int ret;
    uintptr_t addr;
    sc::ptr_chain_node * node;


    //get the starting address
    addr = this->resolv_start_addr(map, obj_tbl);
    if (addr == 0x0) return -1;

    //for all nodes
    for (int i = 0; i < this->nodes.len; ++i) {

        //get the next node
        node = (sc::ptr_chain_node *) cm_vct_get_p(&this->nodes, i);

        //attempt to follow to the next node
        ret = node->_follow(map, sess, addr);
        if (ret != 0) return -1;
    }

    //assert that the final address matches the target address
    return (addr == tgt_addr) ? 0 : -1;
}


//follow & populate live data
/* -1 = can't follow chain, presume invalid, 0 = success */
[[nodiscard]] int sc::ptr_chain::update_live_data(
    const mc_vm_map * map,
    mc_session * sess,
    const sc::obj_table & obj_tbl) noexcept {

    int ret;
    uintptr_t addr;
    sc::ptr_chain_node * node;


    //get the starting address
    addr = this->resolv_start_addr(map, obj_tbl);
    if (addr == 0x0) return -1;

    //for all nodes
    for (int i = 0; i < this->nodes.len; ++i) {

        //get the next node
        node = (sc::ptr_chain_node *) cm_vct_get_p(&this->nodes, i);

        //attempt to follow to the next node & populate deep analysis
        ret = node->_update_live_data(map, sess, addr);
        if (ret != 0) return -1;
    }

    return 0;
}


//getters
[[nodiscard]] bool sc::ptr_chain::get_static() const noexcept {
    return this->is_static;
}

[[nodiscard]] const cm_vct /* <sc::ptr_chain_node> */ &
    sc::ptr_chain::get_nodes() const noexcept {
    return this->nodes;
}



/*
 *  --- [POINTER TREE NODE | PUBLIC]
 */

//constructor
sc::_ptr_tree_node::_ptr_tree_node(
    const int id,
    const cm_lst_node * area_node,
    const uintptr_t own_addr,
    const uintptr_t ptr_addr,
    const _ptr_tree_node * parent) noexcept
 : _lockable(),
   id(id),
   area_node(area_node),
   own_addr(own_addr),
   ptr_addr(ptr_addr),
   parent(parent) {

    //initialise the children list
    cm_new_lst(&this->child_nodes, sizeof(sc::_ptr_tree_node));

    return;
}


//destructor
sc::_ptr_tree_node::~_ptr_tree_node() noexcept {

    cm_lst_node * tmp_node;
    sc::_ptr_tree_node * tmp_p_tree_node;


    //cleanup child nodes
    tmp_node = this->child_nodes.head;
    for (int i = 0; i < this->child_nodes.len; ++i) {
        tmp_p_tree_node = _SC_GET_NODE_PTR_TREE_NODE(tmp_node);
        tmp_p_tree_node->sc::_ptr_tree_node::~_ptr_tree_node();
        tmp_node = tmp_node->next;
    }

    //delete the child node list
    cm_del_lst(&this->child_nodes);

    return;
}


//add a new child
[[nodiscard]] sc::_ptr_tree_node * sc::_ptr_tree_node::add_child(
    const int id,
    const cm_lst_node * area_node,
    const uintptr_t own_addr,
    const uintptr_t ptr_addr) noexcept {

    int ret;

    cm_lst_node * new_node;
    cm_byte _placeholder[sizeof(sc::_ptr_tree_node)];


    //await a write lock
    ret = this->_await_write();
    if (ret != 0) return nullptr;

    //add a new child node
    new_node = cm_lst_apd(&this->child_nodes, _placeholder);
    if (new_node == nullptr) {
        sc_errno = SC_ERR_CMORE;
        return nullptr;
    }

    //construct a new pointer tree node in the new child node
    new (new_node->data) sc::_ptr_tree_node(
        id, area_node, own_addr, ptr_addr, this);    

    //unlock
    this->_unlock();

    return (sc::_ptr_tree_node *) new_node->data;
}


//determine if this is a leaf node
[[nodiscard]] bool sc::_ptr_tree_node::has_children() const noexcept {
    return (this->child_nodes.len > 0);
}


//get children
[[nodiscard]] const cm_lst /* <sc::_ptr_tree_node> */ &
    sc::_ptr_tree_node::get_children() const noexcept {
    return this->child_nodes;
}



/*
 *  --- [POINTER TREE | PUBLIC] ---
 */

//constructor
sc::_ptr_tree::_ptr_tree() noexcept
 : _lockable(), _ctor_failable(),
   root_node(nullptr) {

    int ret;


    //initialise the depth levels vector
    ret = cm_new_vct(&this->depth_lvls, sizeof(cm_vct));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        this->_set_ctor_failed(true);
    }

    return;
}


//destructor
sc::_ptr_tree::~_ptr_tree() noexcept {

    cm_vct * lvl_vct;


    //destroy existing tree, if any
    if (this->root_node != nullptr) {
        this->root_node->~_ptr_tree_node();
        std::free(this->root_node);
    }

    //destroy depth level vectors
    for (int i = 0; i < this->depth_lvls.len; ++i) {
        lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, i);
        if (lvl_vct == nullptr) continue;
        cm_del_vct(lvl_vct);
    }

    //destroy outer vector
    _CTOR_VCT_DELETE_IF_INIT(this->depth_lvls);
    
    return;
}


//resetter
[[nodiscard]] int sc::_ptr_tree::reset() noexcept {

    cm_vct * lvl_vct;


    //acquire write lock
    _LOCK_WRITE(-1)

    //destroy existing tree, if any
    if (this->root_node != nullptr) {
        this->root_node->~_ptr_tree_node();
        std::free(this->root_node);
    }

    //destroy depth level vectors
    for (int i = 0; i < this->depth_lvls.len; ++i) {
        lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, i);
        if (lvl_vct == nullptr) continue;
        cm_del_vct(lvl_vct);
    }

    //empty outer vector
    cm_vct_emp(&this->depth_lvls);

    //unlock
    _UNLOCK
    return 0;
}


//initialise a new root node
[[nodiscard]] int sc::_ptr_tree::init_root_node(
    const cm_lst_node * area_node,
    const uintptr_t own_addr,
    const uintptr_t ptr_addr) noexcept {

    int ret;
    void * ret_data;
    cm_vct /* <sc::_ptr_tree_node *> */ * lvl_vct;


    //acquire a write lock
    _AWAIT_WRITE(-1)

    //allocate space for the root node
    this->root_node = (sc::_ptr_tree_node *)
                          std::malloc(sizeof(sc::_ptr_tree_node));    
    if (this->root_node == nullptr) { sc_errno = SC_ERR_MEM; return -1; }

    //construct a new root node
    new (this->root_node) sc::_ptr_tree_node(
        this->next_id, area_node, own_addr, ptr_addr, nullptr);
    ++this->next_id;

    //add a tree level for the root node if one does not exist
    if ((this->depth_lvls.is_init == false)
        or (this->depth_lvls.len == 0)) {

        ret = this->inc_depth_lvl();
        if (ret != 0) goto _ptr_tree_init_root_node_fail;
    }

    //add the root node to the first level
    lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, 0);
    if (lvl_vct == nullptr) goto _ptr_tree_init_root_node_fail;
    ret_data = cm_vct_apd(lvl_vct, this->root_node);
    if (ret_data == nullptr) goto _ptr_tree_init_root_node_fail;

    //unlock
    _UNLOCK
    return 0;

    _ptr_tree_init_root_node_fail:
    this->root_node->~_ptr_tree_node();
    std::free(this->root_node);
    return -1;
}


//increment the depth level
[[nodiscard]] int sc::_ptr_tree::inc_depth_lvl() noexcept {

    int ret;
    cm_vct * new_vct;

    cm_byte placeholder[sizeof(cm_vct)] = {0};


    //acquire a write lock
    _AWAIT_WRITE(-1);

    //allocate space for a new tree level vector
    new_vct = (cm_vct *) cm_vct_apd(&this->depth_lvls, placeholder);
    if (new_vct == nullptr) {
        sc_errno = SC_ERR_CMORE;
        goto _ptr_tree_inc_depth_lvl_fail;
    }

    //initialise the new tree level vector
    ret = cm_new_vct(new_vct, sizeof(sc::_ptr_tree_node *));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        /* discard */ ret = cm_vct_rmv(&this->depth_lvls, -1);
        goto _ptr_tree_inc_depth_lvl_fail;
    }

    //unlock
    _UNLOCK
    return 0;

    _ptr_tree_inc_depth_lvl_fail:
    _UNLOCK
    return -1;
}


[[nodiscard]] int sc::_ptr_tree::add_to_lvl(
    const int lvl, const sc::_ptr_tree_node * p_tree_node) noexcept {

    int ret;
    void * ret_data;
    cm_vct * lvl_vct;


    //acquire a write lock
    _AWAIT_WRITE(-1);

    //get a pointer to the appropriate tree level vector
    lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, -1);
    if (lvl_vct == nullptr) {
        sc_errno = SC_ERR_CMORE;
        goto _ptr_tree_add_to_lvl_fail;
    }

    //add a new node pointer to this tree level vector
    ret_data = cm_vct_apd(lvl_vct, &p_tree_node);
    if (ret_data == nullptr) {
        sc_errno = SC_ERR_CMORE;
        goto _ptr_tree_add_to_lvl_fail;
    }

    //unlock
    _UNLOCK
    return 0;

    _ptr_tree_add_to_lvl_fail:
    _UNLOCK
    return -1;
}


//get the next id for a node + increment counter
[[nodiscard]] int sc::_ptr_tree::get_next_id() noexcept {
    return this->next_id++;
}


//get a pointer to some tree level vector
[[nodiscard]] const cm_vct /* <sc::_ptr_tree_node *> */ *
    sc::_ptr_tree::get_depth_lvl(const int lvl) const noexcept {

    cm_vct * lvl_vct;


    //fetch the appropriate tree level vector
    lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, lvl);
    if (lvl_vct == nullptr) { sc_errno = SC_ERR_CMORE; return nullptr; }

    //return the tree level vector
    return lvl_vct;
}


//get the root node pointer
[[nodiscard]] const sc::_ptr_tree_node *
    sc::_ptr_tree::get_root_node() const noexcept {

    //return the root node
    return this->root_node;
}



/*
 *  --- [POINTER SCAN | PRIVATE] ---
 */

//reset the pointer scan
[[nodiscard]] int sc::ptrscan::do_reset() noexcept {
    
    int ret;
    sc::ptr_chain * p_chain;


    //reset the pointer tree
    ret = this->tree.reset();
    if (ret != 0) return -1;

    //delete pathnames vector
    this->obj_tbl.reset();

    //free chains
    for (int i = 0; i < this->chains.len; ++i) {
        p_chain = (sc::ptr_chain *) cm_vct_get_p(&this->chains, i);
        if (p_chain == nullptr) { sc_errno = SC_ERR_CMORE; continue; }
        p_chain->~ptr_chain();
    }

    //free chains vector
    _CTOR_VCT_DELETE_IF_INIT(this->chains);

    //reset the depth level
    this->depth_lvl = 0;
    this->depth_lvl_vct_p = nullptr;

    //reset the results state
    this->_unset_bits(sc::_scan_sf::scan_data);

    //unlock
    return 0;
}


//await for a single pass over the scan set to finish
[[nodiscard]] int sc::ptrscan::do_await_scan(
    sc::worker_pool & w_pool, const bool do_block) noexcept {

    int ret;
    int ret_val = 0;


    //get locks & check state
    ret = this->handle_entry(nullptr, nullptr, sc::_scan_sf::running, 0b0);
    if (ret != 0) return -1;

    //await for a scan run to conclude
    ret = w_pool._await_run(do_block);
    if (ret == -1) {
        ret_val = -1;
        goto _ptrscan_do_await_scan_cleanup;
    }
    if (ret == -2) {
        ret_val = -2;
        this->_unset_bits(sc::_scan_sf::running);
        goto _ptrscan_do_await_scan_cleanup;
    }

    //mark state as no longer running a scan
    this->_unset_bits(sc::_scan_sf::running);
    
    _ptrscan_do_await_scan_cleanup:
    this->handle_exit(nullptr, nullptr);
    
    return ret_val;
}



/*
 *  FIXME: This entire function does not cleanup properly in case of
 *         error.
 */

//recursively traverse the pointer tree to build chains
[[nodiscard]] int sc::ptrscan::chain_recurse(
    const mc_vm_map * map,
    const cm_rbt & static_set_tree,
    cm_vct /* <off_t> */ & off_stack,
    cm_vct /* <int> */ & obj_tbl_idx_stack,
    cm_vct /* <off_t> */ & obj_tbl_off_stack,
    const sc::_ptr_tree_node & p_tree_node,
    const uintptr_t tgt_addr) noexcept {

    int ret;
    void * ret_data;

    bool is_static;
    off_t off;
    int obj_tbl_idx;
    off_t obj_tbl_off;

    const cm_lst_node * area_node;
    const mc_vm_area * area;

    const cm_lst_node * obj_node;
    const mc_vm_obj * obj;

    const cm_rbt_node * ret_node;

    const cm_lst_node * child_node;
    sc::_ptr_tree_node * p_tree_child_node;

    void * alloc_addr;    
    cm_byte placeholder[sizeof(sc::ptr_chain)] = {0};


    //find the area for this node
    area_node = mc_get_area_by_addr(map, p_tree_node.own_addr, nullptr);
    if (area_node == nullptr) {
        sc_errno = SC_ERR_MEMCRY;
        return -1;
    }
    area = MC_GET_NODE_AREA(area_node);
    
    //determine if this area is treated as static
    ret_node = cm_rbt_get_n(&static_set_tree, area_node);
    if (ret_node == nullptr) {
        if (cm_errno == CM_ERR_USER_KEY) {
            is_static = false;
        } else {
            sc_errno = SC_ERR_CMORE;
            return -1;
        }
    } else { is_static = true; }

    //find the object for this node
    obj_node = (area->obj_node_p == nullptr)
                ? area->last_obj_node_p : area->obj_node_p;
    obj = MC_GET_NODE_OBJ(obj_node);


    //add to the offset stack
    off = tgt_addr - p_tree_node.ptr_addr;
    ret_data = cm_vct_apd(&off_stack, &off);
    if (ret_data == nullptr) { sc_errno = SC_ERR_CMORE; return -1; }

    //add to the object table index stack
    obj_tbl_idx = this->obj_tbl.add_pathname(obj->pathname);
    if (obj_tbl_idx == -1) return -1;
    ret_data = cm_vct_apd(&obj_tbl_idx_stack, &obj_tbl_idx);
    if (ret_data == nullptr) { sc_errno = SC_ERR_CMORE; return -1; }

    //add to the object table offset stack
    obj_tbl_off = mc_get_obj_off(obj_node, p_tree_node.own_addr);
    ret_data = cm_vct_apd(&obj_tbl_off_stack, &obj_tbl_off);
    if (ret_data == nullptr) { sc_errno = SC_ERR_CMORE; return -1; }


    //if this is a static node OR this node has no children, then
    //construct a new chain
    if ((is_static == true) || (p_tree_node.get_children().len == 0)) {

        //allocate space for the new chain
        alloc_addr = cm_vct_apd(&this->chains, placeholder);
        if (alloc_addr == nullptr) { sc_errno = SC_ERR_CMORE; return -1; }

        //construct the new chain in place
        new (alloc_addr) sc::ptr_chain(
            off_stack, obj_tbl_idx_stack, obj_tbl_off_stack, is_static);


    //else, recurse down the tree
    } else {

        //get children of this node
        const cm_lst /* <sc::_ptr_tree_node> */ & children
            = p_tree_node.get_children();

        //for all children
        child_node = children.head;
        for (int i = 0; i < children.len; ++i) {

            //setup the call
            p_tree_child_node = _SC_GET_NODE_PTR_TREE_NODE(child_node);

            //perform recursive call
            ret = this->chain_recurse(
                      map,
                      static_set_tree,
                      off_stack,
                      obj_tbl_idx_stack,
                      obj_tbl_off_stack,
                      *p_tree_child_node,
                      p_tree_node.own_addr);
            if (ret != 0) return -1;

            //advance iteration
            child_node = child_node->next;
        }
        
    } //end else recurse down the tree


    //pop offst stack
    ret = cm_vct_rmv(&off_stack, off_stack.len - 1);
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }

    //pop object table index stack
    ret = cm_vct_rmv(&obj_tbl_idx_stack, obj_tbl_idx_stack.len - 1);
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }

    //pop object table offset stack
    ret = cm_vct_rmv(&obj_tbl_off_stack, obj_tbl_off_stack.len - 1);
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }
    
    return 0;
}



/*
 *  --- [POINTER SCAN | INTERNAL] ---
 */

//process address callback
[[nodiscard]] off_t sc::ptrscan::_process_addr(
    const sc::_scan_arg & arg,
    const opt & opts,
    const _opt_scan & opts_scan) noexcept {

    int ret;
    void * ret_data;

    sc::addr_width aw;
    sc::smart_scan ss;

    uintptr_t potential_ptr;
    sc::_ptr_tree_node * p_tree_node;

    off_t smallest_off;
    off_t cur_off;
    off_t lvl_off;

    cm_vct /* <sc::_ptr_tree_node *> */ hit_p_tree_nodes;

    #ifdef SC_TRACE_PTRSCAN
    mc_vm_area * _trace_area;
    mc_vm_obj * _trace_obj;
    #endif


    //cast scan options
    sc::opt_ptrscan * opts_ptr = (sc::opt_ptrscan *) &opts_scan;

    //get address width
    ret = opts.get_addr_width(aw);
    if (__builtin_expect((ret != 0), 0)) return -1;

    //get smart scan
    ret = opts_ptr->get_smart_scan(ss);
    if (__builtin_expect((ret != 0), 0)) return -1;

    //get preset offset for this level, if any
    if (opts_ptr->get_preset_offsets().len > this->depth_lvl) {
        ret = cm_vct_get(this->depth_lvl_vct_p, this->depth_lvl, &lvl_off);
        if (__builtin_expect((ret != 0), 0)) {
            sc_errno = SC_ERR_CMORE;
            return -1;
        }
    } else lvl_off = -1;

    //setup hit tree nodes
    ret = cm_new_vct(&hit_p_tree_nodes, sizeof(sc::_ptr_tree_node *));
    if (__builtin_expect((ret != 0), 0)) {
        goto _ptrscan_process_addr_fail;
    }

    //for every potential pointer in this buffer
    for (off_t buf_off = 0;
         (arg.get_buf_left() - buf_off) >= arg.get_buf_left() - (size_t) aw;
         buf_off += opts_ptr->get_alignment()) {

        //get potential pointer
        switch(aw) {
            case (sc::AW64):
                potential_ptr
                    = *((uint64_t *) (arg.get_cur_byte() + buf_off));
                break;
            case (sc::AW32):
                potential_ptr
                    = *((uint32_t *) (arg.get_cur_byte() + buf_off));
                break;
            default:
                sc_errno = SC_ERR_OPT_MISSING;
                goto _ptrscan_process_addr_fail;
        }

        //setup smallest offset
        smallest_off = opts_ptr->get_max_obj_sz();

        //for every pointer node at this tree level
        for (int i = 0; i < this->depth_lvl_vct_p->len; ++i) {

            /*
             *  NOTE: Performing a manual vector access here to permit
             *        vectorisation optimisations, hopefully.
             */

            //get next pointer node
            p_tree_node
                = ((sc::_ptr_tree_node *) this->depth_lvl_vct_p->data) + i;

            //if potential pointer can't be pointing to this node, continue
            if (__builtin_expect(
                    (potential_ptr
                     < p_tree_node->own_addr - opts_ptr->get_max_obj_sz())
                    || (potential_ptr > (p_tree_node->own_addr)), 1)) {
                continue;
            }


            /*
             *  NOTE: From here on, the potential pointer appears to point
             *        to the the tree node in this iteration.
             */

            //check the preset offset matches, if any
            if (__builtin_expect(
                  ((lvl_off != 0)
                   && (potential_ptr != (p_tree_node->own_addr - lvl_off))),
                  0)) continue;

            //if this is a smart scan, adjust hits
            if (__builtin_expect((ss == sc::SMART_SCAN_ENABLED), 1)) {
                cur_off = p_tree_node->own_addr - potential_ptr;

                /*
                 *  NOTE: A smart scan assumes a pointer belongs only to
                 *        tree nodes to which its offset is smallest.
                 *        Therefore, it's necessary to discard previously
                 *        smallest matches, and update the smallest offset.
                 */

                //if this offset is greater than current minimum, skip
                if (__builtin_expect((cur_off > smallest_off), 1))
                    continue;
    
                //if this offset is smaller than current minimum, reset
                if (__builtin_expect((cur_off < smallest_off), 1)) {
                    cm_vct_emp(&hit_p_tree_nodes);
                    smallest_off = cur_off;
                }
            }

            //save this pointer node
            ret_data = cm_vct_apd(&hit_p_tree_nodes, &p_tree_node);
            if (__builtin_expect((ret_data == nullptr), 0)) {
                sc_errno = SC_ERR_CMORE;
                goto _ptrscan_process_addr_fail;
            }

        } //end for every pointer node at this tree level


        /*
         *  NOTE: At this point, this potential pointer is deemed to
         *        belong to some subset of tree nodes at the current
         *        depth level.
         */

        //for all discovered hits
        for (int i = 0; i < hit_p_tree_nodes.len; ++i) {

            //get next pointer node
            ret = cm_vct_get(&hit_p_tree_nodes, i, &p_tree_node);
            if (__builtin_expect((ret != 0), 0)) {
                sc_errno = SC_ERR_CMORE;
                goto _ptrscan_process_addr_fail;
            }

            //create a child node under it
            p_tree_node = p_tree_node->add_child(
                              this->tree.get_next_id(),
                              arg.get_area_node(),
                              arg.get_addr() + buf_off,
                              potential_ptr);
            if (p_tree_node == nullptr) goto _ptrscan_process_addr_fail;

        } //end for all discovered hits

        //reset hits before next iteration
        cm_vct_emp(&hit_p_tree_nodes);

    } //end for every potential pointer in this buffer

    _ptrscan_process_addr_fail:
    cm_del_vct(&hit_p_tree_nodes);

    return -1;
}



/*
 *  --- [POINTER SCAN | PUBLIC] ---
 */

//constructor
sc::ptrscan::ptrscan() noexcept
 : _scan(),
   depth_lvl(0),
   depth_lvl_vct_p(nullptr),
   state_flags(0b0) {

    int ret;


    //assert the pointer tree constructor succeeded
    if (this->tree.get_ctor_failed() == true) {
        this->_set_ctor_failed(true);
        return;
    }

    //zero-out vectors
    std::memset(&this->chains, 0, sizeof(this->chains));

    //assert the object table constructor succeeded
    if (this->obj_tbl.get_ctor_failed() == false) {
        this->_set_ctor_failed(true);
        return;
    }

    //initialise the chains vector
    ret = cm_new_vct(&this->chains, sizeof(sc::ptr_chain));
    if (ret != 0) {
        this->_set_ctor_failed(true);
        sc_errno = SC_ERR_CMORE;
        return;
    }

    return;
}


//destructor
sc::ptrscan::~ptrscan() noexcept {

    int ret_val = 0;

    const char * pathname;
    sc::ptr_chain * p_chain;


    //free chains
    for (int i = 0; i < this->chains.len; ++i) {
        p_chain = (sc::ptr_chain *) cm_vct_get_p(&this->chains, i);
        if (p_chain == nullptr) { sc_errno = SC_ERR_CMORE; continue; }
        p_chain->~ptr_chain();
    }

    //free chains vector
    _CTOR_VCT_DELETE_IF_INIT(this->chains);
}


//reset the scan state
[[nodiscard]] int sc::ptrscan::reset() noexcept {

    int ret;


    //get locks & check state
    ret = this->handle_entry(nullptr, nullptr, 0b0, 0b0);
    if (ret != 0) return -1;

    //assert a scan is not running
    if (this->_get_bits(sc::_scan_sf::running) > 0) {
        sc_errno = SC_ERR_STATE;
        return -1;
    }

    //reset the scan
    ret = this->do_reset();

    //release locks
    this->handle_exit(nullptr, nullptr);

    return (ret != 0) ? -1 : 0;
}


//dispatch a single pass over the target scan set
[[nodiscard]] int sc::ptrscan::dispatch_scan(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr,
    sc::worker_pool & w_pool,
    const cm_byte w_pool_flags) noexcept {

    int ret;
    cm_lst_node * tgt_area_node;


    //get locks & check state
    ret = handle_entry(&opts, &opts_ptr, sc::_scan_sf::running, 0b0);
    if (ret != 0) return -1;


    //reset if the target address has changed & a scan is in progress
    if ((opts_ptr.get_target_addr() != this->tree.get_root_node()->own_addr)
        && (this->_get_bits(sc::_scan_sf::scan_data) > 0)) {

        ret = this->do_reset();
        if (ret != 0) goto _ptrscan_dispatch_scan_fail_1;
    }


    //initialise root node if one isn't present
    if (this->depth_lvl == 0) {

        //map target address to an area node
        tgt_area_node = mc_get_area_by_addr(
            opts.get_map(), opts_ptr.get_target_addr(), nullptr);
        if (tgt_area_node == nullptr) {
            sc_errno = SC_ERR_OPT_BAD;
            goto _ptrscan_dispatch_scan_fail_1;
        }

        //initialise a new root node
        ret = this->tree.init_root_node(
            tgt_area_node, opts_ptr.get_target_addr(), 0x0);
        if (ret != 0) goto _ptrscan_dispatch_scan_fail_2;

        //advance current depth level
        ++this->depth_lvl;
    }


    //setup the worker pool
    ret = w_pool._setup(opts, opts_ptr, *this, w_pool_flags);
    if (ret != 0) goto _ptrscan_dispatch_scan_fail_2;

    //dispatch a single pass over target area set
    ret = w_pool._dispatch_run();
    if (ret != 0) goto _ptrscan_dispatch_scan_fail_2;

    //mark state as running a scan
    this->_set_bits(sc::_scan_sf::running);


    //release locks
    this->handle_exit(&opts, &opts_ptr);
    
    return 0;

    _ptrscan_dispatch_scan_fail_2:
    /* discard */ ret = this->do_reset();

    _ptrscan_dispatch_scan_fail_1:
    this->handle_exit(&opts, &opts_ptr);

    return -1;
}


//await a single pass over the scan set to complete (blocking)
[[nodiscard]] int
    sc::ptrscan::await_scan(sc::worker_pool & w_pool) noexcept {
    return this->do_await_scan(w_pool, true);
}


//await a single pass over the scan set to complete (non-blocking)
[[nodiscard]] int
    sc::ptrscan::try_await_scan(sc::worker_pool & w_pool) noexcept {
    return this->do_await_scan(w_pool, false);
}


//flatten the pointer tree into pointer chains
[[nodiscard]] int sc::ptrscan::flatten_tree(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr) noexcept {

    int ret;
    int ret_val = -1;

    cm_vct /* <off_t> */ off_stack;
    cm_vct /* <int> */ obj_tbl_idx_stack;
    cm_vct /* <off_t> */ obj_tbl_off_stack;
    const sc::_ptr_tree_node * p_root_node;

    const mc_vm_map * map;
    const sc::map_area_set * static_set;
    const cm_rbt * static_set_tree;


    //get locks & check state
    ret = this->handle_entry(
        &opts,
        &opts_ptr,
        sc::_scan_sf::scan_data | sc::_scan_sf::running,
        sc::_scan_sf::scan_data);
    if (ret != 0) return -1;

    //assert a map is provided
    map = opts.get_map();
    if (map == nullptr) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_flatten_tree_cleanup_1;
    }

    //assert a static set is provided
    static_set = opts_ptr.get_static_set();
    if (static_set == nullptr) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_flatten_tree_cleanup_1;
    }

    //read lock the static set
    ret = static_set->_lock_read();
    if (ret != 0) goto _ptrscan_flatten_tree_cleanup_1;

    //assert the static set is populated
    static_set_tree = &static_set->get_set();
    if (static_set_tree->is_init == false) {
        sc_errno = SC_ERR_OPT_EMPTY;
        goto _ptrscan_flatten_tree_cleanup_2;
    }


    //initialise the offset stack
    ret = cm_new_vct(&off_stack, sizeof(off_t));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        goto _ptrscan_flatten_tree_cleanup_2;
    }

    //initialise the object table offset stack
    ret = cm_new_vct(&obj_tbl_idx_stack, sizeof(int));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        goto _ptrscan_flatten_tree_cleanup_3;
    }

    //initialise the object table offset stack
    ret = cm_new_vct(&obj_tbl_off_stack, sizeof(off_t));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        goto _ptrscan_flatten_tree_cleanup_4;
    }

    //fetch the pointer tree root node
    p_root_node = this->tree.get_root_node();


    //start recursion
    ret = this->chain_recurse(
              map,
              *static_set_tree,
              off_stack,
              obj_tbl_idx_stack,
              obj_tbl_off_stack,
              *p_root_node,
              p_root_node->own_addr);
    if (ret != 0) goto _ptrscan_flatten_tree_cleanup_5;


    //set return to 0 to indicate success
    ret_val = 0;

    _ptrscan_flatten_tree_cleanup_5:
    cm_del_vct(&obj_tbl_off_stack);

    _ptrscan_flatten_tree_cleanup_4:
    cm_del_vct(&obj_tbl_idx_stack);

    _ptrscan_flatten_tree_cleanup_3:
    cm_del_vct(&off_stack);

    _ptrscan_flatten_tree_cleanup_2:
    static_set->_unlock();

    _ptrscan_flatten_tree_cleanup_1:
    this->handle_exit(&opts, &opts_ptr);

    return ret_val;
}


//verify pointer chains
[[nodiscard]] int sc::ptrscan::verify_chains(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr,
    const uintptr_t tgt_addr) noexcept {

    int ret;
    int ret_val = -1;

    mc_vm_map * map;
    const cm_vct /* <const mc_session *> */ * sess_vct;
    mc_session * sess;

    sc::ptr_chain * p_chain;


    //get locks & check state
    ret = this->handle_entry(
        &opts,
        &opts_ptr,
        sc::_scan_sf::running | sc::_ptrscan_sf::chains_data,
        sc::_ptrscan_sf::chains_data);
    if (ret != 0) return -1;

    //assert a map is provided
    map = opts.get_map();
    if (map == nullptr) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_verify_chains_fail_1;
    }

    //assert a session is provided
    sess_vct = &opts.get_sessions();
    if ((sess_vct->is_init == false) || (sess_vct->len == 0)) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_verify_chains_fail_1;
    }
    sess = *(mc_session **) cm_vct_get_p(sess_vct, 0);


    //for all pointer chains
    for (int i = 0; i < this->chains.len; ++i) {

        //get the next chain
        p_chain = (sc::ptr_chain *) cm_vct_get_p(&this->chains, i);

        //try to verify the chain & discard it on fail
        ret = p_chain->verify(map, sess, this->obj_tbl, tgt_addr);
        if (ret != 0) {
            sc_errno = 0;
            p_chain->~ptr_chain();
            cm_vct_rmv(&this->chains, i);
            --i;
        }
    }

    //cleanup
    _ptrscan_verify_chains_fail_1:
    this->handle_exit(&opts, &opts_ptr);

    return ret_val;
}

//update live data in existing chains
[[nodiscard]] int sc::ptrscan::update_live_chains(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr,
    const uintptr_t tgt_addr) noexcept {

    int ret;
    int ret_val = -1;

    mc_vm_map * map;
    const cm_vct /* <const mc_session *> */ * sess_vct;
    mc_session * sess;

    sc::ptr_chain * p_chain;


    //get locks & check state
    ret = this->handle_entry(
        &opts,
        &opts_ptr,
        sc::_scan_sf::running | sc::_ptrscan_sf::chains_data,
        sc::_ptrscan_sf::chains_data);
    if (ret != 0) return -1;

    //assert a map is provided
    map = opts.get_map();
    if (map == nullptr) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_verify_chains_fail_1;
    }

    //assert a session is provided
    sess_vct = &opts.get_sessions();
    if ((sess_vct->is_init == false) || (sess_vct->len == 0)) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _ptrscan_verify_chains_fail_1;
    }
    sess = *(mc_session **) cm_vct_get_p(sess_vct, 0);


    //for all pointer chains
    for (int i = 0; i < this->chains.len; ++i) {

        //get the next chain
        p_chain = (sc::ptr_chain *) cm_vct_get_p(&this->chains, i);

        //try to verify the chain & discard it on fail
        ret = p_chain->update_live_data(map, sess, this->obj_tbl);
        if (ret != 0) {
            sc_errno = 0;
            p_chain->~ptr_chain();
            cm_vct_rmv(&this->chains, i);
            --i;
        }
    }

    //cleanup
    _ptrscan_verify_chains_fail_1:
    this->handle_exit(&opts, &opts_ptr);

    return ret_val;
}
