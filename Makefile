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

# ----------------------------------------
# Project macros and functions
# ----------------------------------------

# Git version
# Based on: https://stackoverflow.com/questions/1704907/how-can-i-get-my-c-code-to-automatically-print-out-its-git-version-hash
GIT_VERSION = "$(shell git describe --always --dirty --tags)"

# Recursive wildcard
# Based on: https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Name of the executable
RELEASE_EXEC = $(addprefix $(RELEASE_DIR)/, $(PROJECT))
DEBUG_EXEC = $(addprefix $(DEBUG_DIR)/, $(PROJECT))

# Source files
SRCS = $(call rwildcard,$(SDIR),*.c)

# Dependency files (auto generated)
DEPS = $(patsubst %,%.d,$(basename $(subst $(SDIR),$(DDIR),$(SRCS))))
RELEASE_DEPS = $(addprefix $(RELEASE_DIR)/, $(DEPS))
DEBUG_DEPS = $(addprefix $(DEBUG_DIR)/, $(DEPS))

# Object files
OBJS = $(patsubst %,%.o,$(basename $(subst $(SDIR),$(ODIR),$(SRCS))))
RELEASE_OBJS = $(addprefix $(RELEASE_DIR)/, $(OBJS))
DEBUG_OBJS = $(addprefix $(DEBUG_DIR)/, $(OBJS))

# Output directories
RELEASE_ODIR = $(addprefix $(RELEASE_DIR)/, $(ODIR))
DEBUG_ODIR = $(addprefix $(DEBUG_DIR)/, $(ODIR))
RELEASE_ODIRS = $(sort $(dir $(RELEASE_OBJS)))
DEBUG_ODIRS = $(sort $(dir $(DEBUG_OBJS)))

# Dependency directories
RELEASE_DDIR = $(addprefix $(RELEASE_DIR)/, $(DDIR))
DEBUG_DDIR = $(addprefix $(DEBUG_DIR)/, $(DDIR))
RELEASE_DDIRS = $(sort $(dir $(RELEASE_DEPS)))
DEBUG_DDIRS = $(sort $(dir $(DEBUG_DEPS)))

# Flags for compiler
REL_CFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
DEB_CFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)

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
$(RELEASE_ODIR)/%.o: $(SDIR)/%.c $(RELEASE_DDIR)/%.d | $(RELEASE_DDIRS) $(RELEASE_ODIRS)
	@ echo "${GREEN}Building target ${BOLD}$@${GREEN}, using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(REL_CFLAGS) -MT $@ -MMD -MP -MF $(patsubst %,%.Td,$(basename $(subst $(RELEASE_ODIR),$(RELEASE_DDIR),$@))) -c $(filter %.c %.s %.o,$^) -o $@
	@ mv -f $(patsubst %,%.Td,$(basename $(subst $(RELEASE_ODIR),$(RELEASE_DDIR),$@))) $(patsubst %,%.d,$(basename $(subst $(RELEASE_ODIR),$(RELEASE_DDIR),$@))) && touch $@

debug: $(DEBUG_EXEC)

$(DEBUG_EXEC): $(DEBUG_OBJS)
	@ echo "${GREEN}Building binary ${BOLD}$@${GREEN} using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(filter %.s %.o,$^) -o $@ $(LIBS)
	@ touch $@

$(DEBUG_ODIR)/%.o: $(SDIR)/%.c
$(DEBUG_ODIR)/%.o: $(SDIR)/%.c $(DEBUG_DDIR)/%.d | $(DEBUG_DDIRS) $(DEBUG_ODIRS)
	@ echo "${GREEN}Building target ${BOLD}$@${GREEN}, using dependencies ${BOLD}$^${NORMAL}"
	$(CC) $(DEB_CFLAGS) -MT $@ -MMD -MP -MF $(patsubst %,%.Td,$(basename $(subst $(DEBUG_ODIR),$(DEBUG_DDIR),$@))) -c $(filter %.c %.s %.o,$^) -o $@
	@ mv -f $(patsubst %,%.Td,$(basename $(subst $(DEBUG_ODIR),$(DEBUG_DDIR),$@))) $(patsubst %,%.d,$(basename $(subst $(DEBUG_ODIR),$(DEBUG_DDIR),$@))) && touch $@

# ----------------------------------------
# Automatic dependency generation rules
# ----------------------------------------

# Based on http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

$(RELEASE_DEPS): ;
.PRECIOUS: $(RELEASE_DEPS)

-include $(RELEASE_DEPS)

$(DEBUG_DEPS): ;
.PRECIOUS: $(DEBUG_DEPS)

-include $(DEBUG_DEPS)

# ----------------------------------------
# Script rules
# ----------------------------------------

$(RELEASE_ODIRS) $(DEBUG_ODIRS) $(RELEASE_DDIRS) $(DEBUG_DDIRS):
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

log:
	@ echo "${GREEN}Git project log:${NORMAL}"
	git log --oneline --decorate --all --graph

clean:
	rm -fr $(RELEASE_DIR)/ $(DEBUG_DIR)/ *.d *.o *.a *.so

remade: clean release

.PHONY: all release debug run memcheck debugger log clean remade

# ----------------------------------------
