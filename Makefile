CLIENT_DIR = client
SERVER_DIR = server
GATEWAY_DIR = gateway
AI_DIR = ai
COORDINATOR_DIR = coordinator
LAUNCHER_DIR = launcher
GUI_CLIENT_DIR = gui_client
TUI_CLIENT_DIR = tui_client
BUILD_DIR = build
BIN_DIR = bin/axiom

build_dir:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)/{assets,libs,maps,rules,scripts}
