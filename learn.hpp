#pragma once

#include "handlers.hpp" // InputHandler, OutputHandler

#include "types.hpp" // pvec_t, index_t

#include <array>    // std::array
#include <bit>      // std::bit_width
#include <cstddef>  // std::size_t

template <std::size_t N_in, std::size_t N_out, lg_t lgN, std::size_t D_in, std::size_t D, std::size_t Branches, std::size_t Probes>
concept NeuralChecks = (
  !!N_in and !!N_out and !!Branches and !!Probes and
  (lgN >= std::bit_width(N_in + N_out)) and
  (D >= D_in) and
  (lgN >= std::bit_width(Branches + Probes)));

template <
  std::size_t N_in,
  std::size_t N_out,
  lg_t lgN,
  std::size_t D_in,
  std::size_t D,
  std::size_t Branches = 8,
  std::size_t Probes = Branches>
requires NeuralChecks<N_in, N_out, lgN, D_in, D, Branches, Probes>
class NeuralCulture {
 private:
  using pos_t = pvec_t<lgD>;
  using idx_t = index_t<lgN>;
  static constexpr std::size_t N = static_cast<std::size_t>(1) << lgN;
  std::array<data_t, N> value;
  std::array<pos_t, N> position;
  std::array<std::array<idx_t, Branches + Probes>, N> synapse;
};
