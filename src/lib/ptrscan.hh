#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"



namespace sc {

class _ptrscan_tree_node {

    _SC_DBG_PRIVATE:
        //[attributes]
        //children of this code
        cm_lst /* <sc::_ptrscan_tree_node> */ children;

    public:
        //[attributes]
        const int id;
        const cm_lst_node * const area_node;

        const uintptr_t own_addr;
        const uintptr_t ptr_addr;

        const sc::_ptrscan_tree_node * parent;
    
        //[methods]
        //ctor & dtor
        _ptrscan_tree_node(const int id,
                           const cm_lst_node * area_node,
                           const uintptr_t own_addr,
                           const uintptr_t ptr_addr,
                           const _ptrscan_tree_node * parent)
         : id(id),
           area_node(area_node),
           own_addr(own_addr),
           ptr_addr(ptr_addr),
           parent(parent) {}
        ~_ptrscan_tree_node();

        //connect a child
        void connect_child(
            const _ptrscan_tree_node * child_node) noexcept;

        //free children
        void clear() noexcept;

        //getters & setters
        [[nodiscard]] const cm_lst /* <sc::_ptrscan_tree_node> */ &
            get_children() const noexcept;
        [[nodiscard]] bool has_children() const noexcept;
};


class _ptrscan_tree {

    _SC_DBG_PRIVATE:
        //[attributes]
        pthread_mutex_t write_lock;
        int next_id;

        //nodes at each level of the pointer tree
        cm_vct /* cm_vct <sc::_ptrscan_tree_node *> */ depth_levels;
        sc::_ptrscan_tree_node * root_node;

    public:
        //[methods]
        //ctor & dtor
        _ptrscan_tree(int max_depth);
        ~_ptrscan_tree();

        //reset
        void reset() noexcept;

        /*
         *  NOTE: Not inheriting from `_lockable` and using a rw lock 
         *        because we want other threads to be able to continue 
         *        reading while the write is happening.
         */

        //lock tree for writing

        //add a node
        void add_node(sc::_ptrscan_tree_node * parent_node,
                      const cm_lst_node * area_node,
                      const int depth_level,
                      const uintptr_t own_addr,
                      const uintptr_t ptr_addr) noexcept;


        //getters & setters
        [[nodiscard]] pthread_mutex_t & get_write_mutex() noexcept;

        [[nodiscard]] cm_vct /* <sc::_ptrscan_tree_node *> */ &
            get_depth_level_vct(const int level) const noexcept;

        [[nodiscard]] const sc::_ptrscan_tree_node *
            get_root_node() const noexcept;
};


}; //namespace sc
