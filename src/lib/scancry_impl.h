#ifndef SCANCRY_IMPL_H
#define SCANCRY_IMPL_H

#ifdef __cplusplus
#include <optional>
#include <memory>
#include <vector>
#endif

//external libraries
#include <cmore.h>
#include <memcry.h>


// [debugging & unit testing support]
#ifdef DEBUG 
#define _SC_DBG_STATIC
#define _SC_DBG_INLINE
#define _SC_DBG_PRIVATE public
#else
#define _SC_DBG_STATIC static
#define _SC_DBG_INLINE inline
#define _SC_DBG_PRIVATE private
#endif


#ifdef __cplusplus
namespace sc {

class opt;

/*
 *  Dependency injection abstract scan class.
 */
struct _scan_arg {

    //[members]
    const uintptr_t addr;
    const off_t area_off;
    const size_t buf_left;
    const cm_byte * const cur_byte;
    const cm_lst_node * const area_node;

    //[methods]
    _scan_arg(const uintptr_t addr, const off_t area_off, const size_t buf_left,
              const cm_byte * const cur_byte, const cm_lst_node * const area_node)
     : addr(addr), area_off(area_off), buf_left(buf_left), cur_byte(cur_byte), area_node(area_node) {};
};


class _scan {

    _SC_DBG_PRIVATE:
        //[attributes]
        //TODO any?

    public:
        //[methods]
        //dependency injection function
        virtual void process_addr(const struct _scan_arg arg, const void * const arg_custom,
                                  const opt & opts, const void * const opts_custom) = 0;
};


/*
 *  Pointer scanner `arg_custom`
 */
class _ptrscan_tree_node;

struct _arg_ptrscan {

    //[attributes]
    std::vector<std::shared_ptr<_ptrscan_tree_node>> & depth_level_vec;
    std::optional<off_t> cur_offset;
};


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
