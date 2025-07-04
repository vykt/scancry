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



namespace sc {

class _ptrscan_tree_node {

    _SC_DBG_PRIVATE:
        //[attributes]
        //children of this code
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
        _ptrscan_tree_node(const int id,
                           const cm_lst_node * area_node,
                           const uintptr_t own_addr,
                           const uintptr_t ptr_addr,
                           const std::shared_ptr<_ptrscan_tree_node> parent)
         : id(id),
           area_node(area_node),
           own_addr(own_addr),
           ptr_addr(ptr_addr),
           parent(parent) {}

        //connect a child
        void connect_child(
            const std::shared_ptr<_ptrscan_tree_node> child_node);

        //free children
        void clear();

        //getters & setters
        [[nodiscard]] const std::list<std::shared_ptr<_ptrscan_tree_node>>
            & get_children() const noexcept;
        [[nodiscard]] bool has_children() const noexcept;
};


class _ptrscan_tree {

    _SC_DBG_PRIVATE:
        //[attributes]
        int next_id;
        pthread_mutex_t write_mutex;

        //2D vector is desirable here, we need fast iteration
        std::vector<
            std::vector<std::shared_ptr<_ptrscan_tree_node>>> depth_levels;
        std::shared_ptr<_ptrscan_tree_node> root_node;

    public:
        //[methods]
        //ctor
        _ptrscan_tree(int max_depth);
        ~_ptrscan_tree() { pthread_mutex_destroy(&write_mutex); };

        //tree modifiers
        void add_node(std::shared_ptr<_ptrscan_tree_node> node,
                      const cm_lst_node * area_node,
                      const int depth_level,
                      const uintptr_t own_addr,
                      const uintptr_t ptr_addr);
        void reset();


        //getters & setters
        [[nodiscard]] pthread_mutex_t & get_write_mutex() noexcept;
        [[nodiscard]] const std::vector<std::shared_ptr<_ptrscan_tree_node>>
            & get_depth_level_vct(int level) const noexcept;
        [[nodiscard]] std::vector<
            std::vector<std::shared_ptr<_ptrscan_tree_node>>>
                ::const_reverse_iterator
                    get_depth_level_crbegin() const noexcept;
        [[nodiscard]] std::vector<
            std::vector<std::shared_ptr<_ptrscan_tree_node>>>
                ::const_reverse_iterator
                    get_depth_level_crend() const noexcept;
        [[nodiscard]] const std::shared_ptr<
            _ptrscan_tree_node> get_root_node() const;
};


}; //namespace sc
