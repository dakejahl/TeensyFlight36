# Target device
MCU=MK66FX1M0

# The name of your project (used to name the compiled .hex file)
# TARGET = $(notdir $(CURDIR))
TARGET = main

# The teensy version to use -- teensy3 core uses this
TEENSY = 36

TEENSY_CORE_SPEED = 180000000

# literally only used for our USB delay -- other Arduino libraries might use this
TEENSYDUINO = 145

# configurable options
OPTIONS = -DUSB_SERIAL -DLAYOUT_US_ENGLISH

# directory to build in
BUILDDIR = $(abspath $(CURDIR)/build)
INCLUDE_DIR = $(abspath $(CURDIR)/include)
SOURCE_DIR = $(abspath $(CURDIR)/src)


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
TOOLSPATH = $(CURDIR)/tools
TEENSY_TOOLS = $(CURDIR)/tools/tool-teensy

# path location for Teensy 3 core
TEENSYCOREPATH = teensy3

# CMSIS_LIB_ARM = $(abspath $(CURDIR)/CMSIS/DSP/Lib/GCC/)

# path location for the arm-none-eabi compiler -- use the same one as for PX4
COMPILERPATH = $(abspath $(TOOLSPATH)/toolchain-gccarmnoneeabi/bin)

# path location for FreeRTOS directory structure
FREERTOSPATH = freertos
FREERTOSPORT = portable/GCC/ARM_CM4F

DISPATCH_QUEUE = src/dispatch_queue
TIMERS = src/timers
SPI = src/spi
MPU9250 = src/mpu9250
SERIAL = src/serial
TASKS = src/tasks
BOARD = src/board
PWM = src/pwm
HEADER_LIBS = lib/
ESTIMATION = src/estimation
CALIBRATION = src/calibration

SEGGER_SYSVIEW = segger_system_view/SEGGER
SEGGER_SYSVIEW_CONFIG = segger_system_view/Config

SEGGER_SYSVIEW_FREERTOS = segger_system_view/FreeRTOSV10/
SEGGER_SYSVIEW_FREERTOS_CONFIG = segger_system_view/FreeRTOSV10/Config


#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -O0 -mthumb -ffunction-sections -fdata-sections -nostdlib
CPPFLAGS += -DTEENSYDUINO=$(TEENSYDUINO) -DF_CPU=$(TEENSY_CORE_SPEED) -MMD $(OPTIONS)
CPPFLAGS += -I$(FREERTOSPATH)/include -I$(FREERTOSPATH)/$(FREERTOSPORT)/
CPPFLAGS += -I$(SOURCE_DIR) -I$(TEENSYCOREPATH) -I$(INCLUDE_DIR)
CPPFLAGS += -I$(DISPATCH_QUEUE)
CPPFLAGS += -I$(TIMERS)
CPPFLAGS += -I$(SPI)
CPPFLAGS += -I$(MPU9250)
CPPFLAGS += -I$(SERIAL)
CPPFLAGS += -I$(TASKS)
CPPFLAGS += -I$(BOARD)
CPPFLAGS += -I$(PWM)
CPPFLAGS += -I$(SEGGER_SYSVIEW_FREERTOS) -I$(SEGGER_SYSVIEW_FREERTOS_CONFIG)
CPPFLAGS += -I$(SEGGER_SYSVIEW) -I$(SEGGER_SYSVIEW_CONFIG)
CPPFLAGS += -I$(HEADER_LIBS)
CPPFLAGS += -I$(ESTIMATION)
CPPFLAGS += -I$(CALIBRATION)


# Eigen directives
CPPFLAGS += -D__EIGEN_NO_DEBUG__ -D__EIGEN_DONT_ALIGN_ -D__EIGEN_MALLOC_ALREADY_ALIGNED__ -D__EIGEN_NO_MALLOC__

# compiler options for C++ only
CXXFLAGS = -std=gnu++14 -felide-constructors -fno-exceptions -fno-rtti

# linker options
LDFLAGS = -O0 -Wl,--gc-sections -mthumb

# additional libraries to link
LIBS = -lm -lstdc++
# LIBS += $(CMSIS_LIB_ARM)/libarm_cortexM4lf_math.a

