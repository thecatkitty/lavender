LX_CC = gcc
W32_CC = i686-w64-mingw32-gcc-win32

TOOLSOURCES = enckey.cpp

tools: $(TOOLSOURCES:%.cpp=$(BIN)/$(DIR)/%.exe) $(TOOLSOURCES:%.cpp=$(BIN)/$(DIR)/%)

$(BIN)/$(DIR)/%.exe: $(SRC)/$(DIR)/%.cpp
	@mkdir -p $(@D)
	@mkdir -p $(OBJ)/$(DIR)
	$(W32_CC) -std=c++17 --static -s -o $@ $^ -lstdc++

$(BIN)/$(DIR)/%: $(SRC)/$(DIR)/%.cpp
	@mkdir -p $(@D)
	@mkdir -p $(OBJ)/$(DIR)
	$(LX_CC) -std=c++17 --static -s -o $@ $^ -lstdc++
