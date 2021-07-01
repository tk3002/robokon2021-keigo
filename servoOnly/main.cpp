#include "mbed.h"

Serial pc(USBTX,USBRX);
DigitalOut led(LED1);
PwmOut  xservo(D2);
PwmOut  yservo(D4);

int main(){
	
	//This is a test code
	while(true){
		pc.printf("Hello, Mbed! led is %d\n",(int)led);
		led=!led;
		wait(1);
		xservo.write(0.031);
		yservo.write(0.031);
		wait(1);
		xservo.write(0.1165);
		yservo.write(0.1165);
		
	}
	
    return 0;
}