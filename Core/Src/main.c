/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "dfsdm.h"
#include "dma2d.h"
#include "dsihost.h"
#include "ltdc.h"
#include "quadspi.h"
#include "sai.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "test_image_splash.h"

#include <string.h>
#include <stdio.h>




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

/* USER CODE BEGIN PV */

/* Private variables ---------------------------------------------------------*/
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

static __IO int32_t  front_buffer   = 0;
static __IO int32_t  pend_buffer   = -1;





extern SAI_HandleTypeDef haudio_out_sai, haudio_in_sai; //Can find this type def declared elsewhere

/* Buffer containing the PCM samples coming from the line in/analogue mic */
int16_t RecordBuffer[RECORD_BUFFER_SIZE];
/* Buffer used to stream the recorded PCM samples towards the audio codec. */
int16_t PlaybackBuffer[RECORD_BUFFER_SIZE];

/*In theory..
 *
 * We have a DMA transfer of a LR blocks, both in and out..
 * In a 1 sample, 1 sample out block we 'write' the 1 sample into a circular buffer d[n], then
 * 'read' it out from a delayed location d[n-k], incrementing n per sample
 *
 * In a block transfer system, we can do the same so long as the circular buffer is big enough to hold
 * the incoming blocks of samples. So long as we always process an output blocks worth (getting d[n-k] bigger
 * than the block size is fine), then we will be dandy.
 *
 */

volatile uint32_t  audio_rec_buffer_state; //current buffer status, see above

//Pointer to AudioDriverType
static AUDIO_DrvTypeDef  *audio_drv;

//Touch Screen status
TS_StateTypeDef TS_State = {0};
uint32_t ts_status = TS_OK;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */


/* Private function prototypes -----------------------------------------------*/
/*


*/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Statics are only accessible with main.c, the others are truly global
//not ideal. They're interfaced using main.h which is included in other modules
//Some would benefit from being local to either their associated functions
//or objects.
static uint32_t LCD_X_Size = 0;
static uint32_t LCD_Y_Size = 0;
float coeffOne = 0.5; //Default float value
float coeffTwo = 0.5; //Default float value
float coeffOne_inc = 1/480.0f; //inc, based on size of LCD
float coeffTwo_inc = 1/800.0f; //Ditto

//int16_t bufferNumber=0; //FlipBuffers

int16_t CurrentLsample, CurrentRsample;


/**
  * @brief  On Error Handler on condition TRUE.
  * @param  condition : Can be TRUE or FALSE
  * @retval None
  */
void OnError_Handler(uint32_t condition)
{
  if(condition)
  {
    BSP_LED_On(LED1);
    while(1) { ; } /* Blocking on error */
  }
}


