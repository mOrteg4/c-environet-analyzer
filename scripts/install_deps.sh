#!/bin/bash

# EnviroNet Analyzer - C++ Implementation
# Dependency Installation Script
# 
# This script installs all required dependencies for building and running
# the C++ version of EnviroNet Analyzer on Debian/Ubuntu/Raspberry Pi OS

set -e

echo "=========================================="
echo "EnviroNet Analyzer - C++ Dependencies"
echo "=========================================="

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "This script should not be run as root"
   echo "Please run as a regular user with sudo privileges"
   exit 1
fi

# Update package list
echo "Updating package list..."
sudo apt update

# Install build essentials and development tools
echo "Installing build essentials and development tools..."
sudo apt install -y \
    build-essential \
    cmake \
    g++ \
    clang-format \
    clang-tidy \
    git \
    pkg-config \
    valgrind \
    doxygen \
    graphviz

# Install networking libraries
echo "Installing networking libraries..."
sudo apt install -y \
    libpcap-dev \
    libnl-3-dev \
    libnl-genl-3-dev \
    iperf3 \
    iw \
    wireless-tools

# Install hardware interface libraries
echo "Installing hardware interface libraries..."
sudo apt install -y \
    libgpiod-dev \
    i2c-tools \
    libi2c-dev

# Install logging and testing libraries
echo "Installing logging and testing libraries..."
sudo apt install -y \
    libspdlog-dev \
    libgtest-dev \
    libgmock-dev

# Install additional utilities
echo "Installing additional utilities..."
sudo apt install -y \
    htop \
    iotop \
    iftop \
    tcpdump \
    wireshark-qt

# Create build directory
echo "Creating build directory..."
mkdir -p build

# Configure git hooks (optional)
if [ -d ".git" ]; then
    echo "Setting up git hooks..."
    if [ ! -f ".git/hooks/pre-commit" ]; then
        cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook to run clang-format
echo "Running clang-format..."
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
git add -u
EOF
        chmod +x .git/hooks/pre-commit
        echo "Git pre-commit hook installed"
    fi
fi

# Create clang-format configuration
echo "Creating clang-format configuration..."
cat > .clang-format << 'EOF'
---
Language: Cpp
BasedOnStyle: Google
AccessModifierOffset: -4
AlignAfterOpenBracket: Align
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AlignEscapedNewlines: Left
AlignOperands: true
AlignTrailingComments: true
AllowAllParametersOfDeclarationOnNextLine: true
AllowShortBlocksOnASingleLine: false
AllowShortCaseLabelsOnASingleLine: false
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None
AlwaysBreakBeforeMultilineStrings: true
AlwaysBreakTemplateDeclarations: Yes
BinPackArguments: true
BinPackParameters: true
BraceWrapping:
  AfterClass: false
  AfterControlStatement: false
  AfterEnum: false
  AfterFunction: false
  AfterNamespace: false
  AfterStruct: false
  AfterUnion: false
  BeforeCatch: false
  BeforeElse: false
  IndentBraces: false
BreakBeforeBinaryOperators: None
BreakBeforeBraces: Attach
BreakBeforeTernaryOperators: true
BreakStringLiterals: true
ColumnLimit: 100
CommentPragmas: '^ IWYU pragma:'
ConstructorInitializerAllOnOneLineOrOnePerLine: true
ConstructorInitializerIndentWidth: 4
ContinuationIndentWidth: 4
Cpp11BracedListStyle: true
DerivePointerAlignment: false
DisableFormat: false
ExperimentalAutoDetectBinPacking: false
FixNamespaceComments: true
IncludeBlocks: Preserve
IndentCaseLabels: true
IndentPPDirectives: None
IndentWidth: 4
IndentWrappedFunctionNames: false
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
NamespaceIndentation: None
PenaltyBreakAssignment: 2
PenaltyBreakBeforeFirstCallParameter: 1
PenaltyBreakComment: 300
PenaltyBreakFirstLessLess: 120
PenaltyBreakString: 1000
PenaltyExcessCharacter: 1000000
PenaltyReturnTypeOnItsOwnLine: 200
PointerAlignment: Left
ReflowComments: true
SortIncludes: true
SortUsingDeclarations: true
SpaceAfterCStyleCast: false
SpaceAfterTemplateKeyword: true
SpaceBeforeAssignmentOperators: true
SpaceInEmptyParentheses: false
SpacesInAngles: false
SpacesInContainerLiterals: false
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false
Standard: Cpp11
TabWidth: 4
UseTab: Never
EOF

# Create clang-tidy configuration
echo "Creating clang-tidy configuration..."
cat > .clang-tidy << 'EOF'
---
Checks: '*,
         -fuchsia-*,
         -google-*,
         -zircon-*,
         -abseil-*,
         -modernize-use-trailing-return-type,
         -llvm-*,
         -llvmlibc-*'
CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: lower_case
  - key: readability-identifier-naming.VariableCase
    value: lower_case
  - key: readability-identifier-naming.ParameterCase
    value: lower_case
  - key: readability-identifier-naming.ConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.MacroDefinitionCase
    value: UPPER_CASE
  - key: readability-identifier-naming.NamespaceCase
    value: lower_case
  - key: readability-identifier-naming.EnumCase
    value: CamelCase
  - key: readability-identifier-naming.EnumConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.StructCase
    value: CamelCase
  - key: readability-identifier-naming.UnionCase
    value: CamelCase
  - key: readability-identifier-naming.TypedefCase
    value: CamelCase
  - key: readability-identifier-naming.VariableCase
    value: lower_case
  - key: readability-identifier-naming.ParameterCase
    value: lower_case
  - key: readability-identifier-naming.ConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.MacroDefinitionCase
    value: UPPER_CASE
  - key: readability-identifier-naming.NamespaceCase
    value: lower_case
  - key: readability-identifier-naming.EnumCase
    value: CamelCase
  - key: readability-identifier-naming.EnumConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.StructCase
    value: CamelCase
  - key: readability-identifier-naming.UnionCase
    value: CamelCase
  - key: readability-identifier-naming.TypedefCase
    value: CamelCase
EOF

echo "=========================================="
echo "Dependencies installed successfully!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Run: mkdir -p build && cd build"
echo "2. Run: cmake .."
echo "3. Run: make -j$(nproc)"
echo "4. Run: ./environet --help"
echo ""
echo "Optional: Install additional tools for development:"
echo "- Visual Studio Code with C++ extensions"
echo "- Qt Creator (for GUI development)"
echo "- Valgrind (for memory debugging)"
echo "- GDB (for debugging)"
echo ""
echo "For hardware testing:"
echo "- Ensure I2C is enabled: sudo raspi-config"
echo "- Check I2C devices: sudo i2cdetect -y 1"
echo "- Install level shifter for 3.3V/5V conversion"
