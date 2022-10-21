#pragma once

#include <algorithm>   // std::transform
#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t, ...
#include <type_traits> // std::conditional_t

template <std::size_t lgN> using index_t =
  std::conditional_t<lgN <= 8, std::uint8_t,
    std::conditional_t<lgN <= 16, std::uint16_t,
      std::conditional_t<lgN <= 32, std::uint32_t,
        std::conditional_t<lgN <= 64, std::uint64_t, /* TODO */ void>>>>;

using lg_t = index_t<std::bit_width(sizeof(std::size_t) << 3)>;

static constexpr auto data_b = 8;
static constexpr auto axial_b = 16;
using data_t = index_t<data_b>; // E.g. neural "voltage"
using axial_t = index_t<axial_b>; // Scalar position along one axis

template <std::size_t D>
class pvec_t {
 private:
  friend class pvec_t<D>;
  static constexpr auto lgD = std::bit_width(D);
  using sq_dist_t = index_t<(axial_b << 1) * lgD>; // Enough to store the sum of D axial_t squares
  std::array<axial_t, D> vec;
 public:
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto operator+(pvec_t<D> const& x) const noexcept -> pvec_t<D>;
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto operator-(pvec_t<D> const& x) const noexcept -> pvec_t<D>;
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto square() const noexcept -> sq_dist_t; // requires std::integral<sq_dist_t>
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto square_dist(pvec_t<D> const& x) const noexcept -> sq_dist_t; // requires std::integral<sq_dist_t>;
};

template <std::size_t D>
[[nodiscard]] [[gnu::always_inline]] inline constexpr auto
pvec_t<D>::operator+(pvec_t<D> const& x) const noexcept -> pvec_t<D> {
  pvec_t<D> result; std::transform(vec.begin(), vec.end(), x.vec.begin(), result.vec.begin(), std::plus<axial_t>()); return result;
}

template <std::size_t D>
[[nodiscard]] [[gnu::always_inline]] inline constexpr auto
pvec_t<D>::operator-(pvec_t<D> const& x) const noexcept -> pvec_t<D> {
  pvec_t<D> result; std::transform(vec.begin(), vec.end(), x.vec.begin(), result.vec.begin(), std::minus<axial_t>()); return result;
}

template <std::size_t D>
[[nodiscard]] [[gnu::always_inline]] inline constexpr auto
pvec_t<D>::square() const noexcept -> sq_dist_t { // requires std::integral<sq_dist_t>
  sq_dist_t result = 0; for (auto const& x : vec) { result += static_cast<sq_dist_t>(x) * static_cast<sq_dist_t>(x); } return result;
}

template <std::size_t D>
[[nodiscard]] [[gnu::always_inline]] inline constexpr auto
pvec_t<D>::square_dist(pvec_t<D> const& x) const noexcept -> sq_dist_t { // requires std::integral<sq_dist_t> {
  return (*this - x).square();
}
