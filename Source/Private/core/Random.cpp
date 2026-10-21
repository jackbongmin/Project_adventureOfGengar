#include "core/Random.h"

#include <random>

namespace gengar {
namespace {
std::mt19937& Rng() {
  static std::mt19937 rng{std::random_device{}()};
  return rng;
}
}  // namespace

float RandRange(float lo, float hi) {
  if (hi <= lo) return lo;
  return std::uniform_real_distribution<float>(lo, hi)(Rng());
}

int RandInt(int lo, int hi) {
  if (hi <= lo) return lo;
  return std::uniform_int_distribution<int>(lo, hi)(Rng());
}

bool Chance(float p) { return RandRange(0.f, 1.f) < p; }

}  // namespace gengar
