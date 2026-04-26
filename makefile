# Makefile for the UI integration build.
# Core and data-layer code that is not ready yet is intentionally excluded.
# UI talks to include/coredummy through IGameFacade until the real core is ready.
#
# Expected folder structure for real GUI runtime:
# PARENT_FOLDER/
# ├── raylib/
# └── tugas-besar-1-swt-5/
#
# Build raylib first:
# cd ../raylib
# mkdir -p build
# cd build
# cmake -DBUILD_SHARED_LIBS=OFF ..
# make -j$(nproc)

CXX := g++

RAYLIB_DIR ?= ../raylib
USE_REAL_RAYLIB ?= $(if $(wildcard $(RAYLIB_DIR)),1,0)

SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
DATA_DIR    := data
CONFIG_DIR  := config

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
    $(SRC_DIR)/coredummy/DummyBoardFactory.cpp \
    $(SRC_DIR)/coredummy/MockGameFacade.cpp \
    $(SRC_DIR)/core/RealGameFacade.cpp \
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

all: check-raylib directories $(TARGET)

check-raylib:
ifeq ($(USE_REAL_RAYLIB),1)
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
        echo "Raylib folder not found: $(RAYLIB_DIR)"; \
        echo "Clone it beside this project: cd .. && git clone https://github.com/raysan5/raylib.git"; \
        exit 1; \
    fi
	@if [ -z "$(RAYLIB_STATIC_LIB)" ]; then \
        echo "Local static raylib library was not found."; \
        echo "I will try system raylib linkage with -lraylib."; \
        echo "If linking fails, build local raylib first."; \
    fi
endif

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Build successful! Executable is at $(TARGET)"
	@if [ "$(USE_REAL_RAYLIB)" = "1" ]; then \
        echo "Built with real Raylib linkage."; \
        if [ -n "$(RAYLIB_STATIC_LIB)" ]; then \
            echo "Using local Raylib: $(RAYLIB_STATIC_LIB)"; \
        else \
            echo "Using system Raylib: -lraylib"; \
        fi; \
    else \
        echo "Built with Raylib stub. This mode is compile-only and will not open a GUI window."; \
    fi

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

stub:
	$(MAKE) USE_REAL_RAYLIB=0

run-stub:
	$(MAKE) USE_REAL_RAYLIB=0 run

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

rebuild: clean all

.PHONY: all clean rebuild run directories stub run-stub check-raylib
