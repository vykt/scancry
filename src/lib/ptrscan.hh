#pragma once

//standard template library
#include <vector>
#include <list>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"



/*
 *  NOTE: Defining the pointer scan tree, originally declared in
 *        `scancry,h`. This is done to reduce clutter in the main header.
 */

namespace sc {

class _ptrscan_tree_node {

    _SC_DBG_PRIVATE:
        //[attributes]
        std::list<_ptrscan_tree_node> children;

    public:
        //[attributes]
        const int id;
        const cm_lst_node * const area_node;

        const uintptr_t own_addr;
        const uintptr_t ptr_addr;

        const std::shared_ptr<_ptrscan_tree_node> parent;
    
        //[methods]
        //ctor
        _ptrscan_tree_node(const int id, const cm_lst_node * area_node,
                           const uintptr_t own_addr, const uintptr_t ptr_addr,
                           const std::shared_ptr<_ptrscan_tree_node> parent);

        //getters & setters
        const std::list<_ptrscan_tree_node> & get_children() const;
};


class _ptrscan_tree {

    _SC_DBG_PRIVATE:
        //[attributes]
        int next_id;
        pthread_mutex_t write_mutex;

        std::vector<std::list<std::shared_ptr<_ptrscan_tree_node>>> depth_levels;
        const std::shared_ptr<_ptrscan_tree_node> root_node;

    public:
        //[methods]
        //ctor
        _ptrscan_tree() : next_id(0) {};
        std::optional<int> add_node(_ptrscan_tree_node & node,
                                    const int level, const cm_lst_node * area_node,
                                    const uintptr_t own_addr, const uintptr_t ptr_addr);
        //getters & setters
        std::vector<std::list<std::shared_ptr<_ptrscan_tree_node>>> & get_depth_levels() const;
        const std::shared_ptr<_ptrscan_tree_node> get_root_node() const;
};

}; //namespace sc
