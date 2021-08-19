/**
 * @author Aaron Berk
 *
 * @section LICENSE
 *
 * Copyright (c) 2010 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Quadrature Encoder Interface.
 *
 * A quadrature encoder consists of two code tracks on a disc which are 90
 * degrees out of phase. It can be used to determine how far a wheel has
 * rotated, relative to a known starting position.
 *
 * Only one code track changes at a time leading to a more robust system than
 * a single track, because any jitter around any edge won't cause a state
 * change as the other track will remain constant.
 *
 * Encoders can be a homebrew affair, consisting of infrared emitters/receivers
 * and paper code tracks consisting of alternating black and white sections;
 * alternatively, complete disk and PCB emitter/receiver encoder systems can
 * be bought, but the interface, regardless of implementation is the same.
 *
 *               +-----+     +-----+     +-----+
 * Channel A     |  ^  |     |     |     |     |
 *            ---+  ^  +-----+     +-----+     +-----
 *               ^  ^
 *               ^  +-----+     +-----+     +-----+
 * Channel B     ^  |     |     |     |     |     |
 *            ------+     +-----+     +-----+     +-----
 *               ^  ^
 *               ^  ^
 *               90deg
 *
 * The interface uses X2 encoding by default which calculates the pulse count
 * based on reading the current state after each rising and falling edge of
 * channel A.
 *
 *               +-----+     +-----+     +-----+
 * Channel A     |     |     |     |     |     |
 *            ---+     +-----+     +-----+     +-----
 *               ^     ^     ^     ^     ^
 *               ^  +-----+  ^  +-----+  ^  +-----+
 * Channel B     ^  |  ^  |  ^  |  ^  |  ^  |     |
 *            ------+  ^  +-----+  ^  +-----+     +--
 *               ^     ^     ^     ^     ^
 *               ^     ^     ^     ^     ^
 * Pulse count 0 1     2     3     4     5  ...
 *
 * This interface can also use X4 encoding which calculates the pulse count
 * based on reading the current state after each rising and falling edge of
 * either channel.
 *
 *               +-----+     +-----+     +-----+
 * Channel A     |     |     |     |     |     |
 *            ---+     +-----+     +-----+     +-----
 *               ^     ^     ^     ^     ^
 *               ^  +-----+  ^  +-----+  ^  +-----+
 * Channel B     ^  |  ^  |  ^  |  ^  |  ^  |     |
 *            ------+  ^  +-----+  ^  +-----+     +--
 *               ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
 *               ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
 * Pulse count 0 1  2  3  4  5  6  7  8  9  ...
 *
 * It defaults
 *
 * An optional index channel can be used which determines when a full
 * revolution has occured.
 *
 * If a 4 pules per revolution encoder was used, with X4 encoding,
 * the following would be observed.
 *
 *               +-----+     +-----+     +-----+
 * Channel A     |     |     |     |     |     |
 *            ---+     +-----+     +-----+     +-----
 *               ^     ^     ^     ^     ^
 *               ^  +-----+  ^  +-----+  ^  +-----+
 * Channel B     ^  |  ^  |  ^  |  ^  |  ^  |     |
 *            ------+  ^  +-----+  ^  +-----+     +--
 *               ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
 *               ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
 *               ^  ^  ^  +--+  ^  ^  +--+  ^
 *               ^  ^  ^  |  |  ^  ^  |  |  ^
 * Index      ------------+  +--------+  +-----------
 *               ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
 * Pulse count 0 1  2  3  4  5  6  7  8  9  ...
 * Rev.  count 0          1           2
 *
 * Rotational position in degrees can be calculated by:
 *
 * (pulse count / X * N) * 360
 *
 * Where X is the encoding type [e.g. X4 encoding => X=4], and N is the number
 * of pulses per revolution.
 *
 * Linear position can be calculated by:
 *
 * (pulse count / X * N) * (1 / PPI)
 *
 * Where X is encoding type [e.g. X4 encoding => X=44], N is the number of
 * pulses per revolution, and PPI is pulses per inch, or the equivalent for
 * any other unit of displacement. PPI can be calculated by taking the
 * circumference of the wheel or encoder disk and dividing it by the number
 * of pulses per revolution.
 */

/**
 * Includes
 */
 // +-------------+
// | X2 Encoding |
// +-------------+
//
// When observing states two patterns will appear:
//
// Counter clockwise rotation:
//
// 10 -> 01 -> 10 -> 01 -> ...
//
// Clockwise rotation:
//
// 11 -> 00 -> 11 -> 00 -> ...
//
// We consider counter clockwise rotation to be "forward" and
// counter clockwise to be "backward". Therefore pulse count will increase
// during counter clockwise rotation and decrease during clockwise rotation.
//
// +-------------+
// | X4 Encoding |
// +-------------+
//
// There are four possible states for a quadrature encoder which correspond to
// 2-bit gray code.
//
// A state change is only valid if of only one bit has changed.
// A state change is invalid if both bits have changed.
//
// Clockwise Rotation ->
//
//    00 01 11 10 00
//
// <- Counter Clockwise Rotation
//
// If we observe any valid state changes going from left to right, we have
// moved one pulse clockwise [we will consider this "backward" or "negative"].
//
// If we observe any valid state changes going from right to left we have
// moved one pulse counter clockwise [we will consider this "forward" or
// "positive"].
//
// We might enter an invalid state for a number of reasons which are hard to
// predict - if this is the case, it is generally safe to ignore it, update
// the state and carry on, with the error correcting itself shortly after.

