# Makefile for C++ Client only

# Compiler settings
CXX = g++
<<<<<<< HEAD
CXXFLAGS = -std=c++17 -Wall -g -mrdrnd -I src/client/include
LDFLAGS = -L -lcryptopp -static -lpthread -lws2_32 
=======
CXXFLAGS = -std=c++17 -Wall -g -mrdrnd -I 
LDFLAGS = -L "C:/Users/hwath/cryptopp" -lcryptopp -static -lpthread -lws2_32 
>>>>>>> e4c683a210f171b5cf47c07656b36cf0b8543c45
SRC_DIR = src/client/src
CLIENT_DIR = src/client/src/client
ENCRYPTION_DIR = src/client/src/encryption
INCLUDE_DIR = src/client/include
BUILD_DIR = src/client/build

# CLIENT source files
CLIENT_SRC = $(SRC_DIR)/main.cpp \
			 $(CLIENT_DIR)/protocolhandler.cpp \
			 $(CLIENT_DIR)/client.cpp \
			 $(CLIENT_DIR)/helpers.cpp \
			 $(CLIENT_DIR)/user.cpp \
             $(ENCRYPTION_DIR)/AESWrapper.cpp \
			 $(ENCRYPTION_DIR)/RSAWrapper.cpp \

# CLIENT object files
CLIENT_OBJ =  $(CLIENT_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Output executable
CLIENT_EXEC = client.exe

# Default target
all: $(CLIENT_EXEC)

# Create the build directory if it doesn't exist
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@) 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilation rule for CLIENT-side code
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Link the CLIENT object files into the final executable
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJ) -o $(CLIENT_EXEC) $(LDFLAGS)

# Clean up the build
clean:
	rm -rf $(BUILD_DIR) $(CLIENT_EXEC)

# Rebuild everything from scratch
rebuild: clean all

# Run the CLIENT
run: $(CLIENT_EXEC)
	./$(CLIENT_EXEC)

