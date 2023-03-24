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

constexpr float G = 0.001;
constexpr float eps = 0.001;
constexpr float dt = 0.01;

constexpr float Gdt = G * dt; // 提前算好这两个常量的乘积
constexpr float eps2 = eps * eps;

struct Star{
  float px[NSTAR], py[NSTAR], pz[NSTAR]; 
  float vx[NSTAR], vy[NSTAR], vz[NSTAR]; 
  float mass[NSTAR];
};

Star stars;

void
init(){
  for(size_t i = 0; i < NSTAR; i++) stars.px[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.py[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.pz[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.vx[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.vy[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.vz[i] = frand(); 
  for(size_t i = 0; i < NSTAR; i++) stars.mass[i] = frand(); 
}

#define unroll_d(x) \
  for(size_t j = 0; j < NSTAR; j++) d##x[j] = stars.p##x[i] - stars.p##x[j]; \

void step(){
  for(size_t i = 0; i < NSTAR; i++){
    float dx[NSTAR], dy[NSTAR], dz[NSTAR], d2[NSTAR], m_d2[NSTAR];
    unroll_d(x); unroll_d(y); unroll_d(z);  
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      d2[j] = dx[j] * dx[j] + dy[j] * dy[j] + dz[j] * dz[j] + eps2;
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      d2[j] = 1 / (d2[j] * std::sqrt(d2[j]));  // 直接存储d2的倒数
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      float m_d2 = stars.mass[j] * d2[j];

    float d_vx = 0, d_vy = 0, d_vz = 0;
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      d_vx += dx[j] * m_d2[j];
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      d_vy += dy[j] * m_d2[j];
  #pragma omp simd
    for(size_t j = 0; j < NSTAR; j++)
      d_vz += dz[j] * m_d2[j];
    stars.vx[i] += d_vx * Gdt;
    stars.vy[i] += d_vy * Gdt;
    stars.vz[i] += d_vz * Gdt;
  }
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
  // printf("Initial energy: %f\n", calc());
  auto dt = benchmark([&] {
    Timer t;
    for (int i = 0; i < 100000; i++){
      step(); 
    }
  });
  // printf("Final energy: %f\n", calc());
  printf("Time elapsed: %ld ms\n", dt);
  return 0;
}
