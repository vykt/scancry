//C++ builtin
#include <new>

//C standard library
#include <cstring>
#ifdef SC_TRACE
#include <cstdio>
#endif

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
 *  --- [POINTER CHAIN | PRIVATE] ---
 */

void sc::ptr_chain::do_copy(const sc::ptr_chain & p_chain) noexcept {

    int ret;


    //call parent copy assignment operators
    _ctor_failable::operator=(p_chain);

    //copy object index
    this->_obj_idx = p_chain._get_obj_idx();

    //copy static status
    this->is_static = p_chain.get_is_static();

    //copy map info
    if (p_chain.in_map() == true) {
        this->is_in_map = true;
        this->obj_node  = p_chain.get_obj_node();
    } else {
        this->is_in_map = false;
        this->pathname  = p_chain.get_pathname();
    }

    //copy offsets
    _CTOR_VCT_COPY_IF_INIT(this->offsets, p_chain.get_offsets());

    return;
}


//object index getter
[[nodiscard]] uint32_t sc::ptr_chain::_get_obj_idx() const noexcept {
    return this->_obj_idx;
}



/*
 *  --- [POINTER CHAIN | PUBLIC] ---
 */

//constructor
sc::ptr_chain::ptr_chain(
    const uint32_t _obj_idx,
    const bool is_static,
    const cm_lst_node * /* one of (set A) */ obj_node,
    const char * /* one of (set A) */ pathname,
    const cm_vct /* <off_t> */ & offsets) noexcept
 : _ctor_failable(),
   _obj_idx(_obj_idx),
   is_static(is_static) {

    int ret;

    //setup an object pointer or pathname fallback
    if (obj_node != nullptr) {
        this->is_in_map = true;
        this->obj_node = obj_node;
        
    } else {
        this->is_in_map = false;
        this->pathname = pathname;
    }

    //copy offsets
    _CTOR_VCT_COPY_IF_INIT(this->offsets, offsets);

    return;
}


//copy constructor
sc::ptr_chain::ptr_chain(const sc::ptr_chain & p_chain) noexcept
 : _ctor_failable(),
   obj_node(nullptr) {

    //zero out the offsets vector
    std::memset(&this->offsets, 0, sizeof(this->offsets));

    this->do_copy(p_chain);
    return;
}


//destructor
sc::ptr_chain::~ptr_chain() noexcept {

    //destroy offsets
    _CTOR_VCT_DELETE_IF_INIT(this->offsets);

    return;
}


//copy assignment operator
sc::ptr_chain & sc::ptr_chain::operator=(
    const sc::ptr_chain & p_chain) noexcept {

    if (this != &p_chain) this->do_copy(p_chain);
    return *this;
}


//determine if chain's starting reference object is in the current map
[[nodiscard]] bool sc::ptr_chain::in_map() const noexcept {
    return this->is_in_map;
}


//getters
[[nodiscard]] bool sc::ptr_chain::get_is_static() const noexcept {
    return this->is_static;
}

[[nodiscard]] const cm_lst_node *
    sc::ptr_chain::get_obj_node() const noexcept {
    return this->obj_node;
}

[[nodiscard]] const char * sc::ptr_chain::get_pathname() const noexcept {
    return this->pathname;
}

[[nodiscard]] const cm_vct & sc::ptr_chain::get_offsets() const noexcept {
    return this->offsets;
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
        tmp_p_tree_node = _GET_NODE_PTR_TREE_NODE(tmp_node);
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
    ret = cm_vct_apd(lvl_vct, this->root_node);
    if (ret != 0) goto _ptr_tree_init_root_node_fail;

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

    cm_byte placeholder[sizeof(cm_vct)] = {0};
    cm_vct * lvl_vct;


    //acquire a write lock
    _AWAIT_WRITE(-1);

    //allocate space for a new tree level vector
    ret = cm_vct_apd(&this->depth_lvls, placeholder);
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        goto _ptr_tree_inc_depth_lvl_fail;
    }

    //get a pointer to the new allocation
    lvl_vct = (cm_vct *) cm_vct_get_p(&this->depth_lvls, -1);
    if (lvl_vct == nullptr) {
        sc_errno = SC_ERR_CMORE;
        goto _ptr_tree_inc_depth_lvl_fail;
    }

    //initialise the new tree level vector
    ret = cm_new_vct(lvl_vct, sizeof(sc::_ptr_tree_node *));
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
    ret = cm_vct_apd(lvl_vct, &p_tree_node);
    if (ret != 0) {
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
    _CTOR_VCT_DELETE_IF_INIT(this->ser_pathnames);

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

    //reset high-level state
    this->state_flags = 0b0;

    //unlock
    return 0;
}


