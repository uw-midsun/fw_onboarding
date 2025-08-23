#!/bin/bash
set -e

echo "Updating package lists and upgrading system..."
sudo apt update && sudo apt full-upgrade -y

add_line_if_dne () {
  if ! grep -Fxq "$1" "$2"; then
    echo "$1" | sudo tee -a "$2"
  fi
}

echo "Installing common tools..."
sudo apt-get update
sudo apt-get install -y tmux git vim curl

echo "Installing Python tooling..."
sudo apt-get install -y python3-pip virtualenv python3-autopep8 pylint
python3 -m pip install --upgrade pip
python3 -m pip install cpplint python-can cantools Jinja2 PyYAML pyserial

echo "Installing CAN tooling..."
sudo apt-get install -y can-utils

echo "Installing Clang/GCC toolchains..."
sudo apt-get install -y binutils-arm-none-eabi libncurses5 libncursesw5 \
  gcc-11 g++-11 clang-11 clang-format-12 gdb

echo "Installing nlohmann-json..."
sudo apt-get install -y nlohmann-json3-dev

echo "Installing ARM GCC toolchain if not already present..."
ARM_GCC_DIR="/usr/local/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi"
if [ ! -d "$ARM_GCC_DIR" ]; then
  wget https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi.tar.xz -O arm-gcc.tar.xz
  sudo tar -xf arm-gcc.tar.xz -C /usr/local
  rm arm-gcc.tar.xz
  add_line_if_dne "export PATH=\$PATH:$ARM_GCC_DIR/bin" /etc/profile
fi

echo "Installing other toolchain utilities..."
sudo apt-get install -y minicom openocd

echo "Configuring minicom..."
sudo touch /etc/minicom/minirc.dfl
add_line_if_dne "pu addcarreturn    Yes" /etc/minicom/minirc.dfl

echo "gcc version: $(gcc --version | head -n 1)"
echo "clang version: $(clang --version | head -n 1)"
echo "arm-none-eabi-gcc version: $(arm-none-eabi-gcc --version | head -n 1)"
echo "openocd version: $(openocd --version)"
echo "gdb version: $(gdb --version | head -n 1)"
echo "minicom version: $(minicom --version | head -n 1)"
echo "can-utils version: $(can-utils --version | head -n 1)"
echo "python3 version: $(python3 --version)"
echo "pip version: $(pip --version)"
