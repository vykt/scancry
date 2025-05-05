//standard template library
#include <optional>
#include <vector>
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
 *  NOTE: All of these functions have to be explicitly instantiated
 *        because C++ is a 45 year old mistake.
 */

//copy contents of a CMore vector to a STL vector
template <typename T, typename T_c>
[[nodiscard]] int c_iface::from_cmore_vct(
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

//explicit instantiations
template int c_iface::from_cmore_vct<const cm_lst_node *, const cm_lst_node *>(
    const cm_vct * cmore_vct,
    std::vector<const cm_lst_node *> & stl_vct,
    std::optional<std::function<
        int(const cm_lst_node **, const cm_lst_node **)>>);



//copy contents of a STL vector into a CMore vector
template <typename T,typename T_c>
[[nodiscard]] int c_iface::to_cmore_vct(
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

//explicitly instantiations
template int c_iface::to_cmore_vct<const cm_lst_node *, const cm_lst_node *>(
    cm_vct * cmore_vct,
    const std::vector<const cm_lst_node *> & stl_vct,
    std::optional<std::function<
        int(const cm_lst_node **, const cm_lst_node **)>>);
