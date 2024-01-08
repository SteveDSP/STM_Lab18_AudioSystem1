#ifndef __CIRCBUFF_HPP__
#define __CIRCBUFF_HPP__

#include "stdint.h"


class DelayBuffer{
protected:
	int16_t * delayBuffer; //Ptr to buffer
	int index; //points to where the newest sample is
	int size ;

public:

  DelayBuffer(uint16_t size_param){
    delayBuffer = new int16_t[size_param](); //Create and return Ptr to array, () init to 0
    size = size_param;
    index = 0;
    /*An alternative to this is through the use of Vectors. These are dynamic arrays that can grow
     * and shrink and have methods associated with them
     */
  }

  ~DelayBuffer(){
	  delete[] delayBuffer; //Just nice house keeping due to dynamic array creation. Safe.
  }

  void insertSample(const int16_t sample);
  int16_t getDelayedSample(int16_t delay);
};


#endif
