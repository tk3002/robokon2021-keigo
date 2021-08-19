//サーボの皿、
#include "mbed.h"
#include <string.h>

#include "pid.h"
#include "qei.h"
#include "aTB6643KQ.hpp"
#include "aSoftwareServo.hpp"

#define SG_PERIOD 20     //20[msec] 
#define SG_NEUTRAL 1475 // 0° の時に1500usのパルスを出す
#define resolution 2048 //分解能設定
//Timer millis;//時間のカウント



Serial pc(USBTX,USBRX); //初期化　usbの送信usbtx usbの受信usbrx ピンも設定可
DigitalOut led(LED1);


QEI wheel(PA_11,PC_4,D9,resolution,0, QEI::X4_ENCODING);//ロータリーエンコーダー
rob::aTB6643KQ motorX(PB_3,PA_10);//もたどら３
rob::aTB6643KQ motorY(PB_5,PB_10);//もたどら４
rob::SoftwareServo xservo(PB_9);
rob::SoftwareServo yservo(PB_8);

const int BUF_LEN=255;
uint8_t buf[BUF_LEN]={};//文字列を入れるための配列
int bufIndex=0; //次に書き込む配列の場所をつかさどる
float val=0.0;
double xyz[3];
double angle=0;
int revo=0;
int i=0;
float xDeg;
float yDeg;
float xmotorDeg;

static rob::aPid<float> xPid(0.1,0.1,0.0,0.02,90.0,-90.0);//pidの設定　時間引数は同じ値を入れる
static rob::aPid<float> yPid(0.1,0.1,0.0,0.02,90.0,-90.0);
static rob::aPid<float> ymotor(0.0000001,0.00000001,0.0,0.02,1,-1);
static rob::aPid<float> xmotor(0.0000001,0.00000001,0.0,0.02,1,-1);



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






void serialReceiver(){
	const uint8_t receiveByte=(uint8_t)pc.getc();//getcでたまっているのを1文字受信
	
	if(receiveByte=='\n'){//もし\n（1つの文字列の終わり）なら
		buf[bufIndex]='\0';//bufの最後に\nを書き込む
		//val=atof((const char*)buf);//bufの文字列をatofで戻し(doubleに変換される)valに書き込む
		bufIndex=0;//\nを読んだってことは配列の場所をリセットする
		xyz[0]=atof((const char*)buf);
		xyz[1]=atof((const char*)buf+14);//
		xyz[2]=atof((const char*)buf+28);
	}else{//\nじゃないなら
		buf[bufIndex]=receiveByte;//bufの指定した場所(bufindex)に1文字書き込む
		bufIndex++;
	}
	return;
}





void servo(){
	
	xDeg=xPid.calc(-xyz[0]);
	xservo.roll(xDeg);
	yDeg=yPid.calc(-xyz[1]);	
	yservo.roll(yDeg);
	
	
	return ;
	
}

void rotary(){
	angle=wheel.getAngle();
	revo=wheel.getRevolutions();
	
	xmotor.set(xDeg);
	xmotorDeg=xmotor.calc(angle);
	//motorX=-1;
	motorY=xmotorDeg;
	motorX=xmotorDeg;

	return;
}



int main(){
	xservo.init();
	yservo.init();
	
	wheel.qei_reset();
	motorY.freq(2000);//周波数の設定　単位Hz
	motorX.freq(2000);
	
	pc.attach(serialReceiver,Serial::RxIrq);//serialreciverを読み出し 1byte読み込み終わったら関数（必ず受ける）の呼び出し	
	
	pc.baud(115200);	
	
	xPid.set(0.0);//目標値
	yPid.set(0.0);
	
	while (true){
		static regularC_ms lockTime(20);
		if(lockTime){
			servo();
			led=!led;
		}
		
		static regularC_ms printTime(100);
		if(printTime){
			//pc.printf("xyz%d %f xDeg:%f yDeg:%f\n",0,xyz[0],xDeg,yDeg); //printfで文字列の出力
			pc.printf("angle%fxDeg%fxmotorDeg%f yDeg:%f \n",angle,xDeg,xmotorDeg,yDeg);
		}
		rotary();
	
	}
	return 0;
}









