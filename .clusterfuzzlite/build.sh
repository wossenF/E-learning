#!/bin/bash -eu
# ClusterFuzzLite build script for Temene / SeNest
#
# This script is executed inside the ClusterFuzzLite Docker container.
# It packages all Jazzer.js fuzzing harnesses as fuzz targets.

set -euo pipefail

REPO="$SRC"
FUZZ_DIR="$REPO/fuzz"
HARNESSES_DIR="$FUZZ_DIR/harnesses"
CORPUS_DIR="$FUZZ_DIR/corpus"
DICTS_DIR="$FUZZ_DIR/dicts"

echo "[build.sh] Building Temene / SeNest fuzzing harnesses"
echo "[build.sh] SRC=$SRC OUT=$OUT WORK=$WORK"

# Compile TypeScript harnesses if possible. Non-fatal because Jazzer.js can
# run TypeScript harnesses directly in source mode.
echo "[build.sh] Compiling TypeScript harnesses..."
cd "$FUZZ_DIR"
npm run --silent build 2>/dev/null || \
    npx tsc --project tsconfig.json --outDir "$WORK/harnesses-dist" || true

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

    cat > "$OUT/$target_name" <<RUNNER
#!/bin/bash
exec node --experimental-vm-modules \
    "$REPO/fuzz/node_modules/.bin/jazzer" \
    "$harness_file" \
    --corpus "\$1" \
    "\${@:2}"
RUNNER
    chmod +x "$OUT/$target_name"

    corpus_subdir="$CORPUS_DIR/${harness}"
    if [ -d "$corpus_subdir" ]; then
        zip -j "$OUT/${target_name}_seed_corpus.zip" "$corpus_subdir"/* 2>/dev/null || true
        echo "[build.sh]   Seed corpus: $(ls "$corpus_subdir" | wc -l) files"
    else
        echo "[build.sh]   No seed corpus for $harness"
    fi

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
ls -la "$OUT"/temene_* 2>/dev/null || echo "  (no targets found - check for errors above)"
