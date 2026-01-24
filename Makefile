CXXFLAGS = -Os -std=c++17 -I./src
LDFLAGS = -lm

SRCDIR = src
SOURCES = $(SRCDIR)/sysfs_reader.cpp \
          $(SRCDIR)/config.cpp \
          $(SRCDIR)/temperature_sensor.cpp \
          $(SRCDIR)/frequency_controller.cpp \
          $(SRCDIR)/pid_controller.cpp \
          $(SRCDIR)/daemon_manager.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all : cputemp

cputemp : $(OBJECTS) $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main.cpp $(OBJECTS) -o cputemp $(LDFLAGS)

$(SRCDIR)/%.o : $(SRCDIR)/%.cpp $(SRCDIR)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -f cputemp $(OBJECTS) *~
