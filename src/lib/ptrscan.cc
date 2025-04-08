//standard template library
#include <optional>
#include <memory>
#include <type_traits>
#include <vector>
#include <list>
#include <string>
#include <exception>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "ptrscan.hh"
#include "error.hh"



/*
 *  --- [TREE NODE | PUBLIC] ---
 */

//connect a node as a child of another node
void sc::_ptrscan_tree_node::connect_child(
    const std::shared_ptr<_ptrscan_tree_node> child_node) {

    this->children.push_back(child_node);
    return;
}


void sc::_ptrscan_tree_node::clear() {
    this->children.clear();
}


//get a reference to the children of a node
const std::list<std::shared_ptr<sc::_ptrscan_tree_node>>
    & sc::_ptrscan_tree_node::get_children() const noexcept {

    return this->children;
}


bool sc::_ptrscan_tree_node::has_children() const noexcept {
    return this->children.empty();
}



/*
 *  --- [TREE | PRIVATE] ---
 */

void sc::_ptrscan_tree::add_node(std::shared_ptr<sc::_ptrscan_tree_node> node,
                                 const cm_lst_node * area_node,
                                 const int depth_level,
                                 const uintptr_t own_addr,
                                 const uintptr_t ptr_addr) {

    //create new node
    std::shared_ptr<sc::_ptrscan_tree_node> new_node
        = std::make_shared<sc::_ptrscan_tree_node>(this->next_id, area_node,
                                                   own_addr, ptr_addr, node);

    //add new node to its parent
    node->connect_child(new_node);

    //add new node to its depth level list
    this->depth_levels[depth_level].push_back(new_node);

    return;
}


void sc::_ptrscan_tree::reset() {

    //free tree node depth layers in bottom-up order
    for (auto iter = this->depth_levels.rbegin();
         iter != this->depth_levels.rend(); ++iter) {

        iter->clear();
    }

    //reset variables
    this->depth_levels.clear();
    this->root_node.reset();
    this->next_id = 0;

    return;
}


pthread_mutex_t & sc::_ptrscan_tree::get_write_mutex() noexcept {
    return this->write_mutex;
}


const std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> &
    sc::_ptrscan_tree::get_depth_level_vct(int level) const noexcept {

    return this->depth_levels[level];
}


std::vector<
    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>>>
        ::const_reverse_iterator
            sc::_ptrscan_tree::get_depth_level_crbegin() const noexcept {

    return this->depth_levels.crbegin();
}


std::vector<
    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>>>
        ::const_reverse_iterator
            sc::_ptrscan_tree::get_depth_level_crend() const noexcept {

    return this->depth_levels.crend();
}


const std::shared_ptr<sc::_ptrscan_tree_node>
    sc::_ptrscan_tree::get_root_node() const {

    return this->root_node;
}



/*
 *  --- [POINTER SCANNER CHAIN | PUBLIC] ---
 */

sc::ptrscan_chain::ptrscan_chain(const cm_lst_node * area_node,
                                 const std::vector<off_t> & offsets)
                                  : area_node(area_node),
                                    offsets(offsets) {}



/*
 *  --- [POINTER SCANNER | PRIVATE] ---
 */

void sc::ptrscan::add_node(std::shared_ptr<sc::_ptrscan_tree_node> parent_node,
                           const cm_lst_node * area_node,
                           const uintptr_t own_addr, const uintptr_t ptr_addr) {

    //create node inside the ptrscan tree
    this->tree_p->add_node(
        parent_node, area_node, this->cur_depth_level, own_addr, ptr_addr);

    return;
}



