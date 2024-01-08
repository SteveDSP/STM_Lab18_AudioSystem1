################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.c \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.c \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
../Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.c 

C_DEPS += \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.d \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.d \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.d \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.d \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.d \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.d 

OBJS += \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.o \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.o \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.o \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.o \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.o \
./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32F769I-Discovery/%.o Drivers/BSP/STM32F769I-Discovery/%.su Drivers/BSP/STM32F769I-Discovery/%.cyclo: ../Drivers/BSP/STM32F769I-Discovery/%.c Drivers/BSP/STM32F769I-Discovery/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_STM32F769I_DISCO_REVB03 -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I../Drivers/CMSIS/DSP/Include -I../Drivers/BSP/STM32F769I-Discovery -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32F769I-2d-Discovery

clean-Drivers-2f-BSP-2f-STM32F769I-2d-Discovery:
	-$(RM) ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.su ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.su ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.su ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_qspi.su ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.su ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.cyclo ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.d ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.o ./Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32F769I-2d-Discovery

