#!/usr/bin/env bash
set -eu

cd "$(dirname "${0}")/.."
ROOT="$(pwd)"
cd -
MODE=Debug
NUMBER_OF_CUBES=3
CUBE_LENGTH_MM=100
MARKER_LENGTH_PX=280
MARKER_SPACE_PX=40
CUBE_LENGTH_PX="$(($MARKER_LENGTH_PX + $MARKER_SPACE_PX * 2))"
export PATH="${ROOT}/external/build/opencv-prefix/src/opencv-build/bin/${MODE}:${PATH}"
seq 0 "$(("${NUMBER_OF_CUBES}" * 6 - 1))" | while read -r ID
do
    MARKER_TMP="$(mktemp).png"
    FIRST_MARKER="$(("${ID}" * 1))"
    "${ROOT}/build/create_marker/${MODE}/create_marker" --help -d=18 -w=1 -h=1 -l=${MARKER_LENGTH_PX} -s=${MARKER_SPACE_PX} -f="${FIRST_MARKER}" "${MARKER_TMP}"
    magick "${MARKER_TMP}" -negate "marker_${ID}.png"
    rm "${MARKER_TMP}"
done

EXTENT="$((${CUBE_LENGTH_PX} * 210 / ${CUBE_LENGTH_MM}))x$((${CUBE_LENGTH_PX} * 297 / ${CUBE_LENGTH_MM}))"
DENSITY="$((${CUBE_LENGTH_PX} * 254 / (${CUBE_LENGTH_MM} * 10)))"
seq 0 "$(("${NUMBER_OF_CUBES}" - 1))" | while read -r ID
do
    BASE="$((6 * "${ID}"))"
    magick montage -tile 1x2 -geometry "+0+${MARKER_SPACE_PX}" \
        -label "Cube ${ID}: Key marker (${BASE})" marker_"$(("${BASE}" + 0))".png \
        "tmp_${ID}-0.png"
    magick montage -tile 1x2 -geometry "+0+${MARKER_SPACE_PX}" \
        -label "Cube ${ID}: Sub marker ($((${BASE} + 1)))" marker_"$(("${BASE}" + 1))".png \
        -label "Cube ${ID}: Sub marker ($((${BASE} + 2)))" marker_"$(("${BASE}" + 2))".png \
        "tmp_${ID}-1.png"
    magick montage -tile 1x2 -geometry "+0+${MARKER_SPACE_PX}" \
        -label "Cube ${ID}: Sub marker ($((${BASE} + 3)))" marker_"$(("${BASE}" + 3))".png \
        -label "Cube ${ID}: Sub marker ($((${BASE} + 4)))" marker_"$(("${BASE}" + 4))".png \
        "tmp_${ID}-2.png"
    seq 0 2 | while read -r SHEET
    do
        magick convert -extent "${EXTENT}" \
            -gravity center "tmp_${ID}-${SHEET}.png" "sheet_${ID}-${SHEET}.png"
    done
done
magick convert -density ${DENSITY} -page ${EXTENT} $(ls sheet_*.png) "markers.pdf"
rm sheet_*.png tmp_*.png
