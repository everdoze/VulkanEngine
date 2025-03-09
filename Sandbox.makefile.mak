DIR := $(subst /,\,${CURDIR})
BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := sandbox
EXTENSION := .exe
COMPILER_FLAGS := -std=c++20 -g -Werror=vla -Wno-missing-braces 
INCLUDE_FLAGS := -Iengine\src -Isandbox\src 
LINKER_FLAGS := -g -Wl,-nodefaultlib:libcmt -lmsvcrtd -lengine -L$(OBJ_DIR)\engine -L$(BUILD_DIR) #-Wl,-rpath,.
DEFINES := -D_DEBUG -DAPI_IMPORT -D_MT -D_DLL

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.cpp) # Get all .c files
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src)) # Get all directories under src.
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for tesbed

all: scaffold compile link

.PHONY: scaffold
scaffold: # create build directory
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # link
	@echo Linking $(ASSEMBLY)...
	@clang $(LINKER_FLAGS) $(OBJ_FILES) -o $(BUILD_DIR)/$(ASSEMBLY)$(EXTENSION) 

.PHONY: compile
compile: #compile .c files
	@echo Compiling...

.PHONY: clean
clean: # clean build directory
	if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)
	if exist $(BUILD_DIR)\$(ASSEMBLY).ilk del $(BUILD_DIR)\$(ASSEMBLY).ilk
	if exist $(BUILD_DIR)\$(ASSEMBLY).pdb del $(BUILD_DIR)\$(ASSEMBLY).pdb
	if exist $(OBJ_DIR)\$(ASSEMBLY) rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.cpp.o: %.cpp # compile .c to .c.o object
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)