CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++11 $(DFLAGS) -Iinclude
LDFLAGS := -pthread

SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

COMMON_SRCS := \
  $(SRC_DIR)/common/Messages.cpp \
  $(SRC_DIR)/common/Socket.cpp \
  $(SRC_DIR)/common/ClientSocket.cpp

CLIENT_SRCS := \
  $(SRC_DIR)/client/ClientMain.cpp \
  $(SRC_DIR)/client/ClientStub.cpp \
  $(SRC_DIR)/client/ClientThread.cpp \
  $(SRC_DIR)/client/ClientTimer.cpp

SERVER_SRCS := \
  $(SRC_DIR)/server/ServerMain.cpp \
  $(SRC_DIR)/server/ServerSocket.cpp \
  $(SRC_DIR)/server/ServerStub.cpp \
  $(SRC_DIR)/server/ServerThread.cpp

COMMON_OBJS := $(COMMON_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
CLIENT_OBJS := $(CLIENT_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
SERVER_OBJS := $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

TARGETS := $(BIN_DIR)/client $(BIN_DIR)/server

all: $(TARGETS)

$(BIN_DIR)/client: $(CLIENT_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BIN_DIR)/server: $(SERVER_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp | $(BUILD_DIR)/client
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp | $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.cpp | $(BUILD_DIR)/common
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $@

$(BUILD_DIR)/client:
	mkdir -p $@

$(BUILD_DIR)/server:
	mkdir -p $@

$(BUILD_DIR)/common:
	mkdir -p $@

debug: DFLAGS := -ggdb -DDEBUG
debug: all

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean debug
