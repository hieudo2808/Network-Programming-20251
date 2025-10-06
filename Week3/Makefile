CXX = g++
SRC1 = resolver.cpp
SRC2 = resolverV2.cpp

BIN1 = resolver
BIN2 = resolverV2

all: $(BIN1) $(BIN2)

$(BIN1): $(SRC1)
	@echo "Compiling $(SRC1)..."
	$(CXX) -o $(BIN1) $(SRC1)

$(BIN2): $(SRC2)
	@echo "Compiling $(SRC2)..."
	$(CXX) -o $(BIN2) $(SRC2)

clean:
	@echo "Cleaning build files..."
	rm -f $(BIN1) $(BIN2)

.PHONY: all clean
