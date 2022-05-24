.SUFFIXES : .c .o

include tools.mk

Q_		  := 
BASE      := $(shell pwd)

DESTDIR   ?= /tftpboot

CXXFLAGS  := -O2 -g -W -Wall -Wextra -Werror -Wuninitialized -Wmissing-declarations -fPIC
LDFLAGS   := -lpthread 
DEFINES   += 

OUT_DIR   := out
TARGET    := myapp

DEFINES   := -D_GNU_SOURCE
INCDIRS   := $(BASE)
SRCDIRS   := $(BASE)
LIBDIRS   :=
SRCS      := main.cpp


INCDIRS   += $(BASE)/base
SRCDIRS   += $(BASE)/base
SRCS      += mutex.cpp
SRCS      += cond_var.cpp
SRCS      += sys_time.cpp
SRCS      += task.cpp
SRCS      += log.cpp
SRCS      += timer.cpp
SRCS      += event_loop.cpp
SRCS      += timer_thread.cpp

INCDIRS   += $(BASE)/core
SRCDIRS   += $(BASE)/core
SRCS      += netlink_manager.cpp
SRCS      += usb_hotplug_manager.cpp


##################################################
# Input
INCDIRS   += $(BASE)/input
SRCDIRS   += $(BASE)/input
SRCS      += input_manager.cpp
SRCS      += keymap.cpp

##################################################
# Network
INCDIRS   += $(BASE)/net
SRCDIRS   += $(BASE)/net
SRCS      += netutil.cpp

##################################################
# WiFi
#WPA_SUPPLICANT := $(BASE_DIR)/build/wpa_supplicant-2.9
#INCDIRS   += $(WPA_SUPPLICANT)/src/common
#LDFLAGS   += -lwpa_client

#INCDIRS   += $(BASE)/wifi
#SRCDIRS   += $(BASE)/wifi
#SRCS      += wifi_device.cpp
#SRCS      += wifi_p2p_device.cpp
#SRCS      += wifi_p2p_group.cpp
#SRCS      += wifi_p2p_config.cpp
#SRCS      += wifi_event.cpp
#SRCS      += wifi_manager.cpp

##################################################
# Utils 
INCDIRS   += $(BASE)/utils
SRCDIRS   += $(BASE)/utils
SRCS      += trim.cpp
SRCS      += ini.cpp
SRCS      += key_value.cpp
SRCS      += popen2.cpp
SRCS      += parcel.cpp

###############################################################################
# DO NOT MODIFY .......
###############################################################################
APP       := $(OUT_DIR)/$(TARGET)
APP_OBJS  := $(SRCS:%=$(OUT_DIR)/%.o)
APP_FLAGS := $(CFLAGS) $(DEFINES)
APP_FLAGS += $(addprefix -I, $(INCDIRS))
APP_FLAGS += $(addprefix -L, $(LIBDIRS))
APP_FLAGS += $(LDFLAGS)

vpath %.cpp $(SRCDIRS)
vpath %.c $(SRCDIRS)

.PHONY: all clean

all: $(OUT_DIR) app 

app: $(APP_OBJS)
	@echo "[Linking... $(notdir $(APP))]"
	$(Q_)$(CXX) -o $(APP) $(APP_OBJS) $(APP_FLAGS)

$(OUT_DIR):
	$(Q_)mkdir $(OUT_DIR)

$(OUT_DIR)/%.c.o: %.c
	@echo "[Compile... $(notdir $<)]"
	$(Q_)$(CC) $(APP_FLAGS) -c $< -o $@

$(OUT_DIR)/%.cpp.o: %.cpp
	@echo "[Compile... $(notdir $<)]"
	$(Q_)$(CXX) $(APP_FLAGS) -c $< -o $@

install:
	@echo "[Install .... $(notdir $(APP))]"
	# $(Q_) install -s --strip-program=$(STRIP) $(APP) $(DESTDIR)$(BIN_DIR)
	$(Q_)cp $(APP) $(DESTDIR)/usr/bin
	$(Q_)install -m 0755 res/p2p_supplicant.conf $(DESTDIR)/etc/p2p_supplicant.conf
	$(Q_)install -m 0755 res/wpa_supplicant.conf $(DESTDIR)/etc/wpa_supplicant.conf

clean: 
	@echo "[Clean... all objs]"
	$(Q_)rm -rf $(OUT_DIR)
