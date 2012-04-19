#ifndef SIUT_PERF_UTILS_CLOCK_HPP_
#define SIUT_PERF_UTILS_CLOCK_HPP_

#include "siut2/perf_utils/Clock.hpp"

#include <iostream>
#include <iomanip>


namespace siut2 {
namespace perf_utils {

/**
  * Class that uses Clock to implement an FPS-counter
  * @author <Andre.Brodtkorb@sintef.no>
  */
class FpsCounter {
public:
	FpsCounter() : frames_(0), elapsed_(0.0) {};
	~FpsCounter() {};

	void start();
	void stop();
	void frame();
	void restart();
	void reset();

	double elapsed();
	unsigned long long frames();
	double fps();
	
	std::ostream& print(std::ostream& os);

private:
	Clock rolex_;
	unsigned long long frames_; //< Number of frames_
	double elapsed_; //< elapsed_ time between start and stop
  FpsCounter(const FpsCounter& other){}
//  FpsCounter& operator=(const FpsCounter& other){}

};

inline void FpsCounter::start(){
	rolex_.start();
}

inline void FpsCounter::stop() {
	elapsed_ = rolex_.stop();
}

inline void FpsCounter::frame() {
	assert(rolex_.isRunning() && "Calling frame() on a stopped FpsCounter is mongo[tm].");
	++frames_;
}

inline void FpsCounter::restart() {
	reset();
	start();
}

inline void FpsCounter::reset() {
	rolex_.reset();
	frames_ = 0;
	elapsed_ = 0.0;
}

inline double FpsCounter::elapsed() {
	return (rolex_.isRunning()) ? rolex_.elapsed() : elapsed_;
}

inline unsigned long long FpsCounter::frames() {
	return frames_;
}

inline double FpsCounter::fps() {
	return (double) frames() / elapsed();
}

inline std::ostream& FpsCounter::print(std::ostream& os) {
	os << std::fixed << std::setprecision(2);
	os << fps() << " fps.";
	os << std::scientific;

	return os;
}

inline std::ostream& operator<<(std::ostream & os, FpsCounter& f) {
	return f.print(os);
}

} // namespace timer
} // namespace siut


#endif
