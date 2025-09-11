#!/usr/bin/env bash
set -e

add_line_if_dne () {
  if ! grep -q "$1" "$2" 2>/dev/null; then
    echo "$1" >> "$2"
  fi
}

# Function to add user local bin to PATH in bashrc
add_to_path_if_not_exists () {
    USER_BIN_PATH="$(python3 -m site --user-base)/bin"

    # Detect OS and shell config file
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if [ -n "$ZSH_VERSION" ]; then
            SHELL_CONFIG="$HOME/.zprofile"
        elif [ -n "$BASH_VERSION" ]; then
            SHELL_CONFIG="$HOME/.bash_profile"
        else
            SHELL_CONFIG="$HOME/.profile"
        fi
    else
        SHELL_CONFIG="$HOME/.bashrc"
    fi

    if ! echo "$PATH" | grep -q "$USER_BIN_PATH"; then
        echo "==> Adding $USER_BIN_PATH to PATH in $SHELL_CONFIG"
        echo "export PATH=\"$USER_BIN_PATH:\$PATH\"" >> "$SHELL_CONFIG"

        # Source config only if running interactively
        if [[ $- == *i* ]]; then
            # shellcheck disable=SC1090
            source "$SHELL_CONFIG"
            echo "Sourced $SHELL_CONFIG to update PATH in current session."
        else
            echo "Please restart your terminal or run 'source $SHELL_CONFIG' to update your PATH."
        fi
    else
        echo "PATH already contains $USER_BIN_PATH"
    fi
}

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "==> Detected Linux (Ubuntu/WSL assumed)"
    apt-get update

    echo "==> Install common tools"
    apt-get -y install tmux git vim curl

    echo "==> Install python tooling"
    apt-get -y install python3-pip virtualenv python3-autopep8 pylint

    echo "==> Install Python requirements"
    if python3 -m pip install -h | grep -q -- '--break-system-packages'; then
        python3 -m pip install -r requirements.txt --break-system-packages
    else
        PYTHONBREAKSYSTEMPACKAGES=1 python3 -m pip install -r requirements.txt
    fi

    # Add user-local bin to PATH
    add_to_path_if_not_exists

    echo "==> Install tooling for CAN"
    apt-get -y install can-utils

    echo "==> Install nlohmann-json"
    apt-get install -y nlohmann-json3-dev

    echo "==> Install clang and gcc"
    apt-get -y install binutils-arm-none-eabi gcc-arm-none-eabi libncurses-dev software-properties-common libsdl2-dev
    add-apt-repository -y ppa:ubuntu-toolchain-r/test
    apt-get -y install gcc-11 g++-11 clang clang-format gdb

    ln -sf "$(which gcc-11)" /usr/bin/gcc

    # Dynamically find the latest installed clang and clang-format
    CLANG_BIN=$(ls /usr/bin/clang-[0-9]* 2>/dev/null | sort -V | tail -n 1)
    CLANG_FORMAT_BIN=$(ls /usr/bin/clang-format-[0-9]* 2>/dev/null | sort -V | tail -n 1)

    if [ -n "$CLANG_BIN" ]; then
      ln -sf "$CLANG_BIN" /usr/bin/clang
      ln -sf "${CLANG_BIN/clang/clang++}" /usr/bin/clang++
    fi

    if [ -n "$CLANG_FORMAT_BIN" ]; then
      ln -sf "$CLANG_FORMAT_BIN" /usr/bin/clang-format
    fi

    echo "==> Install other toolchain pieces"
    apt-get -y install minicom openocd

    echo "==> Setup for minicom"
    mkdir -p /etc/minicom
    touch /etc/minicom/minirc.dfl
    add_line_if_dne "pu addcarreturn    Yes" /etc/minicom/minirc.dfl
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "==> Detected macOS"
    echo "Installing packages with Homebrew (brew)..."

    # Check if brew exists
    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew not found. Please install it from https://brew.sh/"
        exit 1
    fi

    brew update

    brew tap ArmMbed/homebrew-formulae

    brew install \
      tmux \
      git \
      vim \
      curl \
      arm-none-eabi-gcc \
      gcc \
      llvm \
      gdb \
      minicom \
      openocd \
      python@3 \
      sdl2 \
      nlohmann-json

    echo "==> Install Python requirements"

    # Dynamically find pip3 path (should be in PATH)
    PIP3_CMD=$(command -v pip3 || true)

    if [[ -z "$PIP3_CMD" ]]; then
      echo "pip3 not found in PATH, please check your Python installation."
      exit 1
    fi

    "$PIP3_CMD" install --upgrade pip
    "$PIP3_CMD" install -r requirements.txt

    add_to_path_if_not_exists

else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi