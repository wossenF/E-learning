# ClusterFuzzLite Dockerfile for Temene / SeNest
#
# Based on the official ClusterFuzzLite base image which ships:
#   - libFuzzer
#   - clang/clang++ with coverage instrumentation
#   - Jazzer.js prerequisites
#
# Reference: https://google.github.io/clusterfuzzlite/build-integration/

FROM gcr.io/oss-fuzz-base/base-builder-javascript

# ---------------------------------------------------------------------------
# Install Node.js 20 (LTS) — required by Jazzer.js 3.x
# ---------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        curl \
        ca-certificates \
        xz-utils && \
    curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
    apt-get install -y nodejs && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Copy repository
# ---------------------------------------------------------------------------
COPY . $SRC/temene

WORKDIR $SRC/temene

# ---------------------------------------------------------------------------
# Install fuzzing harness dependencies
# ---------------------------------------------------------------------------
RUN cd fuzz && npm install --prefer-offline

# ---------------------------------------------------------------------------
# Install SeNest backend dependencies (needed by harnesses that import
# from the NestJS source tree)
# ---------------------------------------------------------------------------
RUN cd SeNest && npm install --prefer-offline

# ---------------------------------------------------------------------------
# Build script
# ---------------------------------------------------------------------------
COPY .clusterfuzzlite/build.sh $SRC/build.sh
RUN chmod +x $SRC/build.sh
