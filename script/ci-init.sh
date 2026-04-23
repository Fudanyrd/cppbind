#!/usr/bin/env sh

set -e

apt-get -y update
apt-get -y install \
  build-essential \
  cmake \
  python3-dev \
  python3-pip \
  clang-format-15 \
  shellcheck

exit 0
