PROJECT_ROOT := $(shell pwd)
BIN_DIR = $(PROJECT_ROOT)/bin

# Subdirectory paths
CLIENT_DIR = $(PROJECT_ROOT)/src/client
LAUNCHER_DIR = $(PROJECT_ROOT)/src/launcher
SERVER_DIR = $(PROJECT_ROOT)/src/server

# Binary paths
CLIENT_BIN = $(BIN_DIR)/client
LAUNCHER_BIN = $(BIN_DIR)/launcher
SERVER_BIN = $(BIN_DIR)/server

.PHONY: all clean run client launcher server

all: client launcher server

client:
	$(MAKE) -C $(CLIENT_DIR)

launcher:
	$(MAKE) -C $(LAUNCHER_DIR)

server:
	$(MAKE) -C $(SERVER_DIR)

run: all
	@if [ ! -f "$(LAUNCHER_BIN)" ]; then \
		echo "❌ Launcher not found. Run 'make all' first."; \
		exit 1; \
	fi
	@echo "🚀 Starting Axiom..."
	cd $(BIN_DIR) && ./launcher

clean:
	$(MAKE) -C $(CLIENT_DIR) clean
	$(MAKE) -C $(LAUNCHER_DIR) clean
	$(MAKE) -C $(SERVER_DIR) clean
	@echo "✓ All binaries cleaned"

# Development helpers
rebuild: clean all

status:
	@echo "📁 Build Status:"
	@[ -f "$(CLIENT_BIN)" ] && echo "  ✓ client" || echo "  ✗ client"
	@[ -f "$(LAUNCHER_BIN)" ] && echo "  ✓ launcher" || echo "  ✗ launcher"
	@[ -f "$(SERVER_BIN)" ] && echo "  ✓ server" || echo "  ✗ server"
	@echo "📁 Assets:"
	@[ -d "$(BIN_DIR)/assets" ] && echo "  ✓ assets present" || echo "  ✗ assets missing"

# Quick run without rebuild
quick-run:
	cd $(BIN_DIR) && ./launcher
