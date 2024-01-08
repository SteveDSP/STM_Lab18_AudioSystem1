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

	LFOdepth = depth/200.0; //so if depth is 0..100, then will go from 0..0.5 depending on UI, this could be removed
    levOffset = 1.0-LFOdepth;
    LFOval = LFOdepth*sin(2*M_PI*rate*LFOstep)+levOffset;
    LFOstep=(LFOstep+Tau);
    if(LFOstep>1)LFOstep=0;
    float processedSample = LFOval*val;

    return processedSample;

}

void TREMBlock::setRate(float TargetRate){
	rate = TargetRate;
}


void TREMBlock::setDepth(float TargetDepth){
	 depth = TargetDepth;
 }




