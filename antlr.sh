#!/usr/bin/env bash
#
# antlr.sh - Generate MiniML lexer/parser with ANTLR4
#

set -e

ANTLR_VERSION=4.13.1
ANTLR_JAR="antlr-${ANTLR_VERSION}-complete.jar"
GRAMMAR="src/lexer_parser/MiniML.g4"
GEN_DIR="src/lexer_parser/gen"

# download jar if missing
if [ ! -f "$ANTLR_JAR" ]; then
  echo "⬇️  Downloading ANTLR $ANTLR_VERSION..."
  curl -O "https://www.antlr.org/download/${ANTLR_JAR}"
fi

# ensure output dir exists
mkdir -p "$GEN_DIR"

# run ANTLR
echo "⚙️  Generating C++ sources from $GRAMMAR..."
java -jar "$ANTLR_JAR" -Dlanguage=Cpp -visitor -no-listener \
  -o "$GEN_DIR" "$GRAMMAR"

echo "✅ Done. Generated files are in $GEN_DIR"
