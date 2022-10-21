#include "pong.hpp"

#include <chrono>  // ms literal
#include <cstdio>  // std::puts
#include <thread>  // std::this_thread::sleep_for

using namespace std::chrono_literals;

auto main() -> int {
  Pong<9, 14, 2> pong{1, 3};
  unsigned char i = 0;
  do { std::puts(pong); std::this_thread::sleep_for(250ms); } while (pong.step() and ++i);
  std::puts(pong);
}
