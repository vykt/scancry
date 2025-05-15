//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <functional>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "c_iface.hh"
#include "error.hh"



/*
 *  NOTE: For a fully-realised C interface, it must be possible to convert 
 *        between STL containers and those available in CMore. For basic 
 *        types like `int`, it is enough to either `memcpy()` directly or
 *        insert elements one at a time from one container to another.
 *
 *        For STL types only available in C++, they must first be converted 
 *        to C equivalents. This is done using the `convert_cb` callback.
 */

//copy contents of a CMore vector to a STL vector
template <typename T, typename T_c>
[[nodiscard]] int c_iface::vct_from_cmore_vct(
    const cm_vct * cmore_vct, std::vector<T> & stl_vct,
    std::optional<std::function<int(T *, T_c *)>> convert_cb) {

    int ret;
    T_c ent_c;
    T   ent_cc;


    try {
        //if both vectors store the same type, copy directly
        if (convert_cb.has_value() == false) {
            stl_vct.resize(cmore_vct->len);
            std::memcpy(stl_vct.data(), cmore_vct->data,
                        cmore_vct->data_sz * cmore_vct->len);

        //if both vectors store different types, use `convert_cb` to
        //convert each C entry to its C++ equivalent
        } else {
            for (int i = 0; i < cmore_vct->len; ++i) {

                ret = cm_vct_get(cmore_vct, i, &ent_c);
                if (ret != 0) {
                    sc_errno = SC_ERR_CMORE;
                    return -1;
                }

                ret = convert_cb.value()(&ent_cc, &ent_c);
                if (ret != 0) {
                    sc_errno = SC_ERR_TYPECAST;
                    return -1;
                }
                
                stl_vct.push_back(ent_cc);
            }
        } //end if-else

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }

    return 0;
}

//explicit instantiation - `const cm_lst_node *` -> `const cm_lst_node *`
template int c_iface::vct_from_cmore_vct<const cm_lst_node *,
                                         const cm_lst_node *>(
    const cm_vct * cmore_vct,
    std::vector<const cm_lst_node *> & stl_vct,
    std::optional<std::function<
        int(const cm_lst_node **, const cm_lst_node **)>>);

//explicit instantiation - `sc_addr_range` -> `std::pair<uintptr_t, uintptr_t>`
template int c_iface::vct_from_cmore_vct<std::pair<uintptr_t, uintptr_t>,
                                         sc_addr_range>(
    const cm_vct * cmore_vct,
    std::vector<std::pair<uintptr_t, uintptr_t>> & stl_vct,
    std::optional<std::function<
        int(std::pair<uintptr_t, uintptr_t> *, sc_addr_range *)>>);

//explicit instantiation - `off_t` -> `const off_t`
template int c_iface::vct_from_cmore_vct<off_t,
                                         off_t>(
    const cm_vct * cmore_vct,
    std::vector<off_t> & stl_vct,
    std::optional<std::function<
        int(off_t *, off_t *)>>);



//copy contents of a STL vector into a CMore vector
template <typename T,typename T_c>
[[nodiscard]] int c_iface::vct_to_cmore_vct(
    cm_vct * cmore_vct, const std::vector<T> & stl_vct,
    std::optional<std::function<int(T_c *, T *)>> convert_cb) {
    
    int ret;
    T_c ent_c;
    T ent_cc;
    
    //initialise the CMore vector
    ret = cm_new_vct(cmore_vct, sizeof(T));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    try {
        //if both vectors store the same type, copy directly
        if (convert_cb.has_value() == false) {
            ret = cm_vct_rsz(cmore_vct, stl_vct.size());
            if (ret != 0) {
                cm_del_vct(cmore_vct);
                sc_errno = SC_ERR_OPT_EMPTY;
                return -1;
            }

            std::memcpy(cmore_vct->data, stl_vct.data(),
                        stl_vct.size() * sizeof(T));

        //if both vectors store different types, use `convert_cb` to
        //convert each C entry to its C++ equivalent
        } else {
            for (auto it = stl_vct.cbegin(); it != stl_vct.cend(); it++) {

                ent_cc = *it;
                ret = convert_cb.value()(&ent_c, &ent_cc);
                if (ret != 0) {
                    cm_del_vct(cmore_vct);
                    sc_errno = SC_ERR_TYPECAST;
                    return -1;
                }

                ret = cm_vct_apd(cmore_vct, &ent_c);
                if (ret != 0) {
                    cm_del_vct(cmore_vct);
                    sc_errno = SC_ERR_CMORE;
                    return -1;
                }
            }
        } //end if-else

    } catch (const std::exception & excp) {
        cm_del_vct(cmore_vct);
        exception_sc_errno(excp);
        return -1;
    }

    return 0;
}

