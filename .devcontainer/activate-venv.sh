#!/bin/bash
# Auto-activate Python virtual environment for devcontainer
# This script is idempotent and safe to source multiple times
VENV_PATH="/workspaces/python-mercuryapi/.venv"

# Only activate if venv exists and not already activated
if [ -d "$VENV_PATH" ] && [ -f "$VENV_PATH/bin/activate" ]; then
    if [ -z "$VIRTUAL_ENV" ] || [ "$VIRTUAL_ENV" != "$VENV_PATH" ]; then
        source "$VENV_PATH/bin/activate"
    fi
fi

