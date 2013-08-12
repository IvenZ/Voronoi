################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/calcDistance.c \
../src/main.c \
../src/opencl.c \
../src/openmp.c \
../src/serial.c 

OBJS += \
./src/calcDistance.o \
./src/main.o \
./src/opencl.o \
./src/openmp.o \
./src/serial.o 

C_DEPS += \
./src/calcDistance.d \
./src/main.d \
./src/opencl.d \
./src/openmp.d \
./src/serial.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -lGL -lglut -lGLU -fopenmp -lOpenCL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


