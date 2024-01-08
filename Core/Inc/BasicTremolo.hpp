#ifndef __BASICTREMOLO_HPP__
#define __BASICTREMOLO_HPP__


#include <cmath>

//Define the enums for setting maybe?


class TREMBlock {


  float depth; //0..100
  float rate;
  float levOffset = 0; //init level
  float LFOstep = 0; //init timestep
  float Tau = 1/48000.0;
  float LFOdepth;
  float LFOval;


//access specifier
public:
  // constructor built by compiler
  TREMBlock();

  float process(float val);

  void setRate(float TargetRate);

  void setDepth(float TargetDepth);

 };

#endif
