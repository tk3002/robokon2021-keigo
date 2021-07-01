//rollに定数を入れる　親のクラスで実行してみる
#include "mbed.h"
#include <string.h>

#include "pid.h"

#include "sx112.h"

#define SG_PERIOD 20     //20[msec] 
#define SG_NEUTRAL 1475 // 0° の時に1500usのパルスを出す
 
//Timer millis;//時間のカウント

sx112  xservo(PA_11);//データ線の指定
sx112  yservo(PB_2);

Serial pc(USBTX,USBRX); //初期化　usbの送信usbtx usbの受信usbrx ピンも設定可
DigitalOut led(LED1);

const int BUF_LEN=255;
uint8_t buf[BUF_LEN]={};//文字列を入れるための配列
int bufIndex=0; //次に書き込む配列の場所をつかさどる
float val=0.0;
double xyz[2];
int i=0;
float xDeg;
float yDeg;

static rob::aPid<float> xPid(0.1,0.1,0.0,0.02,90.0,-90.0);//pidの設定　時間引数は同じ値を入れる
static rob::aPid<float> yPid(0.1,0.1,0.0,0.02,90.0,-90.0);


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







void sx112::init(){//初期化（一回だけ読む）
    /* 周期の設定を行う */
    period_ms( SG_PERIOD );
    /* 90°に角度調整 */
    pulsewidth_us( SG_NEUTRAL );
}

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

int sx112::roll( float angle ){//エラーを返したいからint？
    int lPulseWidth = 0;
	//パルスは1000usから2000usで設定するので
    /* 回転する角度が0より小 */
    if( angle < 0 ){
        lPulseWidth =   angle  * -9.5;
        lPulseWidth = SG_NEUTRAL - lPulseWidth;
		
    /* 回転する角度が0より大*/
    } else if ( angle >= 0 ){
        lPulseWidth = angle  * 9.5;
        lPulseWidth = SG_NEUTRAL + lPulseWidth;
    /* 上記以外はエラー */
    } else {
        return -1;
    }
    /* ポートに流すパルスの時間を設定する */
    pulsewidth_us( lPulseWidth );
    return 0;

}



void servo(){
	
		
	xDeg=xPid.calc(-xyz[0]);
	xservo.roll(xDeg);
	yDeg=yPid.calc(-xyz[1]);	
	yservo.roll(yDeg);
	
	led=!led;

	
	//wait(1);
	//#xservo.roll(90);
	//wait(1);	
	return ;
	
}


int main(){
	xservo.init();
	yservo.init();
	
	pc.attach(serialReceiver,Serial::RxIrq);//serialreciverを読み出し 1byte読み込み終わったら関数（必ず受ける）の呼び出し	
	
	pc.baud(115200);	
	
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









