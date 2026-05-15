#!/bin/sh
set -eu

SCRIPT_PATH=$(cd "${0%/*}" && pwd)
SCRIPT_NAME=${0##*/}

DEFAULT_SEL4_ROOT="$HOME/Code/sel4/tutorials/"
DEFAULT_BUILD_DIR="$SCRIPT_PATH/build"

SEL4_ROOT="${SEL4_ROOT:-$DEFAULT_SEL4_ROOT}"
BUILD_DIR="$DEFAULT_BUILD_DIR"

# Parse args
CMAKE_ARGS=""
while [ $# -gt 0 ]; do
  case "$1" in
  --sel4-root)
    shift
    [ $# -gt 0 ] || {
      echo "$SCRIPT_NAME: --sel4-root requires a path"
      exit 1
    }
    SEL4_ROOT="$1"
    ;;
  --sel4-root=*)
    SEL4_ROOT="${1#*=}"
    ;;
  --build-dir)
    shift
    [ $# -gt 0 ] || {
      echo "$SCRIPT_NAME: --build-dir requires a path"
      exit 1
    }
    BUILD_DIR="$1"
    ;;
  --build-dir=*)
    BUILD_DIR="${1#*=}"
    ;;
  *)
    CMAKE_ARGS="$CMAKE_ARGS \"$1\""
    ;;
  esac
  shift
done

echo "sel4 is at $SEL4_ROOT"

[ -d "$HOME/.sel4_cache" ] && CACHE_DIR="$HOME/.sel4_cache" || CACHE_DIR="$SCRIPT_PATH/.sel4_cache"

if [ -e "$SCRIPT_PATH/CMakeLists.txt" ]; then
  mkdir -p "$BUILD_DIR"

  # shellcheck disable=SC2086
  eval cmake \
    -S "$SCRIPT_PATH" \
    -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$SEL4_ROOT/kernel/gcc.cmake" \
    -DSEL4_CACHE_DIR="$CACHE_DIR" \
    -DSEL4_ROOT="$SEL4_ROOT" \
    -C "$SCRIPT_PATH/settings.cmake" \
    $CMAKE_ARGS
else
  echo "no CMakeLists.txt found"
fi
