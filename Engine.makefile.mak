DIR := $(subst /,\,${CURDIR})
BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := engine
EXTENSION := .dll
COMPILER_FLAGS := -std=c++17 -g -Werror=vla -Wformat-security
INCLUDE_FLAGS := -Iengine\src -I$(VULKAN_SDK)\include -Iengine\src\vendor
LINKER_FLAGS := -g -Wl,-nodefaultlib:libcmt -lmsvcrtd -shared -luser32 -lvulkan-1 -L$(VULKAN_SDK)\Lib -L$(OBJ_DIR)\engine
DEFINES := -D_DEBUG -DAPI_EXPORT -D_CRT_SECURE_NO_WARNINGS -D_MT -D_DLL

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.cpp) # Get all .c files
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src)) # Get all directories under src.
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for engine


all: scaffold compile link

.PHONY: scaffold
scaffold: # create build directory
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(BUILD_DIR) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # link
	@echo Linking $(ASSEMBLY)...
	@clang $(LINKER_FLAGS) $(OBJ_FILES) -o $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)

.PHONY: compile
compile: #compile .c files
	@echo Compiling...

.PHONY: clean
clean: # clean build directory
	if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) /s /q
	if exist $(BUILD_DIR)\$(ASSEMBLY).lib del $(BUILD_DIR)\$(ASSEMBLY).lib /s /q
	if exist $(BUILD_DIR)\$(ASSEMBLY).pdb del $(BUILD_DIR)\$(ASSEMBLY).pdb /s /q
	if exist $(BUILD_DIR)\$(ASSEMBLY).ilk del $(BUILD_DIR)\$(ASSEMBLY).ilk /s /q
	if exist $(BUILD_DIR)\$(ASSEMBLY).exp del $(BUILD_DIR)\$(ASSEMBLY).exp /s /q
	if exist $(BUILD_DIR)\$(ASSEMBLY).a del $(BUILD_DIR)\$(ASSEMBLY).a /s /q
	if exist $(OBJ_DIR)\$(ASSEMBLY) rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.cpp.o: %.cpp # compile .c to .c.o object
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)