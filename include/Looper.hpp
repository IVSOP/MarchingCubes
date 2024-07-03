#ifndef LOOPER_H
#define LOOPER_H

#undef LOOPER_DEBUG

#include <chrono>
#include <immintrin.h>
#include <thread>
#include <cstdio>
#include <functional>

// basically a spinlock designed to wait for miliseconds with < 1% error
// not called spinlock since there is no lock, it just loops
// very ineficient for > 10ms, since it does not use sleep

// rough assumptions from som tests:
// _mm_pause and rep; nop seem to do the same thing
// all sleep functions are very bad
// _mm_pause delays for 0.01 to 0.02, nop delays around 0.0002

// sleep is never used as (especially in windows) it introduces a MASSIVE delay
// also, I expect the game to always be very heavy on the CPU, so this thread is mine forever and always and I'm not giving it away

#define LOOPER_MAGIC_COEFF 1.15
#define TEST_ITERS 5000 // this takes 50ms to run, is needed to get the cpu nice and hot so pause takes less time
#define NOP_BATCH_SIZE 100

template<uint32_t milis>
class Looper {
private:
	static std::chrono::nanoseconds getAvgPause();

public:
	// static???????
	static void loop(std::function<void()> func);
};

template<uint32_t nanoseconds>
std::chrono::nanoseconds Looper<nanoseconds>::getAvgPause() {
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < TEST_ITERS; i++) {
		// std::this_thread::sleep_for(std::chrono::milliseconds(0));
		_mm_pause();
	}
	auto end = std::chrono::high_resolution_clock::now();

	// multiply by some value to make it bigger for safety, I don't want to keep recalculating it (for now)
	double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / static_cast<double>(TEST_ITERS);
	duration *= LOOPER_MAGIC_COEFF;

	return std::chrono::nanoseconds(static_cast<uint32_t>(duration));
}

template<uint32_t nanoseconds>
void Looper<nanoseconds>::loop(std::function<void()> func) {
	// just so types are deduced, runs once, is fine
	auto start = std::chrono::high_resolution_clock::now();
	auto current = std::chrono::high_resolution_clock::now();

	std::chrono::nanoseconds target_time = std::chrono::nanoseconds(nanoseconds);

	// get the average duration of a pause, in nanoseconds
	// for now assume this is true and good
	std::chrono::nanoseconds pause_duration = getAvgPause();

	// printf("%ld\n", pause_duration.count());
	// exit(1);


	uint32_t num_pauses;

	do {
		start = std::chrono::high_resolution_clock::now();

		func();		

		// calculate how many pauses to achieve target time
		num_pauses = target_time.count() / pause_duration.count();


		for (uint32_t i = 0; i < num_pauses; i++) {
			// std::this_thread::sleep_for(std::chrono::milliseconds(0));
			_mm_pause();
		}

		current = std::chrono::high_resolution_clock::now();

		do {
			for (uint32_t i = 0; i < NOP_BATCH_SIZE; i++) {
				__asm__ __volatile__("nop" : : : "memory");
			}
			current = std::chrono::high_resolution_clock::now();
		} while (std::chrono::duration_cast<std::chrono::nanoseconds>(current - start) < target_time);

		target_time = std::chrono::nanoseconds(nanoseconds) - (target_time - std::chrono::duration_cast<std::chrono::nanoseconds>(current - start));

#ifdef LOOPER_DEBUG
		printf("time elapsed is %f\n", static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(current - start).count()) / 1000.0f);
#endif

	} while(true);
}

#endif
