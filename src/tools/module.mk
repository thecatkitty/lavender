TOOLSOURCES = deckey.cpp enckey.cpp xor.cpp

tools: $(TOOLSOURCES:%.cpp=$(BIN)/$(DIR)/%.exe)

$(BIN)/$(DIR)/%.exe: $(SRC)/$(DIR)/%.cpp
	@mkdir -p $(@D)
	@mkdir -p $(OBJ)/$(DIR)
	cl.exe /nologo /std:c++latest /EHsc /Fo$(OBJ)/$(DIR)/ /Fe$@ $^
