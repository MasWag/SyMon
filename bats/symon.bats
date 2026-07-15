#!/usr/bin/env bats

setup() {
    PROJECT_ROOT="${BATS_TEST_DIRNAME}/.."
    BUILD_DIR="${PROJECT_ROOT}/build"
    EXAMPLE_DIR="${PROJECT_ROOT}/example"
    cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
}

extract_example_io() {
    local spec="$1"
    local input="$2"
    local expected="$3"

    awk '/END_INPUT/{f=0}f;/BEGIN_INPUT/{f=1}' "$spec" |
        sed 's/^# *//;' > "$input"
    awk '/END_OUTPUT/{f=0}f;/BEGIN_OUTPUT/{f=1}' "$spec" |
        sed 's/^# *//;' |
        tr -d '[:space:]' > "$expected"
}

assert_example_output() {
    local mode="$1"
    local spec="$2"
    local input
    local expected

    input=$(mktemp)
    expected=$(mktemp)
    extract_example_io "$spec" "$input" "$expected"

    "${BUILD_DIR}/symon" $mode "$spec" -i "$input" |
        tr -d '[:space:]' |
        diff - "$expected"
    rm -f "$input" "$expected"
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
    assert_example_output "-nf" "$SPEC"
    assert_example_output "-bnf" "${EXAMPLE_DIR}/features2.symon"
}

@test "non-integer boolean" {
    assert_example_output "-bnf" "${EXAMPLE_DIR}/non_integer/boolean.symon"
}

@test "non-integer data-parametric" {
    assert_example_output "-dnf" "${EXAMPLE_DIR}/non_integer/dataparametric.symon"
}

@test "non-integer parametric" {
    assert_example_output "-pnf" "${EXAMPLE_DIR}/non_integer/parametric.symon"
}

@test "expr_assignment" {
    assert_example_output "-bnf" "${EXAMPLE_DIR}/expr_assignment.symon"
    assert_example_output "-bnf" "${EXAMPLE_DIR}/expr_assignment2.symon"
}

@test "data parametric unobservable" {
    readonly SPEC="${EXAMPLE_DIR}/unobservable_data_parametric.symon"
    INPUT=$(mktemp)
    awk '/END_INPUT/{f=0}f;/BEGIN_INPUT/{f=1}' "$SPEC" |
        sed 's/^# *//;' > "$INPUT"
    EXPECTED_OUTPUT=$(mktemp)
    awk '/END_OUTPUT/{f=0}f;/BEGIN_OUTPUT/{f=1}' "$SPEC" |
        sed 's/^# *//;' |
        tr -d '[:space:]' > "$EXPECTED_OUTPUT"
    # We ignore the difference in white spaces
    "${BUILD_DIR}/symon" -dnf "$SPEC" -i "$INPUT" |
        tr -d '[:space:]' |
        diff - "$EXPECTED_OUTPUT"
    rm -f "$INPUT" "$EXPECTED_OUTPUT"
}

@test "boolean unobservable" {
    assert_example_output "-bnf" "${EXAMPLE_DIR}/unobservable_boolean.symon"
    assert_example_output "-bnf" "${EXAMPLE_DIR}/unobservable2_boolean.symon"
    assert_example_output "-dnf" "${EXAMPLE_DIR}/unobservable2_data_parametric.symon"
}

@test "decimal_inputs" {
    assert_example_output "-dnf" "${EXAMPLE_DIR}/decimal_inputs.symon"
    assert_example_output "-dnf" "${EXAMPLE_DIR}/decimal_inputs2.symon"
}

@test "decimal_time" {
    assert_example_output "-bnf" "${EXAMPLE_DIR}/decimal_time_boolean.symon"
    assert_example_output "-dnf" "${EXAMPLE_DIR}/decimal_time_data_parametric.symon"
    assert_example_output "-pnf" "${EXAMPLE_DIR}/decimal_time_parametric.symon"
}