uint8_t CheckForUserInput(void)
{
	if (BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET) //This will cause hanging..
		return 1;
	else
		return 0;

//	if(BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET)
//  {
//    //while (BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET); //This will cause hanging..
//    return 1 ;
//  }
//  return 0;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  //PeripheralInit(); //THIS IS  AUTOGEN BUT UART IS NOT INCLUDED!!?

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DFSDM1_Init();
  MX_SAI1_Init();
  MX_USART1_UART_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_DSIHOST_DSI_Init();
  MX_FMC_Init();
  MX_QUADSPI_Init();
  /* USER CODE BEGIN 2 */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_On(LED_GREEN);


  //HAL_StatusTypeDef settingReload = HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  //This will set up the reload interrupt and also the
  //This could potentially? be used to flip the display buffer to create animations..possibly


  //ITM_SendChar ('x');

  printf("Connected to STM32F769I-Discovery USART 1\r\n");
  printf("\r\n");


  // Enable access to the qspi flash chip in memory mapped mode

  uint8_t bsp_status = 0;
  bsp_status = BSP_QSPI_Init();
  OnError_Handler(bsp_status != QSPI_OK);
  bsp_status = BSP_QSPI_EnableMemoryMappedMode();
  OnError_Handler(bsp_status != QSPI_OK);
  HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

  // Enable the LCD
  uint8_t  lcd_status = LCD_OK;
  lcd_status = BSP_LCD_Init();
  OnError_Handler(lcd_status != LCD_OK);



  //All these functions send to use draw immediate which is no good for animation
  //Need to use the lower level commands to do this and switch buffer on LCD blanking reload

  //Set Brightness
  BSP_LCD_SetBrightness(100); //0 - 100 Max

  // Get the LCD Width and Height
  LCD_X_Size = BSP_LCD_GetXSize();
  LCD_Y_Size = BSP_LCD_GetYSize();


  // Configure the LCD layers with their framebuffers in SRAM
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS + (800*480*4));
  BSP_LCD_SetColorKeying(1, LCD_COLOR_TRANSPARENT); //This ensures both layers are visible when overlaid?

  // Draw the image on layer 0
  //This sets the 'internal active layer variable that is used by the drawign routines

  BSP_LCD_SelectLayer(0); //Selects 0 as active  layer index ..all draws will go to zero..



  //This bitmap is defined in image_xxx.h and placed during programming in the EEPROM accessed using the QuadSPI
  // Need to ensure its flashed into the QSPI 64M external before running.
  BSP_LCD_DrawBitmap(0, 0, webb_first_f769idisco);


  //CheckForUserInput(); //wait...

  //We need to try to write to the background (not active layer) then switch to that layer..
 // Draw text on layer one and use transparency to make the background image visible

  BSP_LCD_SelectLayer(1); //Makes 1 the active layer

  //BSP_LCD_DrawBitmap(0, 0, webb_first_f769idisco);

  BSP_LCD_Clear(LCD_COLOR_TRANSPARENT); //So we can see the Layer 0 underneath ..important otherwise layer is overlaid with keying.

  BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT); //ditto
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAt(0,LINE(2) , (uint8_t *)"Running Lab 18 1.0", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0,LINE(3) , (uint8_t *)"Audio with Basic Tremolo Effect", CENTER_MODE);

  //BSP_LCD_SelectLayer(1); //Makes 1 the active layer for the box drawing..


  //Will need to move to threads...perhaps?
  ts_status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  //Could do with calibration..


  if(ts_status == TS_OK)
  {
	  /* Display touch screen active debug */
      printf("Touch screen is go!\r\n");
  }
  OnError_Handler(ts_status != TS_OK);



  // Setup
  AUDIO_SETUP();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SAI1;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 3;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */





		/**
       * @brief  Retargets the C library printf function to the USART.
       * @param  None
       * @retval None
       */
     PUTCHAR_PROTOTYPE
     {
       /* Place your implementation of fputc here */
       /* e.g. write a character to the USART2 and Loop until the end of transmission */
       HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
       //ITM_SendChar (ch);

       return ch;
     }

