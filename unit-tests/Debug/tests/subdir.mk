################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tests/test-msg.c \
../tests/test-serializacion.c \
../tests/test-string.c 

OBJS += \
./tests/test-msg.o \
./tests/test-serializacion.o \
./tests/test-string.o 

C_DEPS += \
./tests/test-msg.d \
./tests/test-serializacion.d \
./tests/test-string.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2020-2c-Segundo-Saque/cshared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


