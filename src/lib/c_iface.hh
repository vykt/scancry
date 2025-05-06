#pragma once

//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <functional>

//external libraries
#include <cmore.h>


namespace c_iface {

    template <typename T, typename T_c>
    [[nodiscard]] int vct_from_cmore_vct(const cm_vct * cmore_vct,
                                         std::vector<T> & stl_vct,
                                         std::optional<
                                             std::function<int(T *, T_c *)>>);

    template <typename T, typename T_c>
    [[nodiscard]] int vct_to_cmore_vct(cm_vct * cmore_vct,
                                       const std::vector<T> & stl_vct,
                                       std::optional<
                                           std::function<int(T_c *, T *)>>);

    template <typename T, typename T_c>
    [[nodiscard]] int uset_from_cmore_vct(const cm_vct * cmore_vct,
                                          std::unordered_set<T> & stl_vct,
                                          std::optional<
                                              std::function<int(T *, T_c *)>>);

    template <typename T, typename T_c>
    [[nodiscard]] int uset_to_cmore_vct(cm_vct * cmore_vct,
                                        const std::unordered_set<T> & stl_vct,
                                        std::optional<
                                            std::function<int(T_c *, T *)>>);
 

} //namespace `c_iface`
