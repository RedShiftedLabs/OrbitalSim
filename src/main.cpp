#include "Engine3D.hpp"
#include <exception>
#include <print>

int main() {
  Engine3D engine;
  try {
    engine.run();
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return 1;
  }
  return 0;
}