# Teensy 3.6
CPPFLAGS += -D__MK66FX1M0__ -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDSCRIPT = $(TEENSYCOREPATH)/mk66fx1m0.ld
LDFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -T$(LDSCRIPT)

LDFLAGS += -LCMSIS/DSP/Lib/GCC -Llibarm_cortexM4lf_math.a

# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

# automatically create lists of the sources and objects
TC_FILES := $(wildcard $(TEENSYCOREPATH)/*.c)
TCPP_FILES := $(wildcard $(TEENSYCOREPATH)/*.cpp)
DP_Q_FILES := $(wildcard $(DISPATCH_QUEUE)/*.cpp)
TIMER_FILES := $(wildcard $(TIMERS)/*.cpp)
SPI_FILES := $(wildcard $(SPI)/*.cpp)
MPU9250_FILES := $(wildcard $(MPU9250)/*.cpp)
SERIAL_FILES := $(wildcard $(SERIAL)/*.cpp)
TASKS_FILES := $(wildcard $(TASKS)/*.cpp)
BOARD_FILES := $(wildcard $(BOARD)/*.cpp)
PWM_FILES := $(wildcard $(PWM)/*.cpp)
ESTIMATION_FILES := $(wildcard $(ESTIMATION)/*.cpp)
CALIBRATION_FILES := $(wildcard $(CALIBRATION)/*.cpp)




C_FILES := $(wildcard src/*.c)
CPP_FILES := $(wildcard src/*.cpp)

FREERTOS_FILES = $(wildcard $(FREERTOSPATH)/*.c)
FREERTOS_FILES += $(wildcard $(FREERTOSPATH)/$(FREERTOSPORT)/*.c)

# TODO: move to cmake... or figure out a better way to do this...
SYS_VIEW_FILES_ASM = $(wildcard $(SEGGER_SYSVIEW)/*.S)
SYS_VIEW_FILES += $(wildcard $(SEGGER_SYSVIEW)/*.c)
SYS_VIEW_FILES += $(wildcard $(SEGGER_SYSVIEW_CONFIG)/*.c)
SYS_VIEW_FILES += $(wildcard $(SEGGER_SYSVIEW_FREERTOS)/*.c)
SYS_VIEW_FILES += $(wildcard $(SEGGER_SYSVIEW_FREERTOS_CONFIG)/*.c)

# include paths for libraries
# L_INC := $(foreach lib,$(filter %/, $(wildcard $(LIBRARYPATH)/*/)), -I$(lib))

# anything in src/, teensy3 C/C++ files
SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)
SOURCES += $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o)
# All my home grown stuff
SOURCES += $(DP_Q_FILES:.cpp=.o) $(TIMER_FILES:.cpp=.o) $(SPI_FILES:.cpp=.o) $(MPU9250_FILES:.cpp=.o)
SOURCES += $(SERIAL_FILES:.cpp=.o) $(TASKS_FILES:.cpp=.o) $(BOARD_FILES:.cpp=.o)
SOURCES += $(PWM_FILES:.cpp=.o) $(ESTIMATION_FILES:.cpp=.o) $(CALIBRATION_FILES:.cpp=.o)

# Amazon pathed version of FreeRTOS w/ Segger SystemView
SOURCES += $(FREERTOS_FILES:.c=.o)
SOURCES += $(SYS_VIEW_FILES:.c=.o) $(SYS_VIEW_FILES_ASM:.S=.o)

OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

all: hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	@$(abspath $(TEENSY_TOOLS))/teensy_post_compile -file="$(basename $<)" -path=$(CURDIR) -tools="$(abspath $(TEENSY_TOOLS))"

reboot:
	@-$(abspath $(TEENSY_TOOLS))/teensy_reboot

upload: post_compile reboot

$(BUILDDIR)/%.o: %.c
	@echo "[CC]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.S
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS)

%.hex: %.elf
	@echo "[HEX]\t$@"
	@$(SIZE) "$<"
	@$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf "$(BUILDDIR)"
	@rm -f "$(TARGET).elf" "$(TARGET).hex"
