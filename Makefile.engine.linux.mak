BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := engine
COMPILED_NAME := hazkerEngine
EXTENSION := .so
COMPILER_FLAGS := -g -MD -Werror=vla -fdeclspec -fPIC
INCLUDE_FLAGS := -I$(PWD)/engine/src -I$(VULKAN_SDK)\include
LINKER_FLAGS := -g -shared -lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$(VULKAN_SDK)\Lib -L/usr/X11R6/lib
DEFINES := -D_DEBUG -DHEXPORT

SRC_FILES := $(shell find $(ASSEMBLY) -name *.c) 	# .c files
CPP_FILES := $(shell find $(ASSEMBLY) -name *.cpp) # Get all .cpp files
DIRECTORIES := $(shell find $(ASSEMBLY) -type d) 	# directories with .h files
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) $(CPP_FILES:%=$(OBJ_DIR)/%.o) # compiled .o objects

all: scaffold compile link

.PHONY: scaffold
scaffold: # Create build directory
	@echo Scaffolding folder structure...
	@mkdir -p $(addprefix $(OBJ_DIR)/,$(DIRECTORIES))
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # Link
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/lib$(COMPILED_NAME)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: # Compile .c files
	@echo Compiling...

.PHONY: clean
clean: # Clean build directory
	rm -rf $(BUILD_DIR)\$(COMPILED_NAME)
	rm -rf $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c # Compile .c to .c.o object
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp # Compile .cpp to .cpp.o object
	@echo	$<...
	@clang++ $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

-include $(OBJ_FILES:.o=.d)