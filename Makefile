CSS := g++

CXXFLAGS := -O3 -march=native -DNDEBUG -std=c++17 

INC := -Iinclude

SRC := src/main.cpp src/traversal.cpp src/matmul.cpp
BIN := cache_bench

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $(SRC)

clean:
	rm -f $(BIN)

run: $(BIN)
	./$(BIN) > results.csv
	@echo "wrote results.csv