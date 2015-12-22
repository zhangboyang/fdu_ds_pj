CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))

# useful g++ flags when debugging: -g -O0 -DDEBUG -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
# useful linking flags (google pprof): -lprofiler -ltcmalloc
CXXFLAGS := $(shell xml2-config --cflags) $(shell wx-config --cxxflags) -Wall -g -O2 -DDEBUG
ifeq ($(OS),Windows_NT)
# compile options for wxwidgets: ./configure --prefix=/mingw --disable-all-features --enable-gui --disable-debug --enable-mousewheel --enable-wxdib --enable-image --enable-menus --without-subdirs BUILD=release --enable-stattext --enable-textctrl --enable-button --enable-textdlg --enable-uxtheme --enable-dynlib
	CXXFLAGS += -DZBY_OS_WINDOWS
	LD_FLAGS := $(CXXFLAGS) -lopengl32 -lglu32 -lglew32 -lfreeglut $(shell xml2-config --libs) $(shell wx-config --libs) -Wl,--subsystem,console
else
	CXXFLAGS += -DZBY_OS_LINUX
	LD_FLAGS := $(CXXFLAGS) -lGL -lGLU -lGLEW -lglut $(shell xml2-config --libs) $(shell wx-config --libs) -lprofiler -ltcmalloc
endif
CC_FLAGS := -MMD -MP $(CXXFLAGS)
EXEC_FILE := pj

$(EXEC_FILE): $(OBJ_FILES)
	g++ -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.d

-include $(OBJ_FILES:.o=.d)
