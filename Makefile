CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))
CXXFLAGS := $(shell xml2-config --cflags) -Wall -g -DDEBUG
LD_FLAGS := $(CXXFLAGS) -lGL -lGLU -lglut $(shell xml2-config --libs)
CC_FLAGS := -MMD -MP $(CXXFLAGS)
EXEC_FILE := pj

$(EXEC_FILE): $(OBJ_FILES)
	g++ -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.d

-include $(OBJ_FILES:.o=.d)
