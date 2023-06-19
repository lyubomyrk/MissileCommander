#include "WELL512.hpp"

WELL512::WELL512() { init(0); };

WELL512::WELL512(unsigned int seed) { init(seed); };

void WELL512::init(unsigned int seed) {
  index = 0;
  state[0] = seed;
  for (int i = 1; i < 16; i++) {
    state[i] = (1812433253UL * (state[i - 1] ^ (state[i - 1] >> 30)) + i);
  }
};

unsigned long WELL512::well512rand() {
  unsigned long a, b, c, d;
  a = state[index];
  c = state[(index + 13) & 15];
  b = a ^ c ^ (a << 16) ^ (c << 15);
  c = state[(index + 9) & 15];
  c ^= (c >> 11);
  a = state[index] = b ^ c;
  d = a ^ ((a << 5) & 0xDA442D24UL);
  index = (index + 15) & 15;
  a = state[index];
  state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return state[index];
};

unsigned long WELL512::well512rand(unsigned long max) {
  if (max != 0)
    return well512rand() % max;
  else
    return 0;
};

float WELL512::randf() { return (float)well512rand() / (float)0xFFFFFFFF; };