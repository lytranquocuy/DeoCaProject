################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/libraries/24LCxx/24LCxx.c 

OBJS += \
./Core/libraries/24LCxx/24LCxx.o 

C_DEPS += \
./Core/libraries/24LCxx/24LCxx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/libraries/24LCxx/%.o Core/libraries/24LCxx/%.su Core/libraries/24LCxx/%.cyclo: ../Core/libraries/24LCxx/%.c Core/libraries/24LCxx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-libraries-2f-24LCxx

clean-Core-2f-libraries-2f-24LCxx:
	-$(RM) ./Core/libraries/24LCxx/24LCxx.cyclo ./Core/libraries/24LCxx/24LCxx.d ./Core/libraries/24LCxx/24LCxx.o ./Core/libraries/24LCxx/24LCxx.su

.PHONY: clean-Core-2f-libraries-2f-24LCxx

