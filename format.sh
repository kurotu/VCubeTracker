#!/bin/env bash
set -eu

mapfile -t FILES < <(find . -name "*.cpp" -o -name "*.h" \
  | grep -v ./build \
  | grep -v ./external \
  | grep -v ./vendor \
)

clang-format -i "${FILES[@]}"
