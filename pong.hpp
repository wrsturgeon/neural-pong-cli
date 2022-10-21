#pragma once

#include <array>    // std::array
#include <cassert>  // assert
#include <numeric>  // std::gcd
#include <string>   // std::to_string

#include "types.hpp"

static constexpr auto kEmpty   = ' ';
static constexpr auto kHBorder = '-';
static constexpr auto kVBorder = '|';
static constexpr auto kCorner  = '+';
static constexpr auto kBall    = 'O';
static constexpr auto kPaddle  = ']';
static constexpr auto kLose    = 'X';

static constexpr auto hw_sz_b = 16;
static constexpr auto small_b = 8;
static_assert(small_b < hw_sz_b);
using hw_sz_t = index_t<hw_sz_b>;
using small_t = index_t<small_b>;
using area_t = index_t<(hw_sz_b << 1)>;

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth>
concept PongChecks = (
  !!H and !!W and !!BallDiameter and !!PaddleHeight and !!PaddleWidth and // Nonzero parameters
#ifdef NDEBUG
  (H >= PaddleHeight + BallDiameter) and // Can't always win unless we're debugging
#else
  (H >= PaddleHeight) and // Paddles have to fit on the screen
#endif
  (static_cast<hw_sz_t>(W + (PaddleWidth << 1) + 1) > W) and // Overflow check
  (static_cast<hw_sz_t>(H + BallDiameter) > H) and // Overflow check
  (std::gcd(W, H) == 1) // Maximize period between repeating states; stasis will inevitably lose
);

#ifdef NDEBUG
#define NOX noexcept
#else
#define NOX
#endif

template <
  hw_sz_t H,
  hw_sz_t W = static_cast<hw_sz_t>(H * 1.618),
  small_t BallDiameter = 1,
  hw_sz_t PaddleHeight = ((H + 2) / 3),
  small_t PaddleWidth = 1,
  small_t Newlines = 2>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
