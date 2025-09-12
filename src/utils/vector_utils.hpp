#pragma once

#include <vector>
#include <ranges>
#include <algorithm>
#include <iterator>

namespace miniml {
    template <typename R, typename Alloc = std::allocator<std::ranges::range_value_t<R>>>
    auto to_vector(R&& r, Alloc const& alloc = Alloc{}) {
        using T = std::ranges::range_value_t<R>;
        std::vector<T, Alloc> v{alloc};
        if constexpr (std::ranges::sized_range<R>) {
            v.reserve(std::ranges::size(r));
        }
        std::ranges::copy(r, std::back_inserter(v));
        return v;
    }
}
