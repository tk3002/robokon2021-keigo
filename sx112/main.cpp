//片持ち改善　直結化　作って置き換え　らいたー 共用体 通信速度上げる 微分　20ms  チャリも VNC com指定の自動化
//43コマ*240fps=0.18s  2.2m*0.18s=12.2m/s

#include "mbed.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "pid.h"
#include "qei.h"
#include "aTB6643KQ.hpp"

#define SG_PERIOD 20     //20[msec] 
#define SG_NEUTRAL 1475 // 0° の時に1500usのパルスを出す
#define resolution 2048 //分解能設定
//Timer millis;//時間のカウント
#define getxservoAngle 100
#define motorPowercorrec 0.01
#define plusPower 0.01


Serial pc(USBTX, USBRX); //初期化　usbの送信usbtx usbの受信usbrx ピンも設定可
Serial ard(PA_0, PA_1);
DigitalOut led(LED1);





QEI wheelx(PA_11, PC_4, PB_15, resolution, 0, QEI::X4_ENCODING);//ロータリーエンコーダー
QEI wheely(PA_8, PA_9, PB_13, resolution, 0, QEI::X4_ENCODING);
rob::aTB6643KQ motorX(PB_3, PA_10);//もたどら３
rob::aTB6643KQ motorY(PB_5, PB_10);//もたどら４
DigitalIn swich_shot(PC_3);

const int BUF_LEN = 255;
uint8_t buf[BUF_LEN] = {};//文字列を入れるための配列
int bufIndex = 0; //次に書き込む配列の場所をつかさどる
float val = 0.0;
float xyz[3]={0.1,0.1,3};
float xmotorAngle = 0;
float ymotorAngle = 0;
int xmotorRevo = 0;
int ymotorRevo = 0;
int i = 0;
int ingi = 1;
float xservoAngle=0;
//float xservoAngleArray[20000];
float xservoAngleDiff = 0;
float xservoSpeed = 0;
float xmotorSetAngle = 0;
float ymotorSetAngle=0;
float yservoAngle=0;
float xmotorPower=0;
float ymotorPower=0;
float xArcsin=0;
float lastxArcsin=0;
float yArcsin=0;
float lastyArcsin=0;
char willsend[11];

static rob::aPid<float> xPid(0.35, 0.1, 0.00, 0.02, 90.0, -90.0);//pidの設定　時間引数は同じ値を入れる
static rob::aPid<float> yPid(0.2, 0.1, 0.00, 0.02, 90.0, -90.0);
static rob::aPid<float> xmotor(0.0003, 0.0003, 0.0, 0.02, 1, -1);
static rob::aPid<float> ymotor(0.0003, 0.0003, 0.0, 0.02, 1, -1);




class regularC_ms {
private:
	unsigned long interval;
	unsigned long nextTime;
	Timer t;
public:
	regularC_ms(unsigned long intervalArg, unsigned long start = 0) :
		interval(intervalArg)
	{
		t.start();
		nextTime = start;
	}
	bool ist() {
		if (nextTime < (unsigned long)t.read_ms()) {
			nextTime = interval + t.read_ms();
			return true;
		}
		else {
			return false;
		}
	}
	void set(unsigned long val) { interval = val; }
	unsigned long read() { return interval; }
	operator bool() { return ist(); }
};






void serialReceiver() {
	const uint8_t receiveByte = (uint8_t)pc.getc();//getcでたまっているのを1文字受信

	if (receiveByte == '\n') {//もし\n（1つの文字列の終わり）なら
		buf[bufIndex] = '\0';//bufの最後に\nを書き込む
		//val=atof((const char*)buf);//bufの文字列をatofで戻し(doubleに変換される)valに書き込む
		bufIndex = 0;//\nを読んだってことは配列の場所をリセットする
		xyz[0] = atof((const char*)buf);
		xyz[1] = atof((const char*)buf + 14);//
		xyz[2] = atof((const char*)buf + 28);
	}
	else {//\nじゃないなら
		if(!(bufIndex<BUF_LEN))return;
		buf[bufIndex] = receiveByte;//bufの指定した場所(bufindex)に1文字書き込む
		bufIndex++;
	}
	if(xyz[0]==0){
		xyz[0]=0.001;
	}
	if(xyz[1]==0){
		xyz[0]=0.001;
	}
	if(xyz[2]==0){
		xyz[0]=0.001;
	}
	return;
}





void servo() {

	xservoAngle = 0;
	yservoAngle = 20;

	sprintf(willsend, "%05.1f%05.1f\n", xservoAngle + 90, yservoAngle + 90);
	ard.printf(willsend);
	//pc.printf(willsend);
	return;

}