class Pong {
 public:
  static constexpr hw_sz_t BoardHeight = H + BallDiameter;
  static constexpr hw_sz_t BoardWidth  = W + BallDiameter;
  static_assert(BoardHeight > H); // Overflow check
  static_assert(BoardWidth  > W); // Overflow check
  static constexpr area_t BoardArea = static_cast<area_t>(BoardHeight) * static_cast<area_t>(BoardWidth);
 private:
  hw_sz_t paddle_y = 0; // Always <= H - PaddleHeight
  std::array<hw_sz_t, 2> ball_x = {0, 0}; // Always <= W or H, respectively
  std::array<bool, 2> ball_v = {1, 1}; // 0: x, 1: y
  static constexpr hw_sz_t ScreenHeight = 1 + BoardHeight + 1; // Border, ball, space, border
  static constexpr hw_sz_t ScreenWidth = PaddleWidth + BoardWidth + 1 + 1; // Paddle, ball, space, border, newline
  static_assert(ScreenHeight > BoardHeight); // Overflow check
  static_assert(ScreenWidth  > BoardWidth ); // Overflow check
  std::array<char, ScreenHeight * ScreenWidth + Newlines + 1> screen; // + 1 : null terminator
  [[nodiscard]] [[gnu::always_inline]] inline constexpr auto get(hw_sz_t y, hw_sz_t x) NOX -> char&;
 public:
  constexpr Pong(hw_sz_t x_init = 1, hw_sz_t y_init = 1, bool vx_init = 1, bool vy_init = 1, hw_sz_t paddle_y_init = 0) noexcept;
  [[nodiscard]] constexpr auto step() noexcept -> bool;
  [[nodiscard]] [[gnu::always_inline]] inline constexpr operator char const*() const noexcept { return screen.data(); }
  [[gnu::always_inline]] inline constexpr void paddle_up() noexcept;
  [[gnu::always_inline]] inline constexpr void paddle_dn() noexcept;
  [[gnu::always_inline]] inline constexpr void end_game() noexcept;
};

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
[[nodiscard]] [[gnu::always_inline]] inline constexpr auto
Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::get(hw_sz_t y, hw_sz_t x) NOX -> char& {
#ifndef NDEBUG
  if ((y >= BoardHeight) or (x >= BoardWidth)) {
    throw std::out_of_range{"Pong::get("
          + std::to_string(y) + ", " + std::to_string(x) + ") into a ("
          + std::to_string(BoardHeight) + ", " + std::to_string(BoardWidth) + ") board"};
  }
#endif
  return screen[(ScreenWidth * (1 + y)) + PaddleWidth + x];
}

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
constexpr Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::
Pong(hw_sz_t x_init, hw_sz_t y_init, bool vx_init, bool vy_init, hw_sz_t paddle_y_init) noexcept
      : paddle_y{paddle_y_init}, ball_x{y_init, x_init}, ball_v{vy_init, vx_init} {
  assert(x_init <= W);
  assert(y_init <= H);
  assert(paddle_y_init <= H - PaddleHeight);
  assert(x_init > 0); // induction n/a on the very first step, otherwise x = 0 ==> vx = leftward

  // Null terminator
  screen.back() = 0;

  // Extra newlines
  #pragma unroll
  for (small_t i = 0; i < Newlines; ++i) { screen[(ScreenHeight * ScreenWidth) + i] = '\n'; }

  // Fill the background
  std::fill(screen.begin(), screen.end() - Newlines - 1, kEmpty);

  // Corners
  screen[ScreenWidth - 1] = '\n';
  screen[ScreenWidth - 2] = kCorner;
  screen[(ScreenWidth * ScreenHeight) - 2] = kCorner;

  // Vertical border
  for (hw_sz_t i = 1; i < ScreenHeight - 1; ++i) {
    screen[ScreenWidth * i + ScreenWidth - 2] = kVBorder;
    screen[ScreenWidth * i + ScreenWidth - 1] = '\n'; }
  
  // Horizontal borders
  for (hw_sz_t j = 0; j < ScreenWidth - 2; ++j) {
    screen[j] = screen[ScreenWidth * (ScreenHeight - 1) + j] = kHBorder; }
  
  // Ball
  #pragma unroll
  for (hw_sz_t i = ball_x[0]; i < ball_x[0] + BallDiameter; ++i) {
    for (hw_sz_t j = ball_x[1]; j < ball_x[1] + BallDiameter; ++j) {
      get(i, j) = kBall; } }
  
  // Paddle
  for (hw_sz_t i = 0; i < PaddleHeight; ++i) {
    for (hw_sz_t j = 0; j < PaddleWidth; ++j) {
      screen[ScreenWidth * (1 + paddle_y + i) + j] = kPaddle; } }
}

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
[[gnu::always_inline]] inline constexpr void
Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::paddle_up() noexcept {
  if (!paddle_y) { return; }
  screen[ScreenWidth * paddle_y] = kPaddle;
  screen[ScreenWidth * (paddle_y + PaddleHeight)] = kEmpty;
  --paddle_y;
}

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
[[gnu::always_inline]] inline constexpr void
Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::paddle_dn() noexcept {
  if (paddle_y > H - PaddleHeight) { return; }
  ++paddle_y;
  screen[ScreenWidth * paddle_y] = kEmpty;
  screen[ScreenWidth * (paddle_y + PaddleHeight)] = kPaddle;
}

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
[[nodiscard]] constexpr auto
Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::step() noexcept -> bool {

  // Ideal bouncing physics
  switch (ball_x[0]) {
    case H: ball_v[0] = 0; break;
    case 0: ball_v[0] = 1; break; }
  switch (ball_x[1]) {
    case W: ball_v[1] = 0; break;
    case 0:
      if ((paddle_y + PaddleHeight <= ball_x[0]) or
          (paddle_y >= ball_x[0] + BallDiameter)) {
        end_game();
        return false; }
      ball_v[1] = 1;
      break; }

  // Erase trailing edge, move ball, and fill leading edge
  hw_sz_t y = ball_x[0] + (ball_v[0] ? 0 : (BallDiameter - 1));
  hw_sz_t x = ball_x[1] + (ball_v[1] ? 0 : (BallDiameter - 1));
  get(y, x) = kEmpty;
  #pragma unroll
  for (small_t i = 1; i < BallDiameter; ++i) {
    get(y + (ball_v[0] ? i : -i), x) = kEmpty;
    get(y, x + (ball_v[1] ? i : -i)) = kEmpty; }
  y = (ball_x[0] += (ball_v[0] ? 1 : -1)) + (ball_v[0] ? (BallDiameter - 1) : 0);
  x = (ball_x[1] += (ball_v[1] ? 1 : -1)) + (ball_v[1] ? (BallDiameter - 1) : 0);
  get(y, x) = kBall;
  #pragma unroll
  for (small_t i = 1; i < BallDiameter; ++i) {
    get(y + (ball_v[0] ? -i : i), x) = kBall;
    get(y, x + (ball_v[1] ? -i : i)) = kBall; }
  
  // Autoplay
  if (paddle_y + PaddleHeight == ball_x[0]) { paddle_dn(); }
  else if (ball_x[0] + BallDiameter == paddle_y) { paddle_up(); }

  return true;
}

template <hw_sz_t H, hw_sz_t W, small_t BallDiameter, hw_sz_t PaddleHeight, small_t PaddleWidth, small_t Newlines>
requires PongChecks<H, W, BallDiameter, PaddleHeight, PaddleWidth>
[[gnu::always_inline]] inline constexpr void
Pong<H, W, BallDiameter, PaddleHeight, PaddleWidth, Newlines>::end_game() noexcept {
  // Copied from the above but optimized and cracked open to avoid memory leaks
  // we know x = 0, vx = leftward = 0
  // Erase trailing edge, move ball, and fill leading edge
  hw_sz_t y = ball_x[0] + (ball_v[0] ? 0 : (BallDiameter - 1));
  get(y, BallDiameter - 1) = kEmpty;
  #pragma unroll
  for (small_t i = 1; i < BallDiameter; ++i) {
    get(y + (ball_v[0] ? i : -i), BallDiameter - 1) = kEmpty;
    get(y, BallDiameter - 1 - i) = kEmpty; }
  
  y = static_cast<hw_sz_t>(ball_x[0] + (ball_v[0] ? 1 : -1)) + (ball_v[0] ? (BallDiameter - 1) : 0);
  screen[(ScreenWidth * (1 + y)) + PaddleWidth - 1] = kLose;
  #pragma unroll
  for (small_t i = 1; i < BallDiameter; ++i) {
    screen[(ScreenWidth * (1 + y + (ball_v[0] ? -i : i))) + PaddleWidth - 1] = kLose;
    get(y, (ball_v[1] ? -i : i) - 1) = kBall; }
}
