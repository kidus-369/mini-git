#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
INSTALL_DIR="$HOME/Programs/MiniGit"
BIN_NAME="MiniGit"
TARGET_BIN="$INSTALL_DIR/$BIN_NAME"
PATH_LINE='export PATH="$HOME/Programs/MiniGit:$PATH"'

append_path_line_if_missing() {
  local rc_file="$1"

  if [[ ! -f "$rc_file" ]]; then
    touch "$rc_file"
  fi

  if ! grep -Fqx "$PATH_LINE" "$rc_file"; then
    {
      echo
      echo "# Added by MiniGit installer"
      echo "$PATH_LINE"
    } >> "$rc_file"
    echo "Updated $rc_file"
  else
    echo "PATH already configured in $rc_file"
  fi
}

echo "Building MiniGit..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

echo "Installing binary to $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"
cp "$BUILD_DIR/$BIN_NAME" "$TARGET_BIN"
chmod +x "$TARGET_BIN"

echo "Configuring shell startup files..."
append_path_line_if_missing "$HOME/.bashrc"
append_path_line_if_missing "$HOME/.zshrc"

echo
echo "Install complete."
echo "Binary location: $TARGET_BIN"
echo "After reloading your shell, run: MiniGit"
echo "Or run now: export PATH=\"$HOME/Programs/MiniGit:$PATH\""
