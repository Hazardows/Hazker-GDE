DIR := $(subst /,\,$(CURDIR))
BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := tests
COMPILED_NAME := tests
EXTENSION := .exe
COMPILER_FLAGS := -g -MD -Werror=vla -Wno-missing-braces -fdeclspec #-fPIC
INCLUDE_FLAGS := -Iengine\src -Itestbed\src
LINKER_FLAGS := -g -lhazkerEngine.lib -L$(OBJ_DIR)\engine -L$(BUILD_DIR) -Xlinker /NODEFAULTLIB:libcmt -lmsvcrtd #-Wl, -rpath, . 
DEFINES := -D_DEBUG -DHIMPORT

SRC_FILES := $(shell find $(ASSEMBLY) -name *.c)   # Get all .c files
CPP_FILES := $(shell find $(ASSEMBLY) -name *.cpp) # Get all .cpp files
DIRECTORIES := $(shell find $(ASSEMBLY) -type d)   # Directories with .h files
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) $(CPP_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for testbed

all: scaffold compile link

.PHONY: scaffold
scaffold: # Create build directory
	@echo Scaffolding folder structure...
	@mkdir -p $(addprefix $(OBJ_DIR)/,$(DIRECTORIES))
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # Link
	@echo Linking $(ASSEMBLY)...
	clang $(OBJ_FILES) -o $(BUILD_DIR)/$(COMPILED_NAME)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: # Compile .c files
	@echo Compiling...

.PHONY: clean
clean: # Clean build directory
	rm -rf $(BUILD_DIR)/$(ASSEMBLY)
	rm -rf $(OBJ_DIR)/$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c # Compile .c to .c.o object
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp # Compile .cpp to .cpp.o object
	@echo	$<...
	@clang++ $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

-include $(OBJ_FILES:.o=.d)