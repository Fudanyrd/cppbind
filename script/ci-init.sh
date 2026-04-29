#!/usr/bin/env sh

set -e

apt-get -y update
apt-get -y install \
  build-essential \
  cmake \
  python3-dev \
  python3-pip \
  clang-format-15 \
  clang-tidy-15 \
  doxygen \
  clang-15 \
  gawk \
  unzip \
  wget \
  shellcheck

exit 0
