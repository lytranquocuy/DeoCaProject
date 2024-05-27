################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/libraries/LIGOSensor/ringbuffer.c \
../Core/libraries/LIGOSensor/uart.c 

OBJS += \
./Core/libraries/LIGOSensor/ringbuffer.o \
./Core/libraries/LIGOSensor/uart.o 

C_DEPS += \
./Core/libraries/LIGOSensor/ringbuffer.d \
./Core/libraries/LIGOSensor/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/libraries/LIGOSensor/%.o Core/libraries/LIGOSensor/%.su Core/libraries/LIGOSensor/%.cyclo: ../Core/libraries/LIGOSensor/%.c Core/libraries/LIGOSensor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-libraries-2f-LIGOSensor

clean-Core-2f-libraries-2f-LIGOSensor:
	-$(RM) ./Core/libraries/LIGOSensor/ringbuffer.cyclo ./Core/libraries/LIGOSensor/ringbuffer.d ./Core/libraries/LIGOSensor/ringbuffer.o ./Core/libraries/LIGOSensor/ringbuffer.su ./Core/libraries/LIGOSensor/uart.cyclo ./Core/libraries/LIGOSensor/uart.d ./Core/libraries/LIGOSensor/uart.o ./Core/libraries/LIGOSensor/uart.su

.PHONY: clean-Core-2f-libraries-2f-LIGOSensor

