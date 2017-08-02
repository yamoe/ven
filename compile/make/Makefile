

TARGET   = ven
SRC_DIR = ../../ven
INC  = ../../ven/include


CXX = g++
CXXFLAGS = -g -O0 -W -Wall -std=c++11 -pthread -I$(INC) -Wno-unused 
#CXXFLAGS = -O2 -DNDEBUG -std=c++11 -pthread -I$(INC) -Wno-unused 
LDFLAGS = -ldl
OBJECTS  = $(wildcard $(SRC_DIR)/*.cpp)



all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful!"

clean:
	@rm -rf $(TARGET)
	@find $(SRC_DIR) -name '*.o' -exec rm {} \;
	@echo "Clean successful!"

main.o: main.cpp $(INC)/ven/util/inc.h $(INC)/ven/util/thread.h




