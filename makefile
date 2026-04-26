CXX := g++
USE_REAL_RAYLIB ?= 1
RAYLIB_DIR ?= ../raylib

SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin

TARGET := $(BIN_DIR)/game

CXXFLAGS := -Wall -Wextra -std=c++17 -I include
LDLIBS :=

RAYLIB_STATIC_LIB := $(firstword $(wildcard \
	$(RAYLIB_DIR)/build/raylib/libraylib.a \
	$(RAYLIB_DIR)/build/libraylib.a \
))

ifeq ($(USE_REAL_RAYLIB),1)
	CXXFLAGS += -DNIMONSPOLI_USE_REAL_RAYLIB -I $(RAYLIB_DIR)/src
	ifeq ($(RAYLIB_STATIC_LIB),)
		LDLIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	else
		LDLIBS += $(RAYLIB_STATIC_LIB) -lGL -lm -lpthread -ldl -lrt -lX11
	endif
endif

SRCS := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/views/UIElement.cpp \
	$(SRC_DIR)/views/Button.cpp \
	$(SRC_DIR)/views/TextField.cpp \
	$(SRC_DIR)/views/Scene.cpp \
	$(SRC_DIR)/views/SceneManager.cpp \
	$(SRC_DIR)/views/MainMenuScene.cpp \
	$(SRC_DIR)/views/InGameScene.cpp \
	$(SRC_DIR)/views/Nimonspoli.cpp \
	$(SRC_DIR)/core/Account.cpp \
	$(SRC_DIR)/core/AccountManager.cpp \
	$(SRC_DIR)/core/AuctionManager.cpp \
	$(SRC_DIR)/core/BankruptException.cpp \
	$(SRC_DIR)/core/BankruptcyManager.cpp \
	$(SRC_DIR)/core/CardManager.cpp \
	$(SRC_DIR)/core/Game.cpp \
	$(SRC_DIR)/core/GameException.cpp \
	$(SRC_DIR)/core/GameManager.cpp \
	$(SRC_DIR)/core/InsufficientFundException.cpp \
	$(SRC_DIR)/core/InvalidAuctionException.cpp \
	$(SRC_DIR)/core/LiquidationManager.cpp \
	$(SRC_DIR)/core/TradeManager.cpp \
	$(SRC_DIR)/core/TurnManager.cpp \
	$(SRC_DIR)/utils/ActionTile.cpp \
	$(SRC_DIR)/utils/Board.cpp \
	$(SRC_DIR)/utils/BoardBuilder.cpp \
	$(SRC_DIR)/utils/CardTile.cpp \
	$(SRC_DIR)/utils/FestivalTile.cpp \
	$(SRC_DIR)/utils/LogEntry.cpp \
	$(SRC_DIR)/utils/Logger.cpp \
	$(SRC_DIR)/utils/PropertyTile.cpp \
	$(SRC_DIR)/utils/RailroadTile.cpp \
	$(SRC_DIR)/utils/SpecialTile.cpp \
	$(SRC_DIR)/utils/StreetTile.cpp \
	$(SRC_DIR)/utils/TaxTile.cpp \
	$(SRC_DIR)/utils/Tile.cpp \
	$(SRC_DIR)/utils/UtilityTile.cpp \
	$(SRC_DIR)/models/AbilityCard.cpp \
	$(SRC_DIR)/models/AbilityCardDeck.cpp \
	$(SRC_DIR)/models/ChanceCard.cpp \
	$(SRC_DIR)/models/CommunityChestCard.cpp \
	$(SRC_DIR)/models/DerivedAbilityCard.cpp \
	$(SRC_DIR)/models/Player.cpp \
	$(SRC_DIR)/models/TradeToPlayer.cpp \
	$(SRC_DIR)/data-layer/AccountDataManager.cpp \
	$(SRC_DIR)/data-layer/Config.cpp \
	$(SRC_DIR)/data-layer/ConfigComposer.cpp \
	$(SRC_DIR)/data-layer/ConfigHandler.cpp \
	$(SRC_DIR)/data-layer/FileIOException.cpp \
	$(SRC_DIR)/data-layer/GameStateSaver.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Build successful! Executable is at $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

rebuild: clean all

.PHONY: all clean rebuild run directories
