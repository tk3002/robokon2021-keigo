//rollに定数を入れる　親のクラスで実行してみる
#include "mbed.h"
#include <string.h>

#include "pid.h"

#define SG_PERIOD 20 
#define SG_NEUTRAL 0.07375  //0.07375→0 0.031→－90 0.1165→90
  


PwmOut  xservo(D2);//データ線の指定
PwmOut  yservo(D4);
DigitalOut led(LED1);

Serial pc(USBTX,USBRX); //初期化　usbの送信usbtx usbの受信usbrx ピンも設定可


const int BUF_LEN=255;
uint8_t buf[BUF_LEN]={};//文字列を入れるための配列
int bufIndex=0; //次に書き込む配列の場所をつかさどる
float val=0.0;
double xyz[2];
int i=0;
float xDeg;
float yDeg;

static rob::aPid<float> xPid(0.1,0.1,0.0,0.02,90,-90);//pidの設定　時間引数は同じ値を入れる
static rob::aPid<float> yPid(0.1,0.1,0.0,0.02,90,-90);


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
		xyz[1]=atof((const char*)buf+14);
		xyz[2]=atof((const char*)buf+28);
	}else{//\nじゃないなら
		buf[bufIndex]=receiveByte;//bufの指定した場所(bufindex)に1文字書き込む
		bufIndex++;
	}
	return;
}




void servo(){	

	xDeg=xPid.calc(-xyz[0]);
    float lPulseWidth = 0;
	
	if( xDeg < 0 ){
        lPulseWidth =   xDeg  * -9.5/20000;
        lPulseWidth = SG_NEUTRAL - lPulseWidth;    
    } else {
        lPulseWidth = xDeg  * 9.5/20000;
        lPulseWidth = SG_NEUTRAL + lPulseWidth;
    }
	//pc.printf("%f\n",lPulseWidth);
	xservo.write(lPulseWidth);

	
	yDeg=yPid.calc(xyz[1]);
	if( yDeg < 0 ){
        lPulseWidth =   yDeg  * -9.5/20000;
        lPulseWidth = SG_NEUTRAL - lPulseWidth;
    } else {
        lPulseWidth = yDeg  * 9.5/20000;
        lPulseWidth = SG_NEUTRAL + lPulseWidth;
    }	
	pc.printf("%f\n",lPulseWidth);
	yservo.write(lPulseWidth);
	
	led=!led;


	return ;
	
}


int main(){
	
	pc.attach(serialReceiver,Serial::RxIrq);//serialreciverを読み出し 1byte読み込み終わったら関数（必ず受ける）の呼び出し	
	
	pc.baud(115200);	
	
	xservo.period_ms(SG_PERIOD);
	yservo.period_ms(SG_PERIOD);
	xPid.set(0.0);//目標値
	yPid.set(0.0);
	
	while (true){
		static regularC_ms lockTime(20);
		if(lockTime){
		servo();
		}
		
		static regularC_ms printTime(200);
		if(printTime){
			pc.printf("xyz%d %f xDeg:%f yDeg:%f\n",0,xyz[0],xDeg,yDeg); //printfで文字列の出力
		}
	}	
	
	
	return 0;
}









