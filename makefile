# Makefile for Nimonspoli CLI build.
# No GUI / Raylib dependency. Pure terminal game.

CXX := g++

SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
DATA_DIR    := data
CONFIG_DIR  := config

TARGET := $(BIN_DIR)/game

CXXFLAGS := -Wall -Wextra -std=c++17 -I include

SRCS := \
    $(SRC_DIR)/main.cpp \
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
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Executable: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned."

rebuild: clean all

.PHONY: all clean rebuild run directories