//perform a single pass over the target scan set
[[nodiscard]] int sc::ptrscan::do_run_scan(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr,
    sc::worker_pool & w_pool,
    const cm_byte w_pool_flags,
    const bool do_block) noexcept {

    int ret;
    cm_lst_node * tgt_area_node;


    //assert a scan isn't currently in progress
    if (this->state_flags & sc::_ptrscan_flag::running) {
        sc_errno = SC_ERR_SCAN_BUSY;
        return -1;
    }

    //reset if the target address has changed & a scan is in progress
    if ((opts_ptr.get_target_addr() != this->tree.get_root_node()->own_addr)
        && ((this->state_flags & sc::_ptrscan_flag::inprog) > 0)) {

        ret = this->do_reset();
        if (ret != 0) goto _ptrscan_scan_fail_1; 
    }

    //initialise root node if one isn't present
    if (this->depth_lvl == 0) {

        //map target address to an area node
        tgt_area_node = mc_get_area_by_addr(
            opts.get_map(), opts_ptr.get_target_addr(), nullptr);
        if (tgt_area_node == nullptr) {
            sc_errno = SC_ERR_OPT_BAD;
            goto _ptrscan_scan_fail_1;
        }

        //initialise a new root node
        ret = this->tree.init_root_node(
            tgt_area_node, opts_ptr.get_target_addr(), 0x0);
        if (ret != 0) goto _ptrscan_scan_fail_2;

        //advance current depth level
        ++this->depth_lvl;
    }

    //setup the worker pool
    ret = w_pool._setup(opts, opts_ptr, *this, w_pool_flags);
    if (ret != 0) goto _ptrscan_scan_fail_2;

    //perform a single pass over target area set
    if (do_block == true) ret = w_pool._single_run();
    else ret = w_pool._try_single_run();
    if (ret != 0) goto _ptrscan_scan_fail_2;
    this->state_flags |= sc::_ptrscan_flag::running;
    
    return 0;

    _ptrscan_scan_fail_2:
    /* discard */ ret = this->do_reset();

    _ptrscan_scan_fail_1:
    return -1;
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
            ret = cm_vct_apd(&hit_p_tree_nodes, &p_tree_node);
            if (__builtin_expect((ret != 0), 0)) {
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
 : _scan(), _ctor_failable(),
   depth_lvl(0),
   depth_lvl_vct_p(nullptr),
   state_flags(0b0) {

    //assert the pointer tree constructor succeeded
    if (this->tree.get_ctor_failed() == true) {
        this->_set_ctor_failed(true);
        return;
    }

    //zero-out vectors
    std::memset(&this->ser_pathnames, 0, sizeof(this->ser_pathnames));
    std::memset(&this->chains, 0, sizeof(this->chains));

    return;
}


//destructor
sc::ptrscan::~ptrscan() noexcept {

    int ret_val = 0;
    sc::ptr_chain * p_chain;


    //free pathnames vector
    _CTOR_VCT_DELETE_IF_INIT(this->ser_pathnames);

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


    //acquire a write lock
    _LOCK_WRITE(-1);

    //reset & unlock
    ret = this->do_reset();
    _UNLOCK
    return (ret != 0) ? -1 : 0;
}


//perform a single pass over the target scan set
[[nodiscard]] int sc::ptrscan::run_scan(
    const sc::opt & opts,
    const sc::opt_ptrscan & opts_ptr,
    sc::worker_pool & w_pool,
    const cm_byte w_pool_flags) noexcept {

    int ret;

    cm_lst_node * tgt_area_node;


    //acquire a write lock
    _LOCK_WRITE(-1);

    //reset if the target address has changed & a scan is in progress
    if ((opts_ptr.get_target_addr() != this->tree.get_root_node()->own_addr)
        && ((this->state_flags & sc::_ptrscan_flag::inprog) > 0)) {

        ret = this->do_reset();
        if (ret != 0) goto _ptrscan_scan_fail_1; 
    }

    //initialise root node if one isn't present
    if (this->depth_lvl == 0) {

        //map target address to an area node
        tgt_area_node = mc_get_area_by_addr(
            opts.get_map(), opts_ptr.get_target_addr(), nullptr);
        if (tgt_area_node == nullptr) {
            sc_errno = SC_ERR_OPT_BAD;
            goto _ptrscan_scan_fail_1;
        }

        //initialise a new root node
        ret = this->tree.init_root_node(
            tgt_area_node, opts_ptr.get_target_addr(), 0x0);
        if (ret != 0) goto _ptrscan_scan_fail_2;

        //advance current depth level
        ++this->depth_lvl;
    }

    //setup the worker pool
    ret = w_pool._setup(opts, opts_ptr, *this, w_pool_flags);
    if (ret != 0) goto _ptrscan_scan_fail_2;

    //perform a single pass over target area set
    ret = w_pool._single_run();
    if (ret != 0) goto _ptrscan_scan_fail_2;
    this->state_flags |= sc::_ptrscan_flag::running;
    
    //unlock
    _UNLOCK;
    return 0;

    _ptrscan_scan_fail_2:
    /* discard */ ret = this->do_reset();

    _ptrscan_scan_fail_1:
    _UNLOCK
    return -1;
}

