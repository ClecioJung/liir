# ----------------------------------------
# Project definitions
# ----------------------------------------

# Name of the project
PROJECT = liir

# Folders
RELEASE_DIR = release
DEBUG_DIR = debug
ODIR = .obj
DDIR = .deps
SDIR = src

# ----------------------------------------
# Compiler and linker definitions
# ----------------------------------------

# Compiler, linker and debugger
CC = gcc
DEBUGGER = gdb

# Libraries
LIBS = -lm

# Flags for compiler
COMMON_FLAGS = -W -Wall -Wextra -pedantic -Werror -std=c11
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -O0 -g -DDEBUG
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DDIR)/$*.Td

# ----------------------------------------
# Project macros and functions
# ----------------------------------------

# Git version
# Based on: https://stackoverflow.com/questions/1704907/how-can-i-get-my-c-code-to-automatically-print-out-its-git-version-hash
GIT_VERSION = "$(shell git describe --always --dirty --tags)"

# Recursive wildcard
# Based on: https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Name of the compiled file
RELEASE_EXEC = $(addprefix $(RELEASE_DIR)/, $(PROJECT))
DEBUG_EXEC = $(addprefix $(DEBUG_DIR)/, $(PROJECT))

# Source files
SRCS = $(call rwildcard,$(SDIR),*.c)

# Dependency files (auto generated)
DEPS = $(addprefix $(DDIR)/,$(patsubst %,%.d,$(patsubst %,%,$(basename $(notdir $(SRCS))))))

# Object files
OBJS = $(addprefix $(ODIR)/,$(patsubst %,%.o,$(patsubst %,%,$(basename $(notdir $(SRCS))))))
RELEASE_OBJS = $(addprefix $(RELEASE_DIR)/, $(OBJS))
DEBUG_OBJS = $(addprefix $(DEBUG_DIR)/, $(OBJS))

# Output directories
RELEASE_ODIR = $(addprefix $(RELEASE_DIR)/, $(ODIR))
DEBUG_ODIR = $(addprefix $(DEBUG_DIR)/, $(ODIR))

# Flags for compiler
REL_CFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
DCFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)

# ----------------------------------------
# Fomating macros
# ----------------------------------------

BOLD = \033[1m
NORMAL = \033[0m
RED = \033[0;31m
GREEN = \033[0;32m

# ----------------------------------------
# Compilation and linking rules
# ----------------------------------------

all: release

release: $(RELEASE_EXEC)

$(RELEASE_EXEC): $(RELEASE_OBJS)
	@ echo "${GREEN}Building binary ${BOLD}$@${GREEN} using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(filter %.s %.o,$^) -o $@ $(LIBS)
	@ touch $@

$(RELEASE_ODIR)/%.o: $(SDIR)/%.c
$(RELEASE_ODIR)/%.o: $(SDIR)/%.c $(DDIR)/%.d | $(DDIR) $(RELEASE_ODIR)
	@ echo "${GREEN}Building target ${BOLD}$@${GREEN}, using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(REL_CFLAGS) $(DEP_FLAGS) -c $(filter %.c %.s %.o,$^) -o $@
	@ mv -f $(DDIR)/$*.Td $(DDIR)/$*.d && touch $@

debug: $(DEBUG_EXEC)

$(DEBUG_EXEC): $(DEBUG_OBJS)
	@ echo "${GREEN}Building binary ${BOLD}$@${GREEN} using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(filter %.s %.o,$^) -o $@ $(LIBS)
	@ touch $@

$(DEBUG_ODIR)/%.o: $(SDIR)/%.c
$(DEBUG_ODIR)/%.o: $(SDIR)/%.c $(DDIR)/%.d | $(DDIR) $(DEBUG_ODIR)
	@ echo "${GREEN}Building target ${BOLD}$@${GREEN}, using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(DCFLAGS) $(DEP_FLAGS) -c $(filter %.c %.s %.o,$^) -o $@
	@ mv -f $(DDIR)/$*.Td $(DDIR)/$*.d && touch $@

# ----------------------------------------
# Automatic dependency generation rules
# ----------------------------------------

# Based on http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

$(DDIR)/%.d: ;
.PRECIOUS: $(DDIR)/%.d

-include $(DEPS)

# ----------------------------------------
# Script rules
# ----------------------------------------

$(RELEASE_ODIR) $(DEBUG_ODIR) $(DDIR):
	@ echo "${GREEN}Creating directory ${BOLD}$@${NORMAL}"
	mkdir -p $@

run: release
	@ echo "${GREEN}Running the aplication:${NORMAL}"
	./$(RELEASE_EXEC)

memcheck: release
	valgrind --tool=memcheck --track-origins=yes --leak-check=full ./$(RELEASE_EXEC)

debugger: debug
	@ echo "${GREEN}Running the aplication with the debugger${NORMAL}"
	$(DEBUGGER) ./$(DEBUG_EXEC)

clean:
	rm -fr $(RELEASE_DIR)/ $(DEBUG_DIR)/ $(DDIR)/ *.d *.o *.a *.so

remade: clean release

.PHONY: all release debug run memcheck debugger clean remade

# ----------------------------------------
