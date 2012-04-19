#pragma once

#if defined(__unix) || defined(__APPLE__)
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#define NOMINMAX
#include <windows.h>
#endif

#include <cassert>

namespace siut2 {
namespace perf_utils {

/**
  * A class that simulates a standard stop watch with
  * start, stop, lap and reset buttons.
  * @author <Andre.Brodtkorb@sintef.no>
  */
class Clock {
public:
        Clock() : start_time(0.0), lap_time(0.0), elapsed_time(0.0)  {};
	~Clock() {};

	void start();
	double stop();
	double elapsed();
	double lap();
	void reset();

	bool isRunning();

protected:
	double static getCurrentTime();

private:
	double start_time;
	double lap_time;
	double elapsed_time;

  Clock(const Clock& other) {}
  //Clock& operator= (const Clock& other){}
};


/**
  * Returns platform-specific best approximation to "current time". Current time
  * cannot be used for anything else but to record time durations (i.e., not epoch etc.).
  * @return Time in seconds
  */
inline double Clock::getCurrentTime() {
#if defined(__unix) || defined(__APPLE__)
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec+tv.tv_usec*1e-6;
#elif defined(_WIN32)
	LARGE_INTEGER f;
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&t);
	return t.QuadPart/(double) f.QuadPart;
#endif
}

inline void Clock::start() {
	assert(start_time == 0.0 && "Clock allready started.");
	start_time = getCurrentTime()-elapsed_time;
	lap_time = start_time;
}

/**
  * Stops the clock, and resets start time
  * @return The elapsed time since start
  */
inline double Clock::stop() {
	assert(start_time != 0.0 && "Clock not started.");
	elapsed_time = getCurrentTime() - start_time;
	start_time = 0.0;
	lap_time = 0.0;
	return elapsed_time;
}

inline bool Clock::isRunning() {
	return start_time != 0.0;
}

/**
  * @return elapsed time since start
  */
inline double Clock::elapsed() {
	assert(start_time != 0.0 && "Clock not started.");
	return getCurrentTime() - start_time;
}

/**
  * @return Elapsed time since start or last lap
  */
inline double Clock::lap() {
	assert(lap_time != 0.0 && "Clock not started.");
	double old_lap_time = lap_time;
	lap_time = getCurrentTime();
	return lap_time - old_lap_time;
}

inline void Clock::reset() {
	start_time = 0.0;
	lap_time = 0.0;
	elapsed_time = 0.0;
}

} // namespace timer
} // namespace siut