/*
     int _write(int file, char *ptr, int len)
         {
             int i = 0;
             for (i = 0; i<len; i++)
                 ITM_SendChar((*ptr++));
             return len;
         }
*/


     /* ISR Handlers */

     // USE_HAL_DSI_REGISTER_CALLBACKS ??Where is this  defined..

     void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
     {
       (void)hdsi;
       if(pend_buffer >= 0)
       {
         /* Disable DSI Wrapper */
         //__HAL_DSI_WRAPPER_DISABLE(&hdsi_discovery);
         /* Update LTDC configuration */
         //LTDC_LAYER(&hltdc_discovery, 0)->CFBAR = ((uint32_t)Buffers[pend_buffer]);
         //__HAL_LTDC_RELOAD_CONFIG(&hltdc_discovery);
         /* Enable DSI Wrapper */
         //__HAL_DSI_WRAPPER_ENABLE(&hdsi_discovery);

         front_buffer = pend_buffer;
         pend_buffer = -1;
       }
     }



     //override of callback
     void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc){
    	 //bufferNumber++;
    	 //Plot here?
    	 //BSP_LCD_FillRect(350, 200, 100, 100);
     }


     void DMA2_Stream4_IRQHandler(void)
     {
      HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
     }

     void DMA2_Stream1_IRQHandler(void)
     {
      HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
     }


     /*
      * get maximum value of the buffer. VU meter, perhaps?
      */
     void get_max_val(int16_t *buf, uint32_t size, int16_t amp[])
     {
    	 //Look into the structure of this buffer...
         int16_t maxval[4] = { -32768, -32768, -32768, -32768};
         uint32_t idx;
         for (idx = 0 ; idx < size ; idx += 4) {
             if (buf[idx] > maxval[0])
                 maxval[0] = buf[idx];
             if (buf[idx + 1] > maxval[1])
                 maxval[1] = buf[idx + 1];
             if (buf[idx + 2] > maxval[2])
                 maxval[2] = buf[idx + 2];
             if (buf[idx + 3] > maxval[3])
                 maxval[3] = buf[idx + 3];
         }
         memcpy(amp, maxval, sizeof(maxval));
     }





     /**
       * @brief  Audio IN Error callback function.
       * @param  None
       * @retval None
       */
     void BSP_AUDIO_IN_Error_CallBack(void)
     {
         /* This function is called when an Interrupt due to transfer error on or peripheral
            error occurs. */
         /* Display message on the LCD screen */
         //BSP_LCD_SetBackColor(LCD_COLOR_RED);
         //BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"       DMA  ERROR     ", CENTER_MODE);
    	  printf("BSP_AUDIO_IN_Error_CallBack\r\n");

         /* Stop the program with an infinite loop */


         while (BSP_PB_GetState(BUTTON_WAKEUP) != RESET)
         {
             return;
         }


         /* could also generate a system reset to recover from the error */
         /* .... */
     }


 /**
  * @brief  Starts playing audio stream from a data buffer for a determined size.
  * @param  pBuffer: Pointer to the buffer
  * @param  Size: Number of audio data BYTES.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
     void SAIx_In_Init(uint32_t AudioFreq)
     {
         /* Initialize SAI1 block A in MASTER TX */
         /* Initialize the haudio_out_sai Instance parameter */
         haudio_out_sai.Instance = AUDIO_OUT_SAIx;

         /* Disable SAI peripheral to allow access to SAI internal registers */
         __HAL_SAI_DISABLE(&haudio_out_sai);

         /* Configure SAI_Block_x */
         haudio_out_sai.Init.MonoStereoMode = SAI_STEREOMODE;
         haudio_out_sai.Init.AudioFrequency = AudioFreq;
         haudio_out_sai.Init.AudioMode      = SAI_MODEMASTER_TX;
         haudio_out_sai.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
         haudio_out_sai.Init.Protocol       = SAI_FREE_PROTOCOL;
         haudio_out_sai.Init.DataSize       = SAI_DATASIZE_16;
         haudio_out_sai.Init.FirstBit       = SAI_FIRSTBIT_MSB;
         haudio_out_sai.Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;
         haudio_out_sai.Init.Synchro        = SAI_ASYNCHRONOUS;
         haudio_out_sai.Init.OutputDrive    = SAI_OUTPUTDRIVE_ENABLE;
         haudio_out_sai.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
         haudio_out_sai.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
         haudio_out_sai.Init.CompandingMode = SAI_NOCOMPANDING;
         haudio_out_sai.Init.TriState       = SAI_OUTPUT_NOTRELEASED;
         haudio_out_sai.Init.Mckdiv         = 0;

         /* Configure SAI_Block_x Frame */
         haudio_out_sai.FrameInit.FrameLength       = 64;
         haudio_out_sai.FrameInit.ActiveFrameLength = 32;
         haudio_out_sai.FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
         haudio_out_sai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
         haudio_out_sai.FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

         /* Configure SAI Block_x Slot */
         haudio_out_sai.SlotInit.FirstBitOffset = 0;
         haudio_out_sai.SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
         haudio_out_sai.SlotInit.SlotNumber     = 4;
         haudio_out_sai.SlotInit.SlotActive     = CODEC_AUDIOFRAME_SLOT_0123;

         HAL_SAI_Init(&haudio_out_sai);



         /* Initialize SAI1 block B in SLAVE RX synchronous from SAI1 block A */
         /* Initialize the haudio_in_sai Instance parameter */
         haudio_in_sai.Instance = AUDIO_IN_SAIx;

         /* Disable SAI peripheral to allow access to SAI internal registers */
         __HAL_SAI_DISABLE(&haudio_in_sai);

         /* Configure SAI_Block_x */
         haudio_in_sai.Init.MonoStereoMode = SAI_STEREOMODE;
         haudio_in_sai.Init.AudioFrequency = AudioFreq;
         haudio_in_sai.Init.AudioMode      = SAI_MODESLAVE_RX;
         haudio_in_sai.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
         haudio_in_sai.Init.Protocol       = SAI_FREE_PROTOCOL;
         haudio_in_sai.Init.DataSize       = SAI_DATASIZE_16;
         haudio_in_sai.Init.FirstBit       = SAI_FIRSTBIT_MSB;
         haudio_in_sai.Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;
         haudio_in_sai.Init.Synchro        = SAI_SYNCHRONOUS;
         haudio_in_sai.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
         haudio_in_sai.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
         haudio_in_sai.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
         haudio_in_sai.Init.CompandingMode = SAI_NOCOMPANDING;
         haudio_in_sai.Init.TriState       = SAI_OUTPUT_RELEASED;
         haudio_in_sai.Init.Mckdiv         = 0;

         /* Configure SAI_Block_x Frame */
         haudio_in_sai.FrameInit.FrameLength       = 64;
         haudio_in_sai.FrameInit.ActiveFrameLength = 32;
         haudio_in_sai.FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
         haudio_in_sai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
         haudio_in_sai.FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

         /* Configure SAI Block_x Slot */
         haudio_in_sai.SlotInit.FirstBitOffset = 0;
         haudio_in_sai.SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
         haudio_in_sai.SlotInit.SlotNumber     = 4;
         haudio_in_sai.SlotInit.SlotActive     = CODEC_AUDIOFRAME_SLOT_0123;

         HAL_SAI_Init(&haudio_in_sai);

         /* Enable SAI peripheral */
         __HAL_SAI_ENABLE(&haudio_in_sai);

         /* Enable SAI peripheral to generate MCLK */
         __HAL_SAI_ENABLE(&haudio_out_sai);
     }


     /**
       * @brief  Deinitializes the output Audio Codec audio interface (SAI).
       * @retval None
       */
     void SAIx_In_DeInit(void)
     {
         /* Initialize the haudio_in_sai Instance parameter */
         haudio_in_sai.Instance = AUDIO_IN_SAIx;
         haudio_out_sai.Instance = AUDIO_OUT_SAIx;
         /* Disable SAI peripheral */
         __HAL_SAI_DISABLE(&haudio_in_sai);

         HAL_SAI_DeInit(&haudio_in_sai);
         HAL_SAI_DeInit(&haudio_out_sai);
     }


     /**
       * @brief  Initializes SAI Audio IN MSP.
       * @param  hsai: SAI handle
       * @retval None
       */
     void SAI_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params)
     {
         static DMA_HandleTypeDef hdma_sai_rx;
         GPIO_InitTypeDef  gpio_init_structure;

         /* Enable SAI clock */
         AUDIO_IN_SAIx_CLK_ENABLE();

         /* Enable SD GPIO clock */
         AUDIO_IN_SAIx_SD_ENABLE();
         /* CODEC_SAI pin configuration: SD pin */
         gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
         gpio_init_structure.Mode = GPIO_MODE_AF_PP;
         gpio_init_structure.Pull = GPIO_NOPULL;
         gpio_init_structure.Speed = GPIO_SPEED_FAST;
         gpio_init_structure.Alternate = AUDIO_IN_SAIx_AF;
         HAL_GPIO_Init(AUDIO_IN_SAIx_SD_GPIO_PORT, &gpio_init_structure);

         /* Enable Audio INT GPIO clock */
         AUDIO_IN_INT_GPIO_ENABLE();
         /* Audio INT pin configuration: input */
         gpio_init_structure.Pin = AUDIO_IN_INT_GPIO_PIN;
         gpio_init_structure.Mode = GPIO_MODE_INPUT;
         gpio_init_structure.Pull = GPIO_NOPULL;
         gpio_init_structure.Speed = GPIO_SPEED_FAST;
         HAL_GPIO_Init(AUDIO_IN_INT_GPIO_PORT, &gpio_init_structure);

         /* Enable the DMA clock */
         AUDIO_IN_SAIx_DMAx_CLK_ENABLE();

         if (hsai->Instance == AUDIO_IN_SAIx)
         {
             /* Configure the hdma_sai_rx handle parameters */
             hdma_sai_rx.Init.Channel             = AUDIO_IN_SAIx_DMAx_CHANNEL;
             hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
             hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
             hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
             hdma_sai_rx.Init.PeriphDataAlignment = AUDIO_IN_SAIx_DMAx_PERIPH_DATA_SIZE;
             hdma_sai_rx.Init.MemDataAlignment    = AUDIO_IN_SAIx_DMAx_MEM_DATA_SIZE;
             hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
             hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
             hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
             hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
             hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
             hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;

             hdma_sai_rx.Instance = AUDIO_IN_SAIx_DMAx_STREAM;

             /* Associate the DMA handle */
             __HAL_LINKDMA(hsai, hdmarx, hdma_sai_rx);

             /* Deinitialize the Stream for new transfer */
             HAL_DMA_DeInit(&hdma_sai_rx);

             /* Configure the DMA Stream */
             HAL_DMA_Init(&hdma_sai_rx);
         }

         /* SAI DMA IRQ Channel configuration */
         HAL_NVIC_SetPriority(AUDIO_IN_SAIx_DMAx_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
         HAL_NVIC_EnableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

         /* Audio INT IRQ Channel configuration */
         HAL_NVIC_SetPriority(AUDIO_IN_INT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
         HAL_NVIC_EnableIRQ(AUDIO_IN_INT_IRQ);
     }



     uint8_t BSP_AUDIO_IN_OUT_Init(uint32_t AudioFreq)
     {
         uint8_t ret = AUDIO_ERROR;

         /* Disable SAI */
         SAIx_In_DeInit();

         /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
         BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL);
         haudio_out_sai.Instance = AUDIO_OUT_SAIx;
         haudio_in_sai.Instance = AUDIO_IN_SAIx;
         if (HAL_SAI_GetState(&haudio_in_sai) == HAL_SAI_STATE_RESET)
         {
             BSP_AUDIO_OUT_MspInit(&haudio_out_sai, NULL);
             SAI_AUDIO_IN_MspInit(&haudio_in_sai, NULL);
         }


         SAIx_In_Init(AudioFreq);


         if ((wm8994_drv.ReadID(AUDIO_I2C_ADDRESS)) == WM8994_ID)
         {
             /* Reset the Codec Registers */
             wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
             /* Initialize the audio driver structure */
             audio_drv = &wm8994_drv;
             ret = AUDIO_OK;
         } else {
             ret = AUDIO_ERROR;
         }

         if (ret == AUDIO_OK)
         {
             /* Initialize the codec internal registers */
        	 //INPUT_DEVICE_ANALOG_MIC is the LINE IN input
        	 //100 is MAX volume on the CODEC
        	 audio_drv->Init(AUDIO_I2C_ADDRESS, INPUT_DEVICE_ANALOG_MIC | OUTPUT_DEVICE_HEADPHONE , 65, AudioFreq);
             //audio_drv->Init(AUDIO_I2C_ADDRESS, INPUT_DEVICE_ANALOG_MIC | OUTPUT_DEVICE_HEADPHONE , 100, AudioFreq);
         }

         /* Return AUDIO_OK when all operations are correctly done */
         return ret;
     }


      uint8_t _BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
     {
         /* Call the audio Codec Play function */
    	 //This using the correct  size SF
         if (audio_drv->Play(AUDIO_I2C_ADDRESS, (uint16_t *)pBuffer, Size) != 0)
         {
             return AUDIO_ERROR;
         }
         else
         {
             /* Update the Media layer and enable it for play */

        	 //Buffer gets cast to a BYTE pointer, so this only transmits half the buffer
        	 //contents, i.e double buffer.


        	 if (HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) pBuffer, Size) !=  HAL_OK)
                 return AUDIO_ERROR;
             return AUDIO_OK;
         }
     }

      void AUDIO_SETUP(void)
           {

               /* Initialize Audio I/O */
               //96K NOT WORKING
               //48K TESTED  OK
               //44K TESTED OK
               //32K TESTED OK
               //22K TESTED OK
               //16K TESTED OK
               //11K NOT WORKING
               //8K  NOT WORKING

    	       //This will all kick off the SAI and configure the CODEC.
    	       //Call back interrupts, handled in main.c will then communicate with the
    	       //audio listene task

               //Change Sampling rate here...
               if (BSP_AUDIO_IN_OUT_Init(BSP_AUDIO_FREQUENCY_48K) == AUDIO_OK)
               {
              	 printf("Audio I/O initialization OK\r\n");
               } else {
              	 printf("Audio I/O initialization failed.\r\n");
              	 Error_Handler();
               }

               // Start Receiving audio into Record Buffer..
               // Audio Buffer is  defined as int16,
               // so this transfer will only begin to receive into half the buffer. Double buff
               //Not  the record buffer size is cast to uint8, so effectively this halfs the
               //bytes that re transferred per block
               HAL_StatusTypeDef res = HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)RecordBuffer, RECORD_BUFFER_SIZE);
               if (HAL_OK == res)
               {
              	 printf("SAI receive begin OK\r\n");
               } else {
                   printf("SAI receive error: %d\r\n", res);
                   Error_Handler();
               }

               printf("Begin Playback buffer\r\n");

               /* Play the recorded buffer */
               if (_BSP_AUDIO_OUT_Play((uint16_t *) &PlaybackBuffer[0], RECORD_BUFFER_SIZE) == AUDIO_OK)
               {
              	 printf("Audio output OK\r\n");
               } else {
              	 printf("Audio output error\r\n");
              	 Error_Handler();
           	 }
               printf("\r\n");

               audio_rec_buffer_state = BUFFER_OFFSET_NONE; //trap for kickoff


      } // end AUDIO_SETUP function





/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  BSP_LED_Toggle(LED_RED);
	  HAL_Delay(1000);

  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
