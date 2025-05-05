#pragma once

//standard template library
#include <optional>
#include <vector>
#include <functional>

//external libraries
#include <cmore.h>


namespace c_iface {

    template <typename T, typename T_c>
    [[nodiscard]] int from_cmore_vct(const cm_vct * cmore_vct,
                                     std::vector<T> & stl_vct,
                                     std::optional<
                                        std::function<int(T *, T_c *)>>);

    template <typename T, typename T_c>
    [[nodiscard]] int to_cmore_vct(cm_vct * cmore_vct,
                                   const std::vector<T> & stl_vct,
                                   std::optional<
                                        std::function<int(T_c *, T *)>>);
    
} //namespace `c_iface`
