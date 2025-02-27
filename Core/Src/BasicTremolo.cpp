/*
 * BasicTremolo.cpp
 *
 *  Created on: 11 Jul 2023
 *      Author: Steve
 */

#include "BasicTremolo.hpp"

#include <cmath>

TREMBlock::TREMBlock()
  {
    depth = 100.0; //50% depth
    rate = 1.0;   //1Hz

}



//inside class definition
float TREMBlock::process(float val){
    // y(n) = x(n) * ( (1-d) + d*mod) )
	LFOdepth = depth/200.0; //so if depth is 0..100, then will go from 0..0.5 depending on UI,

	//this could be removed, either way need to limit depth to 0..1
    levOffset = 1.0-LFOdepth;

    if (levOffset>=1)
      levOffset = 1.0;

    if (levOffset<=0) //just in case
	  levOffset=0.0;

    //rate needs to be in integers of 2*M_PI to ensure
    //one complete f cycle.

    if (rate<=1) rate=1; //just trap as we don't want mute

    LFOval = LFOdepth*sin(2*M_PI*(int)rate*LFOstep)+levOffset;

    float processedSample = LFOval*val;

    LFOstep=(LFOstep+Tau);
    if(LFOstep>=1)LFOstep=0;

    return processedSample;

}

void TREMBlock::setRate(float TargetRate){
	rate = TargetRate;
}


void TREMBlock::setDepth(float TargetDepth){
	 depth = TargetDepth;
 }




