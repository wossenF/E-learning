#!/bin/bash
set -eu

# Build script for ClusterFuzzLite.
# It must compile each harness in fuzz/ into $OUT.

echo "Building temene_fuzzer..."

# Use $CXX and $CXXFLAGS provided by the build environment, and link with $LIB_FUZZING_ENGINE.
mkdir -p "$OUT"

$CXX $CXXFLAGS -std=c++11 fuzz/temene_fuzzer.cc -o "$OUT/temene_fuzzer" $LIB_FUZZING_ENGINE || {
  echo "Build failed" >&2
  exit 1
}

echo "Built $OUT/temene_fuzzer"
#!/bin/bash -eu
# ClusterFuzzLite build script for Temene / SeNest
#
# This script is executed inside the ClusterFuzzLite Docker container.
# It compiles all Jazzer.js fuzzing harnesses and packages them as fuzz targets.
#
# Environment variables provided by ClusterFuzzLite:
#   $SRC      — source root (where this repo lives)
#   $OUT      — output directory for compiled fuzz targets
#   $WORK     — scratch directory
#
# Reference: https://google.github.io/clusterfuzzlite/build-integration/

set -euo pipefail

REPO="$SRC/temene"
FUZZ_DIR="$REPO/fuzz"
HARNESSES_DIR="$FUZZ_DIR/harnesses"
CORPUS_DIR="$FUZZ_DIR/corpus"
DICTS_DIR="$FUZZ_DIR/dicts"

echo "[build.sh] Building Temene / SeNest fuzzing harnesses"
echo "[build.sh] SRC=$SRC  OUT=$OUT  WORK=$WORK"

# ---------------------------------------------------------------------------
# Step 1: Compile TypeScript harnesses via ts-node / tsc
# ---------------------------------------------------------------------------
echo "[build.sh] Compiling TypeScript harnesses..."
cd "$FUZZ_DIR"
npm run --silent build 2>/dev/null || \
    npx tsc --project tsconfig.json --outDir "$WORK/harnesses-dist" || true
# Note: Jazzer.js can run harnesses directly from TypeScript source via ts-node,
# so a compilation failure here is non-fatal; we fall back to source-direct mode.

# ---------------------------------------------------------------------------
# Step 2: Package each harness as a named fuzz target
# ---------------------------------------------------------------------------
# Jazzer.js fuzz-target format:
#   $OUT/<target_name>  — a shell script that runs Jazzer.js with the harness
#   $OUT/<target_name>_seed_corpus.zip  — seed corpus for the target
#   $OUT/<target_name>.dict             — mutation dictionary

HARNESSES=(
    "01-jwt-verify"
    "02-query-transform"
    "03-file-extension"
    "04-dto-validation"
    "05-objectid-parse"
    "06-bcrypt-hashpwd"
    "07-login-logic"
)

for harness in "${HARNESSES[@]}"; do
    target_name="temene_${harness//-/_}"
    harness_file="$HARNESSES_DIR/${harness}.fuzz.ts"

    echo "[build.sh] Packaging harness: $target_name"

    # --- Runner script ---
    cat > "$OUT/$target_name" <<RUNNER
#!/bin/bash
# Jazzer.js harness runner for $target_name
exec node --experimental-vm-modules \
    "$REPO/fuzz/node_modules/.bin/jazzer" \
    "$harness_file" \
    --corpus "\$1" \
    "\${@:2}"
RUNNER
    chmod +x "$OUT/$target_name"

    # --- Seed corpus zip ---
    corpus_subdir="$CORPUS_DIR/${harness}"
    if [ -d "$corpus_subdir" ]; then
        zip -j "$OUT/${target_name}_seed_corpus.zip" "$corpus_subdir"/* 2>/dev/null || true
        echo "[build.sh]   Seed corpus: $(ls "$corpus_subdir" | wc -l) files"
    else
        echo "[build.sh]   No seed corpus for $harness"
    fi

    # --- Dictionary ---
    # Map harness to its best-fit dictionary
    dict_file=""
    case "$harness" in
        "01-jwt-verify")      dict_file="$DICTS_DIR/jwt.dict" ;;
        "02-query-transform") dict_file="$DICTS_DIR/mongodb.dict" ;;
        "03-file-extension")  dict_file="$DICTS_DIR/filenames.dict" ;;
        "04-dto-validation")  dict_file="$DICTS_DIR/general.dict" ;;
        "05-objectid-parse")  dict_file="$DICTS_DIR/general.dict" ;;
        "06-bcrypt-hashpwd")  dict_file="$DICTS_DIR/general.dict" ;;
        "07-login-logic")     dict_file="$DICTS_DIR/general.dict" ;;
    esac

    if [ -n "$dict_file" ] && [ -f "$dict_file" ]; then
        cp "$dict_file" "$OUT/${target_name}.dict"
        echo "[build.sh]   Dictionary: $dict_file"
    fi
done

echo "[build.sh] Build complete. Targets in $OUT:"
ls -la "$OUT"/temene_* 2>/dev/null || echo "  (no targets found — check for errors above)"
