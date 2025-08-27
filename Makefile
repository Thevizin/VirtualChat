CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
TARGETS = server_linux client_linux
UTILS_DIR = Utils

// Arquivos fonte
SERVER_SOURCES = server_linux.cpp $(UTILS_DIR)/MessagesBlock.cpp $(UTILS_DIR)/ConnectionsBlock.cpp
CLIENT_SOURCES = client_linux.cpp

// Arquivos dos objects
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

// Target (server e client)
all: $(TARGETS)

// Server target
server_linux: $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJECTS)

// Client target
client_linux: $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJECTS)

// Compilar server objects
server_linux.o: server_linux.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

// Compilar client objects
client_linux.o: client_linux.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

// Compilar utility objects
$(UTILS_DIR)/%.o: $(UTILS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

// remover os targets
clean:
	rm -f $(TARGETS) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)



// Rodar o server
run-server: server_linux
	./server_linux

// Rodar o client
run-client: client_linux
	./client_linux



.PHONY: all clean install uninstall run-server run-client help
