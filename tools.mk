ENABLE_DEBUG=y

Q_            ?= @

#ARCH          := arm-linux
#CROSS_COMPILE := $(BASE_DIR)/host/bin/$(ARCH)-
#SYSROOT_DIR   := $(BASE_DIR)/host/arm-buildroot-linux-uclibcgnueabihf/sysroot
#INSTALL_DIR   := $(BASE_DIR)/target

AR       := $(CROSS_COMPILE)ar
AS       := $(CROSS_COMPILE)as
LD       := $(CROSS_COMPILE)ld
NM       := $(CROSS_COMPILE)nm
CC       := $(CROSS_COMPILE)gcc
GCC      := $(CROSS_COMPILE)gcc
CPP      := $(CROSS_COMPILE)cpp
CXX      := $(CROSS_COMPILE)g++
FC       := $(CROSS_COMPILE)gfortran
RANLIB   := $(CROSS_COMPILE)ranlib
READELF  := $(CROSS_COMPILE)readelf
STRIP    := $(CROSS_COMPILE)strip
OBJCOPY  := $(CROSS_COMPILE)objcopy
OBJDUMP  := $(CROSS_COMPILE)objdump
INSTALL  := install

CFLAGS       := -Wall -Wextra -Werror -Wuninitialized -Wmissing-declarations  
CXXFLAGS     := -Wall -Wextra -Werror -Wuninitialized -Wmissing-declarations  
LDFLAGS      :=
INSTALL_FLAG := -s --strip-program=$(STRIP)

ifeq ($(ENABLE_DEBUG), y)
CFLAGS       += -g
CXXFLAGS     += -g
INSTALL_FLAG :=
endif

