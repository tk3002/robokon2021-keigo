#include "aSoftwareServo.hpp"

namespace rob{


const unsigned long SoftwareServo::PULSE_WIDTH=20000;

SoftwareServo::SoftwareServo(const PinName outputPin):
	output(outputPin)
{
	nextOnTime=0xffffffff;
	nextOffTime=0xffffffff;
}

void SoftwareServo::init(){
	timer.start();
	nextOnTime=0;//10ms後に開始
	nextOffTime=PULSE_WIDTH;
}

void SoftwareServo::runInLoop(){
	const unsigned long NEUTRAL=500;
	const unsigned long nowTime=timer.read_us();
	if(nowTime>=nextOnTime){
		output=true;
		nextOnTime+=PULSE_WIDTH;//2ms
	}else if(nowTime>=nextOffTime){
		output=false;
		nextOffTime=nextOnTime+pulseLength;
	}
}

void SoftwareServo::roll(const float angle){
	const unsigned long NEUTRAL=1475;
	pulseLength=NEUTRAL+((angle)*9.5);
}

}