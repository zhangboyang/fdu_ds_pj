CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))

# useful flags when debugging: -g -O0 -DDEBUG -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS := $(shell xml2-config --cflags) $(shell wx-config --cxxflags) -Wall -g -O0 -DDEBUG -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
LD_FLAGS := $(CXXFLAGS) -lGL -lGLU -lglut $(shell xml2-config --libs) $(shell wx-config --libs)
CC_FLAGS := -MMD -MP $(CXXFLAGS)
EXEC_FILE := pj

$(EXEC_FILE): $(OBJ_FILES)
	g++ -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.d

-include $(OBJ_FILES:.o=.d)
