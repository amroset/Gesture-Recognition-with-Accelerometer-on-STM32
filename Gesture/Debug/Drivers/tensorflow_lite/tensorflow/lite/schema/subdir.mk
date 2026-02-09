################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.cc 

CC_DEPS += \
./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.d 

OBJS += \
./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/tensorflow_lite/tensorflow/lite/schema/%.o Drivers/tensorflow_lite/tensorflow/lite/schema/%.su Drivers/tensorflow_lite/tensorflow/lite/schema/%.cyclo: ../Drivers/tensorflow_lite/tensorflow/lite/schema/%.cc Drivers/tensorflow_lite/tensorflow/lite/schema/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L475xx -DCMSIS_NN -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Core/Include -I../Drivers/CMSIS/DSP/Include -I../Drivers/CMSIS/DSP/PrivateInclude -I../Drivers/CMSIS/NN/Include -I../Drivers/CMSIS/Include -I../Drivers/tensorflow_lite -I../Drivers/tensorflow_lite/third_party/flatbuffers/include -I../Drivers/tensorflow_lite/third_party/gemmlowp -I../Drivers/tensorflow_lite/third_party/kissfft -I../Drivers/tensorflow_lite/third_party/ruy -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-tensorflow_lite-2f-tensorflow-2f-lite-2f-schema

clean-Drivers-2f-tensorflow_lite-2f-tensorflow-2f-lite-2f-schema:
	-$(RM) ./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.cyclo ./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.d ./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.o ./Drivers/tensorflow_lite/tensorflow/lite/schema/schema_utils.su

.PHONY: clean-Drivers-2f-tensorflow_lite-2f-tensorflow-2f-lite-2f-schema