void motor() {
	xmotorAngle = -wheelx.getAngle();
	xArcsin = asin(xyz[0]/xyz[2]);
	
	if (isnan(xArcsin)) xArcsin=lastxArcsin;
	
	lastxArcsin=xArcsin;	
	xmotorSetAngle = xArcsin * (180 / 3.14);
	
	xmotor.set(xmotorSetAngle);
	xmotorPower = xmotor.calc(xmotorAngle);
	if (xmotorPower > motorPowercorrec) {//パワー入れても動かない分のブースト
		xmotorPower = xmotorPower + plusPower;
	}
	else if (xmotorPower <= -motorPowercorrec) {
		xmotorPower = xmotorPower - plusPower;
	}
	motorX = -xmotorPower;
	//xservoAngleArray[i] = xservoAngle;


	ymotorAngle = -wheely.getAngle();
	yArcsin=-asin(xyz[1]/xyz[2]);
	if (isnan(yArcsin)!=0) yArcsin=lastyArcsin;
	lastyArcsin=yArcsin;
	ymotorSetAngle = (yArcsin * (180 / 3.14)+23);
	ymotor.set(ymotorSetAngle);
	ymotorPower = ymotor.calc(ymotorAngle);
	if (ymotorPower > motorPowercorrec) {
		ymotorPower = ymotorPower + plusPower;
	}
	else if (ymotorPower <= -motorPowercorrec) {
		ymotorPower = ymotorPower - plusPower;
	}
	motorY = -ymotorPower;
	return;
}

void off(){
	xservoAngle = 0;
	yservoAngle = 5;
	motorX = 0;
	ymotorAngle = -wheely.getAngle();
	ymotor.set(90);
	motorY = ymotor.calc(ymotorAngle);
	sprintf(willsend, "%05.1f%05.1f\n", xservoAngle + 90, yservoAngle + 90);
	ard.printf(willsend);
	
}


void setup() {
	/*
	for(;wheelx.getRevolutions()<1;){
		static regularC_ms xmotorRevoTime(500);
		if(xmotorRevoTime){
		pc.printf("xrevo%d\n",wheelx.getRevolutions());
		}
		motorX=-0.1;


	}

	motorX=0;
	wheelx.qei_reset();
	pc.printf("xAngleSub%f\n",wheelx.getAngle());

	for(;wheelx.getAngle()>0;){
		static regularC_ms xmotorAngleTime(500);
		if(xmotorAngleTime){
		pc.printf("xAngle%f\n",wheelx.getAngle());
		}
		motorX=-0.1;
	}
	motorX=0;
	*/


	for (; wheely.getRevolutions() < 1;) {

		motorY = 0.1;
		static regularC_ms yrevo(500);
		if (yrevo) {
			pc.printf("yrevo%d\n", wheely.getRevolutions());
		}
	}

	motorY = 0;
	return;
}

void check(){
	if(swich_shot==1){
		ingi = 1;
		led = 1;

	}
	
	else if(swich_shot==0){
		//ingi = 0;
		led = 0;
		
	}
}



int main() {

	wheelx.qei_reset();
	wheely.qei_reset();

	motorY.freq(2000);//周波数の設定　単位Hz
	motorX.freq(2000);

	pc.attach(serialReceiver, Serial::RxIrq);//serialreciverを読み出し 1byte読み込み終わったら関数（必ず受ける）の呼び出し	

	swich_shot.mode(PullNone);

	pc.baud(115200);
	ard.baud(115200);

	xPid.set(0.0);//目標値
	yPid.set(0.0);

	//setup();

	while (true) {
		//led=!led;
		static regularC_ms checkTime(1);
		if(checkTime){	
			check();			
		}
		
		static regularC_ms lockTime(20);
		if (lockTime) {
			if (ingi == 1) {
				servo();
				motor();
				
			}else {
				off();
			}
		}
		static regularC_ms printTime(100);
		if (printTime) {
			//pc.printf("xyz%d %f xservoAngle:%f yservoAngle:%f\n",0,xyz[0],xservoAngle,yservoAngle); //printfで文字列の出力
			pc.printf("xmotorSetAngle :%fxmotorAngle:%+06.2fxmotorPower:%lfymotorSetAngle:%+06.2fymotorAngle:%+06.2fymotorPower:%+07.4f \n", xmotorSetAngle, xmotorAngle, xmotorPower, ymotorSetAngle, ymotorAngle, ymotorPower);
			//pc.printf("x:%fy:%fz:%f\n",xyz[0],xyz[1],xyz[2]);
		}
		
	}
	return 0;
}

//+00.00+00.00+0.0000+00.00+00.00+0.0000







