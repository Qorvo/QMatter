#!/bin/sh

check_installed_dependency ()
{
    $1 --version foo >/dev/null 2>&1 || {
        echo >&2 "$1 not installed. installing........"
        return 1
    }
    return 0
}

install_common_dependencies ()
{
    sudo apt-get update
    sudo apt-get upgrade
    sudo apt-get install git clang make ninja-build
}

install_arm_gcc_emb ()
{
    wget -P /tmp https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
    sudo mkdir -p /opt/TOOL_ARMGCCEMB
    sudo tar -xf /tmp/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 -C /opt/TOOL_ARMGCCEMB
}

install_gn ()
{
    git clone https://gn.googlesource.com/gn /tmp/gn
    python3 /tmp/gn/build/gen.py --out-path=/tmp/gn/out
    ninja -C /tmp/gn/out
    sudo cp /tmp/gn/out/gn /usr/local/bin/gn
    sudo chmod +x /usr/local/bin/gn
}

export PATH=$PATH:/opt/TOOL_ARMGCCEMB/gcc-arm-none-eabi-9-2019-q4-major/bin
export TOOLCHAIN=/opt/TOOL_ARMGCCEMB/gcc-arm-none-eabi-9-2019-q4-major
export MAKEFLAGS=-s

if ! check_installed_dependency make; then
    install_common_dependencies
fi

if ! check_installed_dependency arm-none-eabi-gcc; then
    install_arm_gcc_emb
fi

if ! check_installed_dependency gn; then
    install_gn
fi

if ! check_installed_dependency ninja; then
    install_common_dependencies
fi

git submodule update --init --recursive
