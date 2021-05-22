#!/usr/bin/env bash
set -eu

cd "$(dirname "${0}")/.."
ROOT="$(pwd)"
cd -

export PATH="${ROOT}/build/calibration/Release:${PATH}"

calibrate_camera_charuco -w=5 -h=7 --sl=0.0385 --ml=0.023 -d=18 "${1}"