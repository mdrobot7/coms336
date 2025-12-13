TARGET_EXE := render.exe
BUILD_DIR := ./build
SRC_DIR := ./src
LIB_DIR := ./lib

STB_PATH := $(LIB_DIR)/stb

NLOHMANN_JSON_PATH := $(LIB_DIR)/json/single_include

TINYOBJLOADER_PATH := $(LIB_DIR)/tinyobjloader

CC := g++
COMMON_FLAGS := -O3 -g
CFLAGS := -Wall -Wextra
CPPFLAGS := -MMD -MP -I$(STB_PATH) -I$(NLOHMANN_JSON_PATH) -I$(TINYOBJLOADER_PATH)
LDFLAGS := --gc-sections

SOURCES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) # Shell "find" sucks on Windows, so we're doing this
OBJS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d) # Generate sub-makefiles for each C source

# Turn LDFLAGS into -Wl,[flag],[flag]... to pass to GCC
space := $() $()
comma := ,
LDFLAGS := -Wl,$(subst $(space),$(comma),$(LDFLAGS))

ifneq ($(wildcard ./venv/Scripts/activate),)
$(eval PYTHON := ./venv/Scripts/python.exe)
$(eval PIP := ./venv/Scripts/pip.exe)
else ifneq ($(wildcard ./venv/bin/activate),)
$(eval PYTHON := ./venv/bin/python)
$(eval PIP := ./venv/bin/pip)
else
$(info Python venv not found, generating...)
$(shell python -m venv ./venv)
endif

all: $(BUILD_DIR)/$(TARGET_EXE) compiledb

# Link C sources into final executable
$(BUILD_DIR)/$(TARGET_EXE): $(OBJS)
	$(CC) $(COMMON_FLAGS) $(LDFLAGS) $(OBJS) -o $@

# Build C sources
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build ASM sources
$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(CPPFLAGS) $(DFU_CPPFLAGS) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

# Generate ./build/compile_commands.json using compiledb
compiledb: $(BUILD_DIR)/$(TARGET_EXE)
	mkdir -p $(BUILD_DIR)
	$(PYTHON) -m compiledb -n -o $(BUILD_DIR)/compile_commands.json make

documentation:
	pdflatex -halt-on-error -output-directory=docs -job-name=docs -aux-directory=docs/latex_aux ./docs/main.tex

setup:
	mkdir -p ./.vscode
	cp ./scripts/vscode/* ./.vscode
	$(PIP) install compiledb

clean:
	rm -r $(BUILD_DIR)

.PHONY: all compiledb clean
-include $(DEPS)