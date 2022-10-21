#pragma once

#include <bit>      // std::bit_width
#include <cstddef>  // std::size_t

#include "types.hpp" // pvec_t, index_t

template <lg_t lgD>
struct Input {
  data_t value;
  pvec_t<lgD> position;
};

template <std::size_t N, std::size_t D>
class InputHandler {
 public:
  static constexpr lg_t lgN = std::bit_width(N);
  static constexpr lg_t lgD = std::bit_width(D);
 private:
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto operator()(index_t<lgN> i) const noexcept -> Input<lgD>;
};

template <std::size_t N>
class OutputHandler {
 public:
  static constexpr lg_t lgN = std::bit_width(N);
 private:
  [[gnu::always_inline]] inline constexpr void operator()(index_t<lgN> i) const noexcept;
};
