# Based on http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
# Git version: https://stackoverflow.com/questions/1704907/how-can-i-get-my-c-code-to-automatically-print-out-its-git-version-hash

# ----------------------------------------
# Project definitions
# ----------------------------------------

# Name of the project
EXEC = liir

# Folders
ODIR = .obj
DDIR = .deps
SDIR = src
IDIR = src

# .c files
SRCS = $(wildcard $(SDIR)/*.c $(SDIR)/*.cpp)

# .h files
INCS = $(wildcard $(IDIR)/*.h $(IDIR)/*.hpp)

# Dependency files (auto generated)
DEPS = $(patsubst %,%.d,$(basename $(subst $(SDIR),$(DDIR),$(SRCS))))

# Object files
OBJS = $(patsubst %,%.o,$(basename $(subst $(SDIR),$(ODIR),$(SRCS))))

# ----------------------------------------
# Compiler and linker definitions
# ----------------------------------------

# Compiler and linker
CC = gcc
CXX = g++

# Libraries
INCLUDES =

# Flags for compiler
CFLAGS = -W -Wall -Wextra -pedantic -Werror -O2 -std=c11
CXXFLAGS = -W -Wall -Wextra -pedantic -Werror -O2 -std=c++11
DEPFLAGS = -MT $@ -MMD -MP -MF $(DDIR)/$*.Td

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

all: $(EXEC)

$(EXEC): $(OBJS)
	@ echo "${GREEN}Building binary: ${BOLD}$@${GREEN} using dependencies: ${BOLD}$^${NORMAL}"
	$(CXX) $(CXXFLAGS) $(filter %.s %.o,$^) -o $@ $(INCLUDES)
	@ touch $@

$(ODIR)/%.o : $(SDIR)/%.c
$(ODIR)/%.o : $(SDIR)/%.c $(DDIR)/%.d | $(DDIR) $(ODIR)
	@ echo "${GREEN}Building target: ${BOLD}$@${GREEN}, using dependencies: ${BOLD}$^${NORMAL}"
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $(filter %.c %.s %.o,$^) -o $@
	@ mv -f $(DDIR)/$*.Td $(DDIR)/$*.d && touch $@

$(ODIR)/%.o : $(SDIR)/%.cpp
$(ODIR)/%.o : $(SDIR)/%.cpp $(DDIR)/%.d | $(DDIR) $(ODIR)
	@ echo "${GREEN}Building target: ${BOLD}$@${GREEN}, using dependencies: ${BOLD}$^${NORMAL}"
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $(filter %.cpp %.s %.o,$^) -o $@
	@ mv -f $(DDIR)/$*.Td $(DDIR)/$*.d && touch $@

$(DDIR)/%.d: ;
.PRECIOUS: $(DDIR)/%.d

-include $(DEPS)

# ----------------------------------------
# Script rules
# ----------------------------------------

$(DDIR):
	mkdir -p $@

$(ODIR):
	mkdir -p $@

run:
	@ echo "${GREEN}Running the aplication:${NORMAL}"
	./$(EXEC)

memcheck:
	valgrind --tool=memcheck --track-origins=yes --leak-check=full ./$(EXEC)

clean:
	rm -fr $(ODIR)/ $(DDIR)/ $(EXEC) *.d *.o *.a *.so

remade: clean all

.PHONY: all run memcheck clean remade

# ----------------------------------------
