export RAYLIB_PATH ?= $(HOME)/raylib

export BUILD_MODE     := RELEASE

export RAYLIB_LIBTYPE := STATIC

define RAYLIB_MAKEFILE
$(shell $(MAKE) -s -f Makefile.raylib RAYLIB_PATH=$(RAYLIB_PATH) $(1))
endef

# Makefile config
export LDFLAGS   := $(call RAYLIB_MAKEFILE,ldflags)
export LDLIBS    := $(call RAYLIB_MAKEFILE,ldlibs)
export CPPFLAGS  := $(call RAYLIB_MAKEFILE,includes) -I./src
export EXEC_NAME ?= game
export CXXFLAGS  += -std=c++17 -flto -fno-rtti -fno-exceptions
export MKDIR     := mkdir -p

BUILD_DIR := build

BUILD_MODE_PATH = $(BUILD_DIR)/$(BUILD_NAME)

ifeq ($(BUILD_MODE),RELEASE)
	CPPFLAGS   += -DNDEBUG
	CXXFLAGS   += -march=native -O2 -s
	BUILD_NAME := release
else
	CXXFLAGS   += -g -ggdb -O0
	BUILD_NAME := debug
endif

export OBJ_DIR    := $(BUILD_MODE_PATH)/obj
export BUILD_PATH := $(BUILD_MODE_PATH)

EXEC_FILE := $(BUILD_MODE_PATH)/$(EXEC_NAME)

.PHONY: all run dirs info clean cleanall

$(EXEC_FILE):
	$(MAKE) -f Makefile.gen

all: dirs $(EXEC_FILE)

run: all
	$(EXEC_FILE)

dirs:
	$(MKDIR) $(BUILD_DIR) $(BUILD_DIR)/release $(BUILD_DIR)/debug

info:
	$(info [CURRENT_MAKEFILE] CXXFLAGS: $(CXXFLAGS))
	$(info [CURRENT_MAKEFILE] LDFLAGS: $(LDFLAGS))
	$(info [CURRENT_MAKEFILE] LDLIBS: $(LDLIBS))
	$(info [CURRENT_MAKEFILE] CPPFLAGS: $(CPPFLAGS))
	$(info =====SUB MAKE INFO=====)
	$(MAKE) -s -f Makefile.gen info

clean:
	$(MAKE) -f Makefile.gen clean

cleanall:
	$(RM) -R $(BUILD_DIR)
	$(MAKE) -f Makefile.gen cleanall
