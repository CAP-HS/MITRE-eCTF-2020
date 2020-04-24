################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/aes_decrypt.c \
../src/aes_encrypt.c \
../src/ecdh.c \
../src/main.c \
../src/platform.c \
../src/setsecs2.c \
../src/sha256.c \
../src/util.c \
../src/utils.c \
../src/xil_sprintf.c 

OBJS += \
./src/aes_decrypt.o \
./src/aes_encrypt.o \
./src/ecdh.o \
./src/main.o \
./src/platform.o \
./src/setsecs2.o \
./src/sha256.o \
./src/util.o \
./src/utils.o \
./src/xil_sprintf.o 

C_DEPS += \
./src/aes_decrypt.d \
./src/aes_encrypt.d \
./src/ecdh.d \
./src/main.d \
./src/platform.d \
./src/setsecs2.d \
./src/sha256.d \
./src/util.d \
./src/utils.d \
./src/xil_sprintf.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -I"/ectf/mb/drm_audio_fw_bsp/microblaze_0/include" -c -fmessage-length=0 -MT"$@" -I"/ectf/mb/drm_audio_fw_bsp/microblaze_0/include" -mlittle-endian -mcpu=v10.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


