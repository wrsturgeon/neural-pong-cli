#pragma once

// quick C++ized xoshiro (see original comment below for creds)

/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <array>   // std::array
#include <cstdint> // std::uint64_t

template <std::uint8_t k>
[[nodiscard]] [[gnu::always_inline]] inline static auto
rotl(std::uint64_t const x) -> std::uint64_t {
	return (x << k) | (x >> (64 - k));
}

class prng {
 private:
  std::array<std::uint64_t, 4> s;
 public:
  constexpr prng(std::uint64_t s1, std::uint64_t s2, std::uint64_t s3, std::uint64_t s4) noexcept;
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto operator()() noexcept -> std::uint64_t;
};

constexpr prng::prng(
  std::uint64_t s1 = 0xe220a8397b1dcdaf,
  std::uint64_t s2 = 0x6e789e6aa1b965f4,
  std::uint64_t s3 = 0x06c45d188009454f,
  std::uint64_t s4 = 0xf88bb8a8724c81ec)
: s{s1, s2, s3, s4} {}

[[nodiscard]] [[gnu::always_inline]] inline constexpr auto prng::operator()() noexcept -> std::uint64_t {
  std::uint64_t const result = rotl<23>(s[0] + s[3]) + s[0];
  std::uint64_t const t = s[1] << 17;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3]  = rotl<45>(s[3]);
  return result;
}
