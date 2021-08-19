#ifndef A_SOFTWARE_SERVO_HPP_INCLUDE_GUARD
#define A_SOFTWARE_SERVO_HPP_INCLUDE_GUARD

#include "mbed.h"


namespace rob{
class SoftwareServo{
	private:
	static const unsigned long PULSE_WIDTH;
	DigitalOut output;
	Timer timer;
	unsigned long nextOnTime,nextOffTime;
	unsigned long pulseLength;
	
	public:
	SoftwareServo(const PinName outputPin);
	void init();
	void runInLoop();
	void roll(const float angle);
};

}//namespace rob
#endif