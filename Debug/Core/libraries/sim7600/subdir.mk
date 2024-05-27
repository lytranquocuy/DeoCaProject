################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/libraries/sim7600/sim7600.c 

OBJS += \
./Core/libraries/sim7600/sim7600.o 

C_DEPS += \
./Core/libraries/sim7600/sim7600.d 


# Each subdirectory must supply rules for building sources it contributes
Core/libraries/sim7600/%.o Core/libraries/sim7600/%.su Core/libraries/sim7600/%.cyclo: ../Core/libraries/sim7600/%.c Core/libraries/sim7600/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-libraries-2f-sim7600

clean-Core-2f-libraries-2f-sim7600:
	-$(RM) ./Core/libraries/sim7600/sim7600.cyclo ./Core/libraries/sim7600/sim7600.d ./Core/libraries/sim7600/sim7600.o ./Core/libraries/sim7600/sim7600.su

.PHONY: clean-Core-2f-libraries-2f-sim7600

