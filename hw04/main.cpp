#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <cmath>
#include <mylib.h>

constexpr size_t NSTAR = 48;

float frand() {
  return (float)rand() / RAND_MAX * 2 - 1;
}

// TODO: AOS  --->  SOA
struct Star {
  float px, py, pz;
  float vx, vy, vz;
  float mass;
};
std::vector<Star> stars;

void init() {
  for (int i = 0; i < NSTAR; i++) {
    stars.push_back({
      frand(), frand(), frand(),
      frand(), frand(), frand(),
      frand() + 1,
    });
  }
}

constexpr float G = 0.001;
constexpr float eps = 0.001;
constexpr float dt = 0.01;

constexpr float Gdt = G * dt; // 提前算好这两个常量的乘积
constexpr float eps2 = eps * eps;


void step() {
  for (auto &star : stars) {
    for (auto &other : stars) {
      float dx = other.px - star.px;
      float dy = other.py - star.py;
      float dz = other.pz - star.pz;
      float d2 = dx * dx + dy * dy + dz * dz + eps2;
      d2 *= std::sqrt(d2);
      // star.vx += dx * other.mass * G * dt * d2;
      // star.vy += dy * other.mass * G * dt * d2;
      // star.vz += dz * other.mass * G * dt * d2;
      // dx, dy, dz后面的算式都是重复的，可以提取
      float m_G_dt_d2 = other.mass * Gdt / d2;
      star.vx += dx * m_G_dt_d2;
      star.vy += dy * m_G_dt_d2;
      star.vz += dz * m_G_dt_d2;
    }
  }
  for (auto &star : stars) {
    star.px += star.vx * dt;
    star.py += star.vy * dt;
    star.pz += star.vz * dt;
  }
}

float calc() {
  float energy = 0;
  for (auto &star : stars) {
    float v2 = star.vx * star.vx + star.vy * star.vy + star.vz * star.vz;
    energy += star.mass * v2 * 0.5;
    for (auto &other : stars) {
      float dx = other.px - star.px;
      float dy = other.py - star.py;
      float dz = other.pz - star.pz;
      float d2 = dx * dx + dy * dy + dz * dz + eps2;
      energy -= other.mass * star.mass * G / (std::sqrt(d2) * 2);
    }
  }
  return energy;
}

template <class Func>
long benchmark(Func const &func) {
  auto t0 = std::chrono::steady_clock::now();
  func();
  auto t1 = std::chrono::steady_clock::now();
  auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
  return dt.count();
}

int main() {
  {
    Timer t;
    init();
  }
  printf("Initial energy: %f\n", calc());
  auto dt = benchmark([&] {
    Timer t;
    for (int i = 0; i < 100000; i++){
      step(); 
    }
  });
  printf("Final energy: %f\n", calc());
  printf("Time elapsed: %ld ms\n", dt);
  return 0;
}
