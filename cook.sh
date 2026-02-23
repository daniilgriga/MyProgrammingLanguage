#!/usr/bin/env bash
# cook.sh — compile a .cook source file to a native x86-64 ELF binary
#
# Usage:
#   ./cook.sh <file.cook>          compile only
#   ./cook.sh <file.cook> --run    compile and run (reads from stdin)

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <file.cook> [--run]" >&2
    exit 1
fi

SOURCE="$1"
RUN=0

if [ "${2}" = "--run" ]; then
    RUN=1
fi

frontend/build/frontend  "$SOURCE"
middle_end/build/middle_end
backend/build/backend

if [ $RUN -eq 1 ]; then
    backend/build/program
fi