//explicit instantiation - `const cm_lst_node *` -> `const cm_lst_node *`
template int c_iface::vct_to_cmore_vct<const cm_lst_node *,
                                       const cm_lst_node *>(
    cm_vct * cmore_vct,
    const std::vector<const cm_lst_node *> & stl_vct,
    std::optional<std::function<
        int(const cm_lst_node **, const cm_lst_node **)>>);


//explicit instantiation - `std::pair<uintptr_t, uintptr_t>` -> `sc_addr_range`
template int c_iface::vct_to_cmore_vct<std::pair<uintptr_t, uintptr_t>,
                                       sc_addr_range>(
    cm_vct * cmore_vct,
    const std::vector<std::pair<uintptr_t, uintptr_t>> & stl_vct,
    std::optional<std::function<
        int(sc_addr_range *, std::pair<uintptr_t, uintptr_t> *)>>);

//explicit instantiation - `off_t` -> `off_t`
template int c_iface::vct_to_cmore_vct<off_t,
                                       off_t>(
    cm_vct * cmore_vct,
    const std::vector<off_t> & stl_vct,
    std::optional<std::function<
        int(off_t *, off_t *)>>);



/*
 *  NOTE: I imagine it is possible to template over the STL container class
 *        itself, and utilise SFINAE to select the appropriate insertion
 *        function. In my limited experiments I haven't got it to work.
 */


//copy contents of a STL vector into a CMore vector
template <typename T,typename T_c>
[[nodiscard]] int c_iface::uset_to_cmore_vct(
    cm_vct * cmore_vct, const std::unordered_set<T> & stl_uset,
    std::optional<std::function<int(T_c *, T *)>> convert_cb) {
    
    int ret;
    T_c ent_c;
    T ent_cc;
    
    //initialise the CMore vector
    ret = cm_new_vct(cmore_vct, sizeof(T));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    try {    
        for (auto it = stl_uset.cbegin(); it != stl_uset.cend(); it++) {

            ent_cc = *it;

            //if both containers store the same type
            if (convert_cb.has_value() == false) {
                ent_c = ent_cc;
                ret = cm_vct_apd(cmore_vct, &ent_c);
                if (ret != 0) {
                    cm_del_vct(cmore_vct);
                    sc_errno = SC_ERR_CMORE;
                    return -1;
                }

            //if both containers store different types, use `convert_cb` to
            //convert each C entry to its C++ equivalent
            } else {

                ret = convert_cb.value()(&ent_c, &ent_cc);
                if (ret != 0) {
                    cm_del_vct(cmore_vct);
                    sc_errno = SC_ERR_TYPECAST;
                    return -1;
                }

                ret = cm_vct_apd(cmore_vct, &ent_c);
                if (ret != 0) {
                    cm_del_vct(cmore_vct);
                    sc_errno = SC_ERR_CMORE;
                    return -1;
                }

            } //end if-else

        } //end for

    } catch (const std::exception & excp) {
        cm_del_vct(cmore_vct);
        exception_sc_errno(excp);
        return -1;
    }

    return 0;
}

//explicit instantiation - `const cm_lst_node *` -> `const cm_lst_node *`
template int c_iface::uset_to_cmore_vct<const cm_lst_node *,
                                        const cm_lst_node *>(
    cm_vct * cmore_vct,
    const std::unordered_set<const cm_lst_node *> & stl_uset,
    std::optional<std::function<
        int(const cm_lst_node **, const cm_lst_node **)>>);



//sort a CMore area pointer vector
void c_iface::sort_area_vct(cm_vct * cmore_vct) {

    //create a STL copy of the cmore vector
    std::vector<const cm_lst_node *> temp_vct;
    temp_vct.resize(cmore_vct->len);
    std::memcpy(temp_vct.data(), cmore_vct->data,
                cmore_vct->len * cmore_vct->data_sz);

    //use STL's sorting algorithm
    std::sort(temp_vct.begin(), temp_vct.end(),
              [](const cm_lst_node * a_node, const cm_lst_node * b_node) {
        mc_vm_area * a = MC_GET_NODE_AREA(a_node);
        mc_vm_area * b = MC_GET_NODE_AREA(b_node);
        return (a->end_addr < b->end_addr); 
    });

    //copy the STL vector back into the CMore vector
    std::memcpy(cmore_vct->data, temp_vct.data(),
                cmore_vct->len * cmore_vct->data_sz);

    return;
}