std::optional<int> sc::ptrscan::flatten_tree() {

    /*
     *  NOTE: To extract individual pointer chains from the pointer scan
     *        tree, each leaf node is followed up until it reaches the
     *        root node. Only the starting area is recorded.
     *
     */

    off_t offset;
    std::shared_ptr<sc::_ptrscan_tree_node> child, parent;


    //for every depth level
    for (auto iter = ++this->tree_p->get_depth_level_crbegin();
         iter != --this->tree_p->get_depth_level_crend(); ++iter) {

        //for every node at this depth level
        for (auto inner_iter = iter->cbegin();
             inner_iter != iter->cend(); ++inner_iter) {

            //fetch node
            const std::shared_ptr<sc::_ptrscan_tree_node> node = *inner_iter;

            //skip this node if it is not a leaf node
            if (node->has_children() == true) continue;

            /* While it is tempting to recurse here, it is slow. */

            //for each tree edge from this leaf to the root, add a chain entry
            std::vector<off_t> offsets;
            child = node;
            do {
                //fetch parent
                parent = node->parent;

                //add offset
                offset = parent->own_addr - child->ptr_addr;
                offsets.push_back(offset);

            } while (parent != this->tree_p->get_root_node());

            //add chain
            this->chains.emplace_back(ptrscan_chain(node->area_node, offsets));

        } //end for every node at this depth level

    } //end for every depth level

    return 0;
}



/*
 *  --- [POINTER SCANNER | INTERFACE] ---
 */

//temporary storage for a potential new node in the ptrscan tree
struct _potential_node {

    //[attributes]
    const uintptr_t own_addr;
    const uintptr_t ptr_addr;
    const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node;
    const cm_lst_node * area_node;

    //[methods]
    _potential_node(
            const uintptr_t own_addr,
            const uintptr_t ptr_addr,
            const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node,
            const cm_lst_node * area_node)
     : own_addr(own_addr), ptr_addr(ptr_addr),
       parent_tree_node(parent_tree_node), area_node(area_node) {}
};


//process a single address from a worker thread
std::optional<int> sc::ptrscan::_process_addr(
                                        const struct _scan_arg arg,
                                        const opt * const opts,
                                        const _opt_scan * const opts_scan) {

    /*
     *  NOTE: This function is called for each byte of memory that is
     *        scanned; it is imperative that the most common fail cases
     *        are considered first.
     */

    /*
     *  NOTE: `_manage_scan()` already verified the type of `opts_scan`.
     *        It's safe to cast directly.
     */

    //fetch ptrscan options
    const opt_ptrscan * const opts_ptrscan
        = (const opt_ptrscan * const) opts_scan;

    //if not on an alignment boundary, return
    if ((arg.area_off % opts_ptrscan->get_alignment().value()) != 0) return 0;

    //if not enough space is left in the buffer to hold a pointer
    /* FIXME [should be impossible, remove?]
    off_t required_left = (opts.addr_width == sc::AW64) ? 8 : 4;
    if (arg.buf_left < required_left) return 0;
    */

    /*
     *  `const` qualifier is discarded; no better way to do this the way
     *  things are currently organised.
     */

    //re-cache the depth level vector at the start of every area
    if (arg.area_off == 0)
        this->cache.depth_level_vct =
            (std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> *)
            &this->tree_p->get_depth_level_vct(this->cur_depth_level);

    /*
     *  NOTE: `new_nodes` stores nodes that will be added by the end of
     *        this call (if any). In case a smart scan is being performed,
     *        this vector will be reset each time a new minimum offset is
     *        found.
     *
     *  TODO: Determine whether more than a single minimum is possible
     *        during a smart scan. For now assume that it is.
     */

    //setup new node container
    std::vector<struct _potential_node> new_nodes;
    off_t min_obj_sz = opts_ptrscan->get_max_obj_sz().value();

    //get potential pointer value
    uintptr_t potential_ptr;
    if (opts->addr_width == sc::AW32)
        potential_ptr = *((uint32_t *) arg.cur_byte);
    if (opts->addr_width == sc::AW64)
        potential_ptr = *((uint64_t *) arg.cur_byte);


    /*
     *  NOTE: Iteration over each potential parent node begins now.
     */

    //for every ptrscan tree node at this depth
    for (auto level_iter = this->cache.depth_level_vct->begin();
         level_iter != this->cache.depth_level_vct->end(); ++level_iter) {

        //get the current node from the iterator
        const std::shared_ptr<sc::_ptrscan_tree_node> & now_node
            = *level_iter;

        //if this potential pointer falls outside the range of this
        //node, skip it
        if (potential_ptr <
                (now_node->own_addr - opts_ptrscan->get_max_obj_sz().value())
            || potential_ptr > (now_node->own_addr)) continue;


        //else this is a match

        //check the offset is correct, if one applies
        auto presets = opts_ptrscan->get_preset_offsets();
        if (presets.has_value() && presets->size() <= this->cur_depth_level)
            if (potential_ptr != (now_node->own_addr
                    - (*presets)[this->cur_depth_level])) continue;
        
        //if this is a smart scan, manipulate the new node container
        if (opts_ptrscan->get_smart_scan() == true) {

            //if greater than current minimum, ignore this match
            if ((now_node->own_addr - potential_ptr)
                 > min_obj_sz) continue;

            //if smaller than current minimum, reset the node container
            if ((now_node->own_addr - potential_ptr)
                 < min_obj_sz) new_nodes.clear();

        }

        //add this node to the new node container
        new_nodes.push_back(_potential_node(arg.addr, potential_ptr,
                                            now_node, arg.area_node));
        
    } //end for every ptrscan tree node at this depth


    /*
     *  NOTE: Adding nodes all at once at tne end also minimises
     *        mutex thrashing.
     */

    //acquire the mutex
    int ret = pthread_mutex_lock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    //add every new node to the tree
    for (auto new_iter = new_nodes.begin();
         new_iter != new_nodes.end(); ++new_iter) {

        //add the new node to the tree
        this->tree_p->add_node(new_iter->parent_tree_node,
                               new_iter->area_node,
                               this->cur_depth_level,
                               new_iter->own_addr,
                               new_iter->ptr_addr);
    }

    //release the mutex
    ret = pthread_mutex_unlock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    return 0;
}


