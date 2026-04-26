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
	@powershell -NoProfile -Command "if (!(Test-Path '$(RAYLIB_DIR)')) { Write-Host 'Raylib folder not found: $(RAYLIB_DIR)'; Write-Host 'Clone it beside this project: cd .. && git clone https://github.com/raysan5/raylib.git'; exit 1 }"
	@powershell -NoProfile -Command "if ('$(RAYLIB_STATIC_LIB)' -eq '') { Write-Host 'Local static raylib library was not found.'; Write-Host 'I will try system raylib linkage with -lraylib.'; Write-Host 'If linking fails, build local raylib first.' }"
endif

directories:
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(OBJ_DIR)','$(BIN_DIR)','$(DATA_DIR)','$(CONFIG_DIR)' | Out-Null"

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)
	@echo "Build successful! Executable is at $(TARGET)"
	@powershell -NoProfile -Command "if ('$(USE_REAL_RAYLIB)' -eq '1') { Write-Host 'Built with real Raylib linkage.'; if ('$(RAYLIB_STATIC_LIB)' -ne '') { Write-Host 'Using local Raylib: $(RAYLIB_STATIC_LIB)' } else { Write-Host 'Using system Raylib: -lraylib' } } else { Write-Host 'Built with Raylib stub. This mode is compile-only and will not open a GUI window.' }"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

stub:
	$(MAKE) USE_REAL_RAYLIB=0

run-stub:
	$(MAKE) USE_REAL_RAYLIB=0 run

clean:
	@powershell -NoProfile -Command "Remove-Item -Recurse -Force '$(OBJ_DIR)','$(BIN_DIR)' -ErrorAction SilentlyContinue"
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

rebuild: clean all

.PHONY: all clean rebuild run directories stub run-stub check-raylib
