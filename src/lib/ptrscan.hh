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
        std::list<std::shared_ptr<_ptrscan_tree_node>> children;

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
                           const std::shared_ptr<_ptrscan_tree_node> parent)
         : id(id), area_node(area_node),
           own_addr(own_addr), ptr_addr(ptr_addr), parent(parent) {}

        void add_child(std::shared_ptr<_ptrscan_tree_node> child_node);

        //getters & setters
        const std::list<std::shared_ptr<_ptrscan_tree_node>> & get_children() const;
};


class _ptrscan_tree {

    _SC_DBG_PRIVATE:
        //[attributes]
        int next_id;
        int now_depth_level;
        pthread_mutex_t write_mutex;

        //2D vector is desirable here, we need fast iteration
        std::vector<std::vector<std::shared_ptr<_ptrscan_tree_node>>> depth_levels;
        const std::shared_ptr<_ptrscan_tree_node> root_node;

    public:
        //[methods]
        //ctor
        _ptrscan_tree();
        ~_ptrscan_tree();

        //tree modifiers
        void add_node(std::shared_ptr<_ptrscan_tree_node> node,
                      const cm_lst_node * area_node,
                      const uintptr_t own_addr, const uintptr_t ptr_addr);
        void inc_depth();

        //getters & setters
        int get_now_depth_level() const noexcept;
        pthread_mutex_t & get_write_mutex() noexcept;
        const std::vector<std::shared_ptr<_ptrscan_tree_node>> & get_depth_level_vct(int level) const noexcept;
        const std::shared_ptr<_ptrscan_tree_node> get_root_node() const;
};

}; //namespace sc
