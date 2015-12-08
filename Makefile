CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))
CXXFLAGS := -Wall -g
LD_FLAGS := $(CXXFLAGS) -ltinyxml2 -lGL -lGLU -lglut
CC_FLAGS := -MMD $(CXXFLAGS)
EXEC_FILE := pj

$(EXEC_FILE): $(OBJ_FILES)
	g++ -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.d

-include $(OBJFILES:.o=.d)
