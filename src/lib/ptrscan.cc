//standard template library
#include <optional>
#include <memory>
#include <type_traits>
#include <vector>
#include <list>
#include <string>
#include <stdexcept>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "ptrscan.hh"
#include "error.hh"


/*
 *  FIXME: Actually handle exceptions.
 */


/*
 *  --- [TREE NODE | PUBLIC] ---
 */

const std::list<std::shared_ptr<sc::_ptrscan_tree_node>> & sc::_ptrscan_tree_node::get_children() const {
    return this->children;
}


void sc::_ptrscan_tree_node::add_child(std::shared_ptr<_ptrscan_tree_node> child_node) {
    this->children.push_back(child_node);
    return;
} 


/*
 *  --- [TREE | PRIVATE] ---
 */

//RAII for pthread_mutex_t
sc::_ptrscan_tree::_ptrscan_tree() : next_id(0), now_depth_level(0) {

    int ret;

    ret = pthread_mutex_init(&this->write_mutex, nullptr);
    if (ret != 0) {
        throw std::runtime_error(std::string(__FUNCTION__)
                                 + ": ptrscan tree's write_mutex initialisation failed.");
    }
}


/*
 *  FIXME: If pthread_mutex_destroy fails, memory is silently leaked.
 *         Consider a better approach, and whether the better approach
 *         is worth forgoing RAII for.
 */
sc::_ptrscan_tree::~_ptrscan_tree() {

    pthread_mutex_destroy(&this->write_mutex);
    return;
}


void sc::_ptrscan_tree::add_node(std::shared_ptr<sc::_ptrscan_tree_node> node,
                                 const cm_lst_node * area_node,
                                 const uintptr_t own_addr, const uintptr_t ptr_addr) {

    //create new node
    std::shared_ptr<sc::_ptrscan_tree_node> new_node
        = std::make_shared<sc::_ptrscan_tree_node>(this->next_id, area_node,
                                                   own_addr, ptr_addr, node);

    //add new node to its parent
    node->add_child(new_node);

    //add new node to its depth level list
    this->depth_levels[this->now_depth_level].push_back(new_node);

    return;
}


void sc::_ptrscan_tree::inc_depth() {
    ++this->now_depth_level;
    return;
}


const std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> & sc::_ptrscan_tree::get_depth_level_vct(int level) const noexcept {
    return this->depth_levels[level];
}


const std::shared_ptr<sc::_ptrscan_tree_node> sc::_ptrscan_tree::get_root_node() const {
    return this->root_node;
}


int sc::_ptrscan_tree::get_now_depth_level() const noexcept {
    return this->now_depth_level;
}



/*
 *  --- [POINTER SCANNER | PRIVATE] ---
 */
void sc::ptrscan::_add_node(std::shared_ptr<sc::_ptrscan_tree_node> parent_node,
                            const cm_lst_node * area_node, const uintptr_t own_addr, const uintptr_t ptr_addr) {

    //create node inside the ptrscan tree
    this->tree_p->add_node(parent_node, area_node, own_addr, ptr_addr);

    return;
}



/*
 *  --- [POINTER SCANNER | PUBLIC] ---
 */

//
struct _potential_node {

    //[attributes]
    const uintptr_t own_addr;
    const uintptr_t ptr_addr;
    const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node;
    const cm_lst_node * area_node;

    //[methods]
    _potential_node(const uintptr_t own_addr, const uintptr_t ptr_addr,
                    const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node,
                    const cm_lst_node * area_node)
     : own_addr(own_addr), ptr_addr(ptr_addr),
       parent_tree_node(parent_tree_node), area_node(area_node) {}
};


//process a single address from a worker thread
void sc::ptrscan::process_addr(const struct _scan_arg arg, const void * const arg_custom,
                               const opt & opts, const void * const opts_custom) {

    /*
     *  NOTE: Considering this function is called for each byte of memory
     *        that is scanned, it is imperative that the most common fail
     *        cases are executed first.
     */

    //fetch ptrscan options (the C way <3)
    opt_ptrscan & opts_ptrscan = *((opt_ptrscan *) opts_custom);
    struct _arg_ptrscan & arg_ptrscan = *((struct _arg_ptrscan *) arg_custom);

    //if not on an alignment boundary, return
    if ((arg.area_off % *opts_ptrscan.get_alignment()) != 0) return;

    //if not enough space is left in the buffer to hold a pointer
    off_t required_left = (opts.addr_width == sc::AW64) ? 8 : 4;
    if (arg.buf_left < required_left) return;

    //re-cache the depth level vector at the start of every area
    if (arg.area_off == 0)
        arg_ptrscan.depth_level_vec
            = this->tree_p->get_depth_level_vct(this->tree_p->get_now_depth_level());

    /*
     *  NOTE: `new_nodes` stores nodes that will be added by the end of
     *        this call (if any). In case a smart scan is being performed,
     *        this vector will be reset each time a new minimum offset is
     *        found.
     */

    //setup new node container
    std::vector<struct _potential_node> new_nodes;
    off_t min_obj_sz = *opts_ptrscan.get_max_obj_sz();

    //get potential pointer value
    uintptr_t potential_ptr;
    if (opts.addr_width == sc::AW32) potential_ptr = *((uint32_t *) arg.cur_byte);
    if (opts.addr_width == sc::AW64) potential_ptr = *((uint64_t *) arg.cur_byte);


    /*
     *  NOTE: Iteration over each potential parent node begins now.
     */

    //for every ptrscan tree node at this depth
    for (auto level_iter = arg_ptrscan.depth_level_vec.begin();
         level_iter != arg_ptrscan.depth_level_vec.end(); ++level_iter) {

        //get the current node from the iterator
        const std::shared_ptr<sc::_ptrscan_tree_node> & now_node = *level_iter;

        //if this potential pointer falls outside the range of this node, skip it
        if (potential_ptr < (now_node->own_addr - *opts_ptrscan.get_max_obj_sz())
            || potential_ptr > (now_node->own_addr)) continue;


        //else this is a match

        //check the offset is correct, if one applies
        if (arg_ptrscan.cur_offset.has_value())
            if (potential_ptr != (now_node->own_addr - *arg_ptrscan.cur_offset)) continue;

        //if this is a smart scan, manipulate the new node container
        if (opts_ptrscan.get_smart_scan() == true) {

            //if greater than current minimum, ignore this match
            if ((now_node->own_addr - potential_ptr) > min_obj_sz) continue;

            //if smaller than current minimum, reset the node container
            if ((now_node->own_addr - potential_ptr) < min_obj_sz) new_nodes.clear();

        }

        //add this node to the new node container
        new_nodes.push_back(_potential_node(arg.addr, potential_ptr,
                                            now_node, arg.area_node));
        
    } //end for every ptrscan tree node at this depth


    /*
     *  NOTE: Adding nodes all at once at tne end also minimises mutex thrashing.
     */

    //acquire the mutex
    int ret = pthread_mutex_lock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        throw std::runtime_error(std::string(__FUNCTION__)
                                 + ": Failed to acquire the ptrscan tree mutex.");
    }

    //add every new node to the tree
    for (auto new_iter = new_nodes.begin(); new_iter != new_nodes.end(); ++new_iter) {

        //add the new node to the tree
        this->tree_p->add_node(new_iter->parent_tree_node, new_iter->area_node,
                               new_iter->own_addr, new_iter->ptr_addr);
    }

    //release the mutex
    ret = pthread_mutex_unlock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        throw std::runtime_error(std::string(__FUNCTION__)
                                 + ": Failed to release the ptrscan tree mutex.");
    }

    return;
}
