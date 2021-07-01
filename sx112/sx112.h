#ifndef SX112_H
#define SX112_H
#include "mbed.h"
#include "PwmOut.h"
 
class sx112: public PwmOut
{
public:
    /* コンストラクタ */
    sx112( PinName  port ):PwmOut( port ){
	}
    /* 初期化処理 */
    void init();//プロトタイプ宣言
    /* 回転処理 */
    int  roll( float angle );
};
 
#endif