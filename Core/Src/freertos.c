/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


#include "AudioProcess.hpp" //include the CPP hook



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */



/* USER CODE END Variables */
osThreadId audioTaskHandle;
osThreadId auxTaskHandle;
osThreadId myFlashAliveTasHandle;
osMessageQId audioQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void BufferHandling(void); //Called by audio thread to deal with messages and then call copy buffer etc.
void CopyBuffer(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BlockSize);
/* USER CODE END FunctionPrototypes */

void StartAudioTask(void const * argument);
void auxTaskStart(void const * argument);
void StartFlashAliveTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */



  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of audioQueue */
  osMessageQDef(audioQueue, 16, uint8_t);
  audioQueueHandle = osMessageCreate(osMessageQ(audioQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of audioTask */
  osThreadDef(audioTask, StartAudioTask, osPriorityHigh, 0, 128);
  audioTaskHandle = osThreadCreate(osThread(audioTask), NULL);

  /* definition and creation of auxTask */
  osThreadDef(auxTask, auxTaskStart, osPriorityIdle, 0, 128);
  auxTaskHandle = osThreadCreate(osThread(auxTask), NULL);

  /* definition and creation of myFlashAliveTas */
  osThreadDef(myFlashAliveTas, StartFlashAliveTask, osPriorityIdle, 0, 128);
  myFlashAliveTasHandle = osThreadCreate(osThread(myFlashAliveTas), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartAudioTask */
/**
  * @brief  Function implementing the audioTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartAudioTask */
void StartAudioTask(void const * argument)
{
  /* USER CODE BEGIN StartAudioTask */

  /* Infinite loop */
  for(;;)
  {
	BufferHandling();
    osDelay(1);
  }
  /* USER CODE END StartAudioTask */
}

/* USER CODE BEGIN Header_auxTaskStart */
/**
* @brief Function implementing the auxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_auxTaskStart */
void auxTaskStart(void const * argument)
{
  /* USER CODE BEGIN auxTaskStart */


  /* Infinite loop */
  for(;;)
  {
	  //HAL_GPIO_TogglePin(GPIOJ, ARD_D4_Pin); //Signify the task is running
      //We can read things and based on wha we read, change Globals
	  ts_status = BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected){


		  //Due to the capacitive nature, sometimes spurious touches
		  //are detected..so watch this with traps.

		  /* Get X and Y position of the first touch post calibrated */
		  uint16_t x1 = TS_State.touchX[0];
		  uint16_t y1 = TS_State.touchY[0];

		  //Need to check these values in case of spurious values
		  //limit with a mod instruction for example
		  x1 = x1 % 800;
		  y1 = y1 % 480;

		  coeffOne = (float)(480-y1)*coeffOne_inc;
		  coeffTwo = (float)(800-x1)*coeffTwo_inc;

		  //For the Tremolo, interesting effects, 1000 and 2 for below. :)
		  coeffOne = coeffOne * 200; // test mod depth 0..200
		  coeffTwo = coeffTwo * 20;  // test rate 0..20



		  //Check we're still in the audio thread here ..use the LEDS as we're back in main..
		  UpdateObjects(); //This allows it to be called separately now..and also only call if a control has changed

      }

      //BSP_LCD_FillRect(100, 200, 10, 100);

      osDelay(10); //this will slow the scan time of the TS
 }
  /* USER CODE END auxTaskStart */
}

/* USER CODE BEGIN Header_StartFlashAliveTask */
/**
* @brief Function implementing the myFlashAliveTas thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFlashAliveTask */
void StartFlashAliveTask(void const * argument)
{
  /* USER CODE BEGIN StartFlashAliveTask */
  /* Infinite loop */


	HAL_GPIO_WritePin(GPIOJ, ARD_D7_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, ARD_D6_Pin, GPIO_PIN_SET );
	HAL_GPIO_WritePin(GPIOC, ARD_D5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOJ, ARD_D4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOF, ARD_D3_Pin,  GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOJ, ARD_D2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, ARD_D1_Pin,  GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, ARD_D0_Pin,  GPIO_PIN_SET);

	for(;;)
	{
		BSP_LED_Toggle(LED_GREEN);//Flash on main board
		//printf("%d\r\n", CurrentLsample);

		//int16_t scaledSample = (abs(CurrentLsample)/10); //gives potentially 100 length
		//printf("%d\r\n", scaledSample);


		//BSP_LCD_FillRect(350, 200, 100, 100);

		//Basic Audio Monitor..

		//flash all the other leds..
		HAL_GPIO_TogglePin(GPIOJ, ARD_D7_Pin);
		HAL_GPIO_TogglePin(GPIOF, ARD_D6_Pin);
		HAL_GPIO_TogglePin(GPIOC, ARD_D5_Pin);
		HAL_GPIO_TogglePin(GPIOJ, ARD_D4_Pin);
		HAL_GPIO_TogglePin(GPIOF, ARD_D3_Pin);
		HAL_GPIO_TogglePin(GPIOJ, ARD_D2_Pin);
		HAL_GPIO_TogglePin(GPIOC, ARD_D1_Pin);
		HAL_GPIO_TogglePin(GPIOC, ARD_D0_Pin);

		osDelay(100);
  }
  /* USER CODE END StartFlashAliveTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/*Handle Buffers..
   * Will get called at half and full buffer point so..can copy the contents into
   * L & R buffer then process so long as the copy is quick enough..
   *
   */
 void CopyBuffer(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BlockSize)
  {

 	 /*
 	  * pbuffer2 is the source
		 	pbuffer1 is the dest
		 	So, always process the source to the destination :)
 	  * The DMA transfers in BYTES and is configured in BYTES so
 	  * it's important when considering indexing and buffer sizes
 	  * An SAI frame is made up of 8  bytes
 	  * Int16  Int16 Int16  Iint16
 	  * This allows transferral of varying sizes of data
 	  * In our case
 	  * L - 1st Int16
 	  * -
 	  * R   3rd Int16
 	  * -
 	  * Our Sample Size is 2 Bytes - Int16
 	  *
 	  *  pbuffer2 is the source
		 pbuffer1 is the dest
 	 	 So..at the point this function is called,  the 'other half' of the buffer is being filled and output
 	 	 therefore we can copy the DMA buffer into a processing buffer (as the DMA is 8  bytes)
 	 	 and then process before copying back
 	 	 So, 2 functions are required, 1 to extract the samples, the  other to return them to the buffer
 	  *
 	  *
 	  *
 	  *
 	  */

     //Lets clear the entire destination playback DMA buffer here..
 	 //We're just clearing  because if we operate only on the left or right
 	 //the  data from  the last block will still be there..
	 //If we're operating on both, this can be removed

 	 for (uint32_t i = 0; i < BlockSize; i++)
 	 {
 		 pbuffer1[i] = 0; //needs  to  be careful as these are still int16_t buffers when we do maths
 	 }

     //Call AudioProcess CPP Access ...

 	 /*Try copy into larger circ buffer here...
 	 * Then AudioProcess would be passed the current read pointer (start of where the input block is in the
 	 * circular buffer, block size, and the
 	 * current output pointer to Playback Buffer, the latter
 	 * being a flip flop between Playback[0] and Playback[size/2]
 	 */

 	 AudioProcess( pbuffer1, pbuffer2, BlockSize);
 	 //This allows audio objects to be C callable defined through AudioProcess.hpp/cpp






} //Copy Buffer function



void BufferHandling(void){

     /* 1st or 2nd half of the record buffer ready for being copied to the Playback buffer */

	osEvent retvalue  = osMessageGet( audioQueueHandle, 1000 );
	uint16_t buffer_state = retvalue.value.v;

    switch(buffer_state){
    	case BUFFER_OFFSET_HALF:{
    		//HAL_GPIO_WritePin(GPIOJ, ARD_D7_Pin, GPIO_PIN_SET); //Indicate rate of half filling buffer
    		//Ready to copy first block of audio samples to output buffer
        	CopyBuffer(&PlaybackBuffer[0],
        	&RecordBuffer[0],
        	RECORD_BUFFER_SIZE / 2);
        	audio_rec_buffer_state = BUFFER_OFFSET_NONE;  //Reset
        	//HAL_GPIO_TogglePin(GPIOC, ARD_D5_Pin);
        	//HAL_GPIO_WritePin(GPIOJ, ARD_D7_Pin, GPIO_PIN_RESET); //Indicate rate of half filling buffer
        	break;
        	}
    	case BUFFER_OFFSET_FULL:{
    		//Ready to copy next block of audio samples to output buffer
    		 CopyBuffer(&PlaybackBuffer[RECORD_BUFFER_SIZE / 2],
        	 &RecordBuffer[RECORD_BUFFER_SIZE / 2],
        	 RECORD_BUFFER_SIZE / 2);
        	 audio_rec_buffer_state = BUFFER_OFFSET_NONE; //Reset
        	 //HAL_GPIO_TogglePin(GPIOC, ARD_D5_Pin);
        	 break;
        	}
        case BUFFER_OFFSET_NONE:{
        	 //Allow subtask like  scan of TS proc
        	 break;
        	 }
        default : {
        	//printf("Error on audio_rec_buffer_state switch\r\n");
        	Error_Handler();
        	break;
        	}
    	} //End of switch
} // end AUDIO_PROCESS function




//These are the important call back functions, declared as weak in the discovery_audio.h/.c
    //So we can override them as needed
    //As they're in main, we will need to make sure they then can see and can send to the FreeRTOS listener
    //thread
    void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
    {
        //audio_tx_buffer_state = 1;
        //Everything is driven by the receive but we need to have
        //this callback
    }


    /**
      * @brief Manages the DMA Transfer complete interrupt.
      * @param None
      * @retval None
      */
    void BSP_AUDIO_IN_TransferComplete_CallBack(void)
    {
       audio_rec_buffer_state = BUFFER_OFFSET_FULL;
       osMessagePut(audioQueueHandle, audio_rec_buffer_state, 1000); //Send 1
    }


    /**
      * @brief  Manages the DMA Half Transfer complete interrupt.
      * @param  None
      * @retval None
      */
    void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
    {
        audio_rec_buffer_state = BUFFER_OFFSET_HALF;
        osMessagePut(audioQueueHandle, audio_rec_buffer_state, 1000); //Send 1
    }



/* USER CODE END Application */

