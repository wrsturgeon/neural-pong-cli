#include "learn.hpp"
#include "pong.hpp"

using pong_t = Pong<9>;
static pong_t pong;

void constexpr paddle_up() noexcept { pong.paddle_up(); }
void constexpr paddle_dn() noexcept { pong.paddle_dn(); }

auto main() -> int {
  NeuralCulture<pong_t::BoardArea, 2, 8, 4, 4, 4, &paddle_up, &paddle_dn> nc;
  return 0;
}
