#!/usr/bin/env bats

setup() {
    PROJECT_ROOT="${BATS_TEST_DIRNAME}/.."
    BUILD_DIR="${PROJECT_ROOT}/build"
    EXAMPLE_DIR="${PROJECT_ROOT}/example"
    cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
}

@test "frequent" {
    FREQUENT_DIR="${EXAMPLE_DIR}/exim4/frequent"
    size=$("${BUILD_DIR}/symon" -dnf "${FREQUENT_DIR}/frequent.symon" -i "${FREQUENT_DIR}/example.log" | wc -l)
    [ "$size" -eq 1 ]
}

@test "periodic" {
    PERIODIC_DIR="${EXAMPLE_DIR}/periodic"
    size=$("${BUILD_DIR}/symon" -pnf "${PERIODIC_DIR}/periodic.symon" -i "${PERIODIC_DIR}/example.log" | wc -l)
    [ "$size" -eq 9 ]
}

@test "features" {
    readonly SPEC="${EXAMPLE_DIR}/features.symon"
    INPUT=$(mktemp)
    awk '/END_INPUT/{f=0}f;/BEGIN_INPUT/{f=1}' "$SPEC" |
        sed 's/^# *//;' > "$INPUT"
    EXPECTED_OUTPUT=$(mktemp)
    awk '/END_OUTPUT/{f=0}f;/BEGIN_OUTPUT/{f=1}' "$SPEC" |
        sed 's/^# *//;' |
        tr -d '[:space:]' > "$EXPECTED_OUTPUT"
    # We ignore the difference in white spaces
    "${BUILD_DIR}/symon" -nf "${EXAMPLE_DIR}/features.symon" -i "${INPUT}" |
        tr -d '[:space:]' |
        diff - "$EXPECTED_OUTPUT"
    rm -f "$INPUT" "$EXPECTED_OUTPUT"
}
