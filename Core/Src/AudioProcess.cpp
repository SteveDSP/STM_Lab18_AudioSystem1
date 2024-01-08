/*
 * AudiProcess.cpp
 *
 *  Created on: 11 Jul 2023
 *      Author: Steve
 */

#include "AudioProcess.hpp" //This interface
#include "BasicTremolo.hpp" //Time based basic tremolo


TREMBlock myTremoloL; //
TREMBlock myTremoloR; //



const float INT16_TO_FLOAT = 1.0f / 32768.0f;

float timeStep = 1/48000.0;
uint16_t timep = 0;
uint16_t previousTimep = 0;


//Coeff update funtion

void UpdateObjects(){


	 myTremoloL.setDepth(coeffOne); //Interestign effect if you mod Depth of only one side. :)
	 myTremoloR.setDepth(coeffOne);

	 myTremoloL.setRate(coeffTwo);
	 myTremoloR.setRate(coeffTwo);



}

//Main CPP based audio processing loop

void AudioProcess(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BlockSize){

	// L R L R ..Recall
	/*
	 *  * Int16  Int16 Int16  Iint16
 	  * This allows transferral of varying sizes of data
 	  * In our case
 	  * L - 1st Int16
 	  * -
 	  * R   3rd Int16
 	  * -
 	  * L - 4th Int16
	  * Results in a L float block of 256, Hence blockSize/4
	 */

    // Tremolo


	for (uint16_t i = 0; i < BlockSize; i+=4){ //hop over right bytes because we will process in twos


		float audio_in, audio_out; //working reg
    	audio_in  = INT16_TO_FLOAT * pbuffer2[i];
    	audio_out = myTremoloL.process(audio_in);

    	pbuffer1[i] = (int16_t)(audio_out*32768.0f); //convert back

    	//This gives right

    	audio_in  = INT16_TO_FLOAT * pbuffer2[i+2];
    	audio_out = myTremoloR.process(audio_in);

    	pbuffer1[i+2] = (int16_t)(audio_out*32768.0f); //convert back
    }


}






