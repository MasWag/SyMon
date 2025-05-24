#!/bin/sh
set -eu

FILE="CMakeLists.txt"

# Check for pipx
if ! command -v pipx >/dev/null 2>&1; then
  echo "ERROR: pipx is not installed. Please install pipx to use this hook."
  exit 1
fi

# Compute original hash
ORIGINAL_HASH=$(sha256sum "$FILE" | cut -d ' ' -f 1)

# Format in place
pipx run cmake-format -i "$FILE"

# Compute new hash
NEW_HASH=$(sha256sum "$FILE" | cut -d ' ' -f 1)

# Compare
if [ "$ORIGINAL_HASH" != "$NEW_HASH" ]; then
  echo "ERROR: $FILE was not properly formatted and has been updated."
  echo "Please review the changes and re-stage the file before committing:"
  echo ""
  echo "  git add $FILE"
  echo ""
  exit 1
fi