std::optional<int> sc::ptrscan::_manage_scan(
                                        sc::worker_mngr & w_mngr,
                                        const opt * const opts,
                                        const _opt_scan * const opts_scan) {

    std::optional<int> ret;


    //reset the pointer scan
    this->reset();

    //fetch ptrscan options
    opt_ptrscan * opts_ptrscan;
    try {
        const opt_ptrscan * const _opts_ptrscan
            = dynamic_cast<const opt_ptrscan * const>(opts_scan);
        opts_ptrscan = (opt_ptrscan *) _opts_ptrscan;
    } catch (std::bad_cast) {
        sc_errno = SC_ERR_OPT_TYPE;
        return std::nullopt;
    }

    //check all necessary options have been set
    if (opts_ptrscan->get_target_addr().has_value() == false
        || opts_ptrscan->get_alignment().has_value() == false
        || opts_ptrscan->get_max_obj_sz().has_value() == false
        || opts_ptrscan->get_max_depth().has_value() == false) {

        sc_errno = SC_ERR_OPT_MISSING;
        return std::nullopt;
    }

    //for every depth level
    for (int i = 0; i < opts_ptrscan->get_max_depth().value(); ++i) {

        //scan the selected address space once
        ret = w_mngr._single_run();
        if (ret.has_value() == false) return std::nullopt;

        //increment current depth
        ++this->cur_depth_level;
    }

    //flatten the tree
    ret = this->flatten_tree();
    if (ret.has_value() == false) return std::nullopt;

    return 0;
}



/*
 *  --- [POINTER SCANNER | PUBLIC] ---
 */

sc::ptrscan::ptrscan()
 : _scan(),
   cur_depth_level(0),
   cache({0}) {}


void sc::ptrscan::reset() {

    //reset variables
    this->tree_p->reset();
    this->cur_depth_level = 0;
    this->chains.clear();

    return;
}
