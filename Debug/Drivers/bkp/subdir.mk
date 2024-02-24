################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/bkp/stm32g4xx_hal_fdcan.c 

C_DEPS += \
./Drivers/bkp/stm32g4xx_hal_fdcan.d 

OBJS += \
./Drivers/bkp/stm32g4xx_hal_fdcan.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/bkp/%.o Drivers/bkp/%.su Drivers/bkp/%.cyclo: ../Drivers/bkp/%.c Drivers/bkp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Drivers/libcyphal -I../Drivers/libvoltbro -I../Drivers/libcyphal/libs -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-bkp

clean-Drivers-2f-bkp:
	-$(RM) ./Drivers/bkp/stm32g4xx_hal_fdcan.cyclo ./Drivers/bkp/stm32g4xx_hal_fdcan.d ./Drivers/bkp/stm32g4xx_hal_fdcan.o ./Drivers/bkp/stm32g4xx_hal_fdcan.su

.PHONY: clean-Drivers-2f-bkp

