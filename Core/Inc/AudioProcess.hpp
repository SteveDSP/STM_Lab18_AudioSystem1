#ifndef __AUDIOPROCESS_HPP__
#define __AUDIOPROCESS_HPP__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"





//CPP call that's accessible from C without the compile mangling the function call (due to Overloading)
void UpdateObjects();
void AudioProcess(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BufferSize); //make the cpp process visible

#ifdef __cplusplus
}
#endif



#endif