#include "QEI.h"
QEI::QEI(const QEI& q) : channelA_(q.Pin[0]), channelB_(q.Pin[1]),index_(q.Pin[2])
{
    pulses_       = 0;
    revolutions_  = 0;
    pulsesPerRev_ = q.pulsesPerRev_;
    encoding_     = q.encoding_;
    timer=q.timer;
    //Workout what the current state is.
    int chanA = channelA_.read();
    int chanB = channelB_.read();

    //2-bit state.
    currState_ = (chanA << 1) | (chanB);
    prevState_ = currState_;

    channelA_.rise(this, &QEI::encode);
    channelA_.fall(this, &QEI::encode);


    if (q.encoding_ == X4_ENCODING) {
        channelB_.rise(this, &QEI::encode);
        channelB_.fall(this, &QEI::encode);
    }
    if (Pin[2] !=  NC) {
        index_.rise(this, &QEI::index);
    }
}
QEI::QEI(PinName channelA,
         PinName channelB,
         PinName index,
         int pulsesPerRev,
         Timer *T,
         Encoding encoding
        ) : channelA_(channelA), channelB_(channelB),
    index_(index)
{
    timer=T;
    Pin[0] = channelA;
    Pin[1] = channelB;
    Pin[2] = index;
    pulses_       = 0;
    revolutions_  = 0;
    pulsesPerRev_ = pulsesPerRev;
    encoding_     = encoding;

    //Workout what the current state is.
    int chanA = channelA_.read();
    int chanB = channelB_.read();

    //2-bit state.
    currState_ = (chanA << 1) | (chanB);
    prevState_ = currState_;

    channelA_.rise(this, &QEI::encode);
    channelA_.fall(this, &QEI::encode);


    if (encoding == X4_ENCODING) {
        channelB_.rise(this, &QEI::encode);
        channelB_.fall(this, &QEI::encode);
    }
    if (index !=  NC) {
        index_.rise(this, &QEI::index);
    }

}
void QEI::state(int i)
{
    if(i==1) {
        channelA_.disable_irq();
        channelB_.disable_irq();
    } else if(i==0) {
        channelA_.enable_irq();
        channelB_.enable_irq();
    }
}
void QEI::qei_reset(void)
{

    pulses_      = 0;
    revolutions_ = 0;
    round_rev = 0;
    sumangle = angle_ =0;
}
void QEI::set(int pul , int rev)
{

    pulses_      = pul;
    revolutions_ = rev;

}
int QEI::getCurrentState(void)
{

    return currState_;

}

int QEI::getPulses(void)
{

    return pulses_;

}

int QEI::getRevolutions(void)
{

    return revolutions_;

}
double QEI::getAngle()
{
    return angle_;
}
int QEI::getAng_rev()
{
    return round_rev;
}
double QEI::getSumangle()
{
    return sumangle;
}

double QEI::getRPM()
{
    static double prev_angle;

    RPM = (sumangle - prev_angle) / gettime() * 60.0 / 360;

    prev_angle = sumangle;
    return RPM;
}
double QEI::getRPS()
{
    static double prev_angle;

    RPS = (sumangle - prev_angle) / gettime() / 360;
    prev_angle = sumangle;
    return RPS;
}
double QEI::getRPMS()
{
    static double prev_angle;

    RPMS = (sumangle - prev_angle) / gettime()*1000/ 360;
    prev_angle = sumangle;
    return RPMS;
}
double QEI::getRPUS()
{
    static double prev_angle;
    RPUS =(sumangle - prev_angle) / gettime()*1000*1000/ 360;
    prev_angle = sumangle;
    prev_angle = sumangle;
    return RPUS;
}
void QEI::encode(void)
{
    int change = 0;
    int chanA  = channelA_.read();
    int chanB  = channelB_.read();
    //printf("QEI\n");
    currState_ = (chanA << 1) | (chanB);

    if (encoding_ == X2_ENCODING) {

        if ((prevState_ == 0x3 && currState_ == 0x0) ||
                (prevState_ == 0x0 && currState_ == 0x3)) {

            pulses_++;
            angle_pulses++;

        } else if ((prevState_ == 0x2 && currState_ == 0x1) ||
                   (prevState_ == 0x1 && currState_ == 0x2)) {

            pulses_--;
            angle_pulses--;

        }

    } else if (encoding_ == X4_ENCODING) {

        if (((currState_ ^ prevState_) != INVALID) && (currState_ != prevState_)) {
            change = (prevState_ & PREV_MASK) ^ ((currState_ & CURR_MASK) >> 1);

            if (change == 0) {
                change = -1;
            }

            pulses_ -= change;
            angle_pulses -= change;
        }

    }
    angle_ = angle_pulses*360/((double)pulsesPerRev_*4);
    sumangle = pulses_*360/((double)pulsesPerRev_*4);
    if(angle_>=360) {
        angle_pulses = angle_ = 0;
        round_rev++;
    } else if(angle_<=-360) {
        angle_pulses = angle_ = 0;
        round_rev--;
    }
    prevState_ = currState_;
}

void QEI::index(void)
{

    revolutions_++;

}

