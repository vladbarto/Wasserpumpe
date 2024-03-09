# Makefile for building a project using the ARM Cortex-M3 toolchain
# and flashing it onto an Atmel SAM3X8E microcontroller.
#
# Author: David Heiß
# Date: Fri Sep 22 10:31:34 PM CEST 2023

# Declare phony targets and default goal
.PHONY = help clean openocd
.PHONY: ${.PHONY}
.DEFAULT_GOAL = help

# Define source files, object files, dependencies and makefiles
sources = $(shell find src -iname "*.c")

objects = build/startup_sam3xa.o \
          build/system_sam3xa.o  \
          build/fix_sam3xa.o     \
          build/stubs.o   

dependencies = $(sources:src/%.c=build/%.d)

makefiles = .vscode/default.mk \
            Makefile

# Compiler and assembler settings
CC = arm-none-eabi-gcc
CFLAGS = -D __SAM3X8E__  \
         -mcpu=cortex-m3 \
         -Wall           \
         -I ../Include

AS = $(CC)
ASFLAGS = -c $(CFLAGS)

# Linker settings
TARGET = sram
LDFLAGS = -e Fix_SAM3XA       \
          -L ../Linker        \
          -T sam3x8e_$(TARGET).ld
LDLIBS = -lc

# Define common recipes as macros
define DIR_GUARD =
	@mkdir -p $(@D)
endef

define S->O =
	$(DIR_GUARD)
	$(AS) $(ASFLAGS) $< -o $@
endef

define C->D = 
	$(DIR_GUARD)
	$(CC) $(CFLAGS) -MM -MT $(@:%.d=%.o) $< -MF $@ 
endef

define C->S = 
	$(DIR_GUARD)
	$(CC) -S $(CFLAGS) $< -o $@
endef

define C->O =
	$(DIR_GUARD)
	$(CC) -c $(CFLAGS) $< -o $@
endef

define C->ELF =
	$(DIR_GUARD)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
endef

# Implicit pattern rules for building object files and dependencies
build/%.d: src/%.c $(makefiles)
	$(C->D)

build/%.s: src/%.c $(makefiles) 
	$(C->S)

build/%.o: src/%.s $(makefiles)
	$(S->O)

build/%.o: src/%.c $(makefiles) 
	$(C->O)

# Explicit rules for building specific object files
build/startup_sam3xa.o: ../Source/startup_sam3xa.c $(makefiles)
	$(C->O)

build/system_sam3xa.o: ../Source/system_sam3xa.c $(makefiles)
	$(C->O)
	
build/stubs.o: ../Source/stubs.c $(makefiles)
	$(C->O)

build/fix_sam3xa.o: ../Source/fix_sam3xa.s
	$(S->O)

# Implicit pattern rule for linking the final executable
build/%.elf: $(objects)
	$(C->ELF)

# Help target to display available make targets and their descriptions
help:
	@make --help
	@printf "\033[3A\033[J"
	@printf "Target:\n"
	@printf "* help    \033[19C Zeigt diese Nachricht an.\n"
	@printf "  clean   \033[19C Löscht den 'build' Ordner.\n"
	@printf "  openocd \033[19C Startet OpenOCD in einem Terminal.\n\n"

# Function to wait for ports to become available
define wait_port = 
while lsof -Pi :$1 -sTCP:LISTEN > /dev/null; do sleep 0.01; done
endef

# Target to start OpenOCD
openocd:
	@if pgrep -x openocd > /dev/null; then killall -9 openocd; $(call wait_port,6666); $(call wait_port,4444); $(call wait_port,3333); fi
	openocd -f interface/cmsis-dap.cfg -f board/atmel_sam3x_ek.cfg -c "bindto 0.0.0.0"

# Target to clean the project by removing the 'build' folder
clean:
	rm -rf build

# Include generated dependency files (if applicable)
ifneq (,$(MAKECMDGOALS))
ifeq (,$(filter $(MAKECMDGOALS),$(.PHONY) :))
include $(dependencies)
endif
endif
