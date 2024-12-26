DIR := $(subst /,\,$(CURDIR))
BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := testbed
COMPILED_NAME := testbed
EXTENSION := .exe
COMPILER_FLAGS := -g -Wmissing-braces -fdeclspec #-fPIC
INCLUDE_FLAGS := -Iengine\src -Itestbed\src
LINKER_FLAGS := -g -lhazkerEngine.lib -L$(OBJ_DIR)\engine -L$(BUILD_DIR) -lmsvcrtd #-Wl, -rpath, .
DEFINES := -D_DEBUG -DHIMPORT

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.c) # Get all .c files
CPP_FILES := $(call rwildcard,$(ASSEMBLY)/,*.cpp) # Get all .cpp files
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src)) # Get all directories under src.
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) $(CPP_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for testbed

all: scaffold compile link

.PHONY: scaffold
scaffold: # Create build directory
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # Link
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)\$(COMPILED_NAME)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: # Compile .c files
	@echo Compiling...

.PHONY: clean
clean: # Clean build directory
	if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)
	rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c # Compile .c to .c.o object
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp # Compile .cpp to .cpp.o object
	@echo	$<...
	@clang++ $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)