#!/usr/bin/env sh

set -e

apt-get -y update
apt-get -y install \
  build-essential \
  cmake \
  python3-dev \
  python3-pip \
  clang-format-15 \
  clang-15 \
  unzip \
  wget \
  shellcheck

exit 0
