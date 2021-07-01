#ifndef __A_PID_HPP__
#define __A_PID_HPP__

#include "mbed.h"

namespace rob{
namespace __aPid_internal__{
	template <typename T>
	class delta{
	private:
		T preVal;
		T deltaT;
	public:
		delta(T dt=(T)1):preVal((T)0),deltaT(dt){}
		T f(T val){
			const T ans=(val-preVal)/deltaT;
			preVal=val;
			return ans;
		}
		void reset(){
			preVal=0;
		}
	};
	
	struct pidGain{
		float p;
		float i;
		float d;
		/*pidGain(float pa,float ia,float da):
		p(pa),i(ia),d(da)
		{}*/
	};

	template <typename T>
	class aPid{
	private:
		float KP,KI,KD;
		const T DT;
		T target;
		T preOpe;
		delta<T> dp;
		delta<T> dd1,dd2;
		T maxLimit,minLimit;
	public:
		aPid(float kPArg,float kIArg,float kDArg,float dtArg,T max,T min);
		aPid(const pidGain gain,float dtArg,T max,T min);
		T calc(T val);
		void set(T val){target=val;}
		T read(){return target;}
		void reset();
		void setGain(float kPArg,float kIArg,float kDArg);
		void setGain(const pidGain arg);
	};
	
	template<typename T>
	void aPid<T>::setGain(float kPArg,float kIArg,float kDArg){
		KP=kPArg;
		KI=kIArg;
		KD=kDArg;
	}
	
	template<typename T>
	void aPid<T>::setGain(const pidGain arg){
		setGain(arg.p, arg.i, arg.d);
	}
	
	template<typename T>
	void aPid<T>::reset(){
		target=(T)0;
		preOpe=(T)0;
		dp.reset();
		dd1.reset();
		dd2.reset();
	}
	
	template<typename T>
	aPid<T>::aPid(float kPArg,float kIArg,float kDArg,float dtArg,T max,T min):
	KP(kPArg),KI(kIArg),KD(kDArg),DT(dtArg),
	dp(DT),dd1(DT),dd2(DT),
	maxLimit(max),minLimit(min)
	{
		reset();
	}
	
	template<typename T>
	aPid<T>::aPid(const pidGain gain,float dtArg,T max,T min):
	DT(dtArg),dp(DT),dd1(DT),dd2(DT),
	maxLimit(max),minLimit(min)
	{
		setGain(gain);
		reset();
	}

	template<typename T>
	T aPid<T>::calc(T val){
		T diff=target-val;
		T deltaOpe,ope;
		
		deltaOpe=
		KP*dp.f(diff)+
		KI*diff+
		KD*dd1.f(dd2.f(diff));
		
		ope=deltaOpe+preOpe;
		if(ope>maxLimit){
			ope=maxLimit;
		}else if(ope<minLimit){
			ope=minLimit;
		}else{
			ope=ope;
		}
		preOpe=ope;
		return ope;
	}
}
using __aPid_internal__::aPid;
using __aPid_internal__::pidGain;

}

#endif