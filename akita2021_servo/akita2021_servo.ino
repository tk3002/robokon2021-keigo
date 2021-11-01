#include <Servo.h>
#include <string.h>
#include <SoftwareSerial.h>
//https://ameblo.jp/geotechlab-workshop/entry-12624606387.html

Servo xservo;
Servo yservo;
float xyz[2];
float buf;

//SoftwareSerial Serial1 = SoftwareSerial(7,2)//rx,tx;

void setup() {
  xservo.attach(5);
  yservo.attach(6);
  Serial.begin( 115200 );
  Serial1.begin(115200);

}

void loop() {
  //xyz[0] =90;
  //xyz[1] =90;
  if (Serial1.available() > 0) {
    String str = Serial1.readStringUntil('\n');
    //buf=str.toFloat();
    //Serial.println(buf);
    String str0 = str.substring(0, 5);
    String str1 = str.substring(5);
    xyz[0] = str0.toFloat();
    xyz[1] = str1.toFloat();  
    xservo.writeMicroseconds(710+(9.5*xyz[0]));//増えると時計回り
    yservo.writeMicroseconds(530+(9.5*xyz[1]));//増えると上向き
  
    //Serial.println(xyz[0]);
    //Serial.println(xyz[1]);
  }
  
  
}
