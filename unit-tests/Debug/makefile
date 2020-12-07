################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include tests/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: unit-tests

dependents:
	-cd /home/utnso/tp-2020-2c-Segundo-Saque/cshared/Debug && $(MAKE) clean all

# Tool invocations
unit-tests: $(OBJS) $(USER_OBJS) /home/utnso/tp-2020-2c-Segundo-Saque/cshared/Debug/libcshared.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2020-2c-Segundo-Saque/cshared/Debug" -o "unit-tests" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) unit-tests
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/tp-2020-2c-Segundo-Saque/cshared/Debug/libcshared.so:

-include ../makefile.targets
