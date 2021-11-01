//直径40mm　円周125.6mm 全長1330mm　10.5回転  リミットセンサ不調時は駆動用バッテリを確認
#include "mbed.h"
#include <string.h>

#include "qei.h"
#include "aTB6643KQ.hpp"

#define resolution 2048
#define setrevo 4
#define power 1

Serial pc(USBTX,USBRX);
DigitalOut led(LED1);
//QEI wheel(PA_11,PC_4,PB_15,resolution,0, QEI::X4_ENCODING);//ロータリーエンコーダー
rob::aTB6643KQ motorL(PB_3,PA_10);//モーター
rob::aTB6643KQ motorR(PB_5,PB_10);
//InterruptIn  touchR(PB_8);
//InterruptIn  touchL(PC_9);
DigitalIn touchR(PB_8);
DigitalIn touchL(PC_9); 

float angle=0;
int revo=setrevo;
int anglecount=0;
int ingi=0;

class regularC_ms{
private:
	unsigned long interval;
	unsigned long nextTime;
	Timer t;
public:
	regularC_ms(unsigned long intervalArg,unsigned long start=0):
	interval(intervalArg)
	{
		t.start();
		nextTime=start;
	}
	bool ist(){
		if(nextTime<(unsigned long)t.read_ms()){
			nextTime=interval+t.read_ms();
			return true;
		}else{
			return false;
		}
	}
	void set(unsigned long val){interval=val;}
	unsigned long read(){return interval;}
	operator bool(){return ist();}
};

void motor(){
	led=!led;
	if (ingi==2){
		motorR=power;
		motorL=-power;
	}else{
		motorR=-power;
		motorL=power;
	}
			
}

void randomIngi(){
	ingi=1 + rand() % 2;

}


int main(){
	touchR.mode(PullUp);
	touchL.mode(PullUp);
	//touchR.rise(R);//割り込み
	//touchL.rise(L);
	
	pc.baud(9600);
	while (true){
		static regularC_ms sendTime(20);
		if(sendTime){			
			motor();		
			pc.printf("ingi:%dtouchR:%dtouchL:%d\n",ingi,touchR,touchL);
			
		}
		static regularC_ms checkTime(1);
		if(checkTime){	
			if(touchR==1){
				ingi=1;
			}
			else if(touchL==1){
				ingi=2;
			}
		}
		static regularC_ms randomTime(500);
		if(randomTime){
			randomIngi();
		}
	}

	
    return 0;
}