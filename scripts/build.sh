#!/bin/bash

# EnviroNet Analyzer - C++ Implementation
# Build Script
# 
# This script builds the C++ version of EnviroNet Analyzer

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-"RelWithDebInfo"}
BUILD_DIR=${BUILD_DIR:-"build"}
INSTALL_DIR=${INSTALL_DIR:-"/usr/local"}
JOBS=${JOBS:-$(nproc)}

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    if ! command_exists pkg-config; then
        missing_deps+=("pkg-config")
    fi
    
    if ! pkg-config --exists libpcap; then
        missing_deps+=("libpcap-dev")
    fi
    
    if ! pkg-config --exists libnl-3.0; then
        missing_deps+=("libnl-3-dev")
    fi
    
    if ! pkg-config --exists libnl-genl-3.0; then
        missing_deps+=("libnl-genl-3-dev")
    fi
    
    if ! pkg-config --exists spdlog; then
        missing_deps+=("libspdlog-dev")
    fi
    
    if ! pkg-config --exists gtest; then
        missing_deps+=("libgtest-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_error "Please run: ./scripts/install_deps.sh"
        exit 1
    fi
    
    print_success "All dependencies are installed"
}

# Function to clean build directory
clean_build() {
    if [ "$1" = "--clean" ] || [ "$1" = "-c" ]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
}

# Function to create build directory
create_build_dir() {
    print_status "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
}

# Function to configure with CMake
configure_cmake() {
    print_status "Configuring with CMake..."
    cd "$BUILD_DIR"
    
    cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DENABLE_TESTING=ON \
        -DENABLE_COVERAGE=OFF
    
    if [ $? -eq 0 ]; then
        print_success "CMake configuration successful"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
    
    cd ..
}

# Function to build the project
build_project() {
    print_status "Building project with $JOBS jobs..."
    cd "$BUILD_DIR"
    
    make -j"$JOBS"
    
    if [ $? -eq 0 ]; then
        print_success "Build successful"
    else
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
}

# Function to run tests
run_tests() {
    if [ "$1" = "--test" ] || [ "$1" = "-t" ]; then
        print_status "Running tests..."
        cd "$BUILD_DIR"
        
        if [ -f "environet_tests" ]; then
            ./environet_tests
            if [ $? -eq 0 ]; then
                print_success "All tests passed"
            else
                print_warning "Some tests failed"
            fi
        else
            print_warning "Test executable not found, skipping tests"
        fi
        
        cd ..
    fi
}

# Function to install the project
install_project() {
    if [ "$1" = "--install" ] || [ "$1" = "-i" ]; then
        print_status "Installing project to $INSTALL_DIR..."
        cd "$BUILD_DIR"
        
        sudo make install
        
        if [ $? -eq 0 ]; then
            print_success "Installation successful"
            print_status "You can now run: environet --help"
        else
            print_error "Installation failed"
            exit 1
        fi
        
        cd ..
    fi
}

# Function to create package
create_package() {
    if [ "$1" = "--package" ] || [ "$1" = "-p" ]; then
        print_status "Creating package..."
        cd "$BUILD_DIR"
        
        if command_exists cpack; then
            cpack
            if [ $? -eq 0 ]; then
                print_success "Package created successfully"
                ls -la *.deb *.tar.gz 2>/dev/null || true
            else
                print_error "Package creation failed"
            fi
        else
            print_warning "cpack not found, skipping package creation"
        fi
        
        cd ..
    fi
}

# Function to show help
show_help() {
    echo "EnviroNet Analyzer - C++ Build Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --clean, -c        Clean build directory before building"
    echo "  --test, -t         Run tests after building"
    echo "  --install, -i      Install the project after building"
    echo "  --package, -p      Create package after building"
    echo "  --help, -h         Show this help message"
    echo ""
    echo "Environment variables:"
    echo "  BUILD_TYPE         Build type (Debug, Release, RelWithDebInfo) [default: RelWithDebInfo]"
    echo "  BUILD_DIR          Build directory [default: build]"
    echo "  INSTALL_DIR        Install directory [default: /usr/local]"
    echo "  JOBS               Number of parallel jobs [default: auto-detected]"
    echo ""
    echo "Examples:"
    echo "  $0                 # Build with default settings"
    echo "  $0 --clean         # Clean and build"
    echo "  $0 --test          # Build and run tests"
    echo "  $0 --install       # Build and install"
    echo "  BUILD_TYPE=Debug $0 --test  # Debug build with tests"
}

# Main script
main() {
    # Parse command line arguments
    local clean_build_flag=false
    local run_tests_flag=false
    local install_flag=false
    local package_flag=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean|-c)
                clean_build_flag=true
                shift
                ;;
            --test|-t)
                run_tests_flag=true
                shift
                ;;
            --install|-i)
                install_flag=true
                shift
                ;;
            --package|-p)
                package_flag=true
                shift
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    print_status "Starting build process..."
    print_status "Build type: $BUILD_TYPE"
    print_status "Build directory: $BUILD_DIR"
    print_status "Install directory: $INSTALL_DIR"
    print_status "Jobs: $JOBS"
    
    # Check dependencies
    check_dependencies
    
    # Clean build if requested
    if [ "$clean_build_flag" = true ]; then
        clean_build --clean
    fi
    
    # Create build directory
    create_build_dir
    
    # Configure with CMake
    configure_cmake
    
    # Build the project
    build_project
    
    # Run tests if requested
    if [ "$run_tests_flag" = true ]; then
        run_tests --test
    fi
    
    # Install if requested
    if [ "$install_flag" = true ]; then
        install_project --install
    fi
    
    # Create package if requested
    if [ "$package_flag" = true ]; then
        create_package --package
    fi
    
    print_success "Build process completed successfully!"
    print_status "Binary location: $BUILD_DIR/environet"
    
    if [ "$run_tests_flag" = false ]; then
        print_status "To run tests: $0 --test"
    fi
    
    if [ "$install_flag" = false ]; then
        print_status "To install: $0 --install"
    fi
}

# Run main function with all arguments
main "$@"
