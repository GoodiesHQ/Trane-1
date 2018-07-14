TARGET=trane
CXX=g++
RM=rm -f
# CPPFLAGS=-Wall -std=c++14 -pthread -I./inc -I/usr/include -I/usr/local/include -Os -fdata-sections -ffunction-sections -Wl,--gc-sections
CPPFLAGS=-Wall -std=c++14 -pthread -I./inc -I/usr/include -I/usr/local/include -O0
SOURCES_SERVER=./src/server.cpp
SOURCES_CLIENT=./src/client.cpp
INCLUDES:=$(wildcard inc/*.hpp)

$(TARGET): obj
	@$(LD) $(TARGET) $(LFLAGS) $(OBJECTS)
	@echo "Link Complete"

obj: client server
	@echo "Compile Complete"

client: $(SOURCES_CLIENT)
	$(CXX) $(SOURCES_CLIENT) $(CPPFLAGS) -o $(TARGET)_client

server: $(SOURCES_SERVER)
	$(CXX) $(SOURCES_SERVER) $(CPPFLAGS) -o $(TARGET)_server

# clean:
# @echo "Clean Complete"
