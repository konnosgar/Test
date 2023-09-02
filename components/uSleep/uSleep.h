#pragma once

#include "freertos/FreeRTOS.h"
#include <sys/time.h>
#include "xtensa/core-macros.h" // Access to ccount

#ifdef __cplusplus
extern "C" {
#endif

#define GetTickCount() XTHAL_GET_CCOUNT()

static uint32_t PicoSecsPerSleepTime;
static uint32_t FemtoSecsPerSleepTime;

void SleepTimes(const uint32_t aReps)
{ // We time the whole framework on this function, so it must be as close as to the SleepNano function bellow
  uint32_t Reps, Counter;
  Reps = (uint64_t)(1000ULL * (uint64_t)aReps) / 1000ULL; // On purpose we assign to the variable, so this function is as close as possible to SleepNano and the rest
  for (Counter = 0; Counter < Reps; Counter++)
    asm volatile ("nop;");
}

void InitSleep()
{
  int StartTick, NowTick, Diff;
  uint64_t Value, CPUMHz;

  CPUMHz = configCPU_CLOCK_HZ / 1000000;

  // Measure Sleep Times
  StartTick = GetTickCount();
  SleepTimes(8388608);
  NowTick = GetTickCount();
  Diff = NowTick - StartTick;

  Value = (uint64_t)(1000ULL * 1000ULL * 1000ULL * (uint64_t)Diff) / (uint64_t)(CPUMHz * 8388608ULL);
  FemtoSecsPerSleepTime = Value;
  PicoSecsPerSleepTime = Value / 1000ULL;
}

void SleepNOP(const uint32_t aNOPs)
{ // Six times more per repetition than just calling NOP.
  uint32_t Reps, Counter;
  Reps = aNOPs / 6.02607;
  for (Counter = 0; Counter < Reps; Counter++)
    asm volatile ("nop;");
}

void SleepNano(const uint32_t aNano)
{
  uint32_t Reps, Counter;
  Reps = (uint64_t)(1000ULL * 1000ULL * (uint64_t)aNano) / (uint64_t)FemtoSecsPerSleepTime;
  for (Counter = 0; Counter < Reps; Counter++)
    asm volatile ("nop;");
}

void SleepMicro(const uint32_t aMicro)
{
  uint32_t Reps, Counter;
  Reps = (uint64_t)(1000ULL * 1000ULL * 1000ULL * (uint64_t)aMicro) / (uint64_t)FemtoSecsPerSleepTime;
  for (Counter = 0; Counter < Reps; Counter++)
    asm volatile ("nop;");
}

void SleepMilli(const uint32_t aMilli)
{
  uint32_t Reps, Counter;
  Reps = (uint64_t)(1000ULL * 1000ULL * 1000ULL * 1000ULL * (uint64_t)aMilli) / (uint64_t)FemtoSecsPerSleepTime;
  for (Counter = 0; Counter < Reps; Counter++)
    asm volatile ("nop;");
}

#ifdef __cplusplus
}
#endif