TARGET=trane
CXX=g++ -c
LD=g++ -o
RM=rm -f
CPPFLAGS=-Wall -std=c++14 -lpthread
LFLAGS=-Wall -lpthread 
SOURCES:=$(wildcard src/*.cpp)
INCLUDES:=$(wildcard inc/*.hpp)
OBJECTS:=$(SOURCES:.c=*.o)

$(TARGET): obj
	@$(LD) $(TARGET) $(LFLAGS) $(OBJECTS)
	@echo "Link Complete"

obj: $(SOURCES)
	@$(CXX) $(CPPFLAGS) $(SOURCES)
	@echo "Compile Complete"

# clean:
# @echo "Clean Complete"
