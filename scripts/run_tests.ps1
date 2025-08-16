# EnviroNet Analyzer - Comprehensive Test Runner (PowerShell)
# This script runs all tests with different configurations and provides detailed output

param(
    [string]$BuildDir = "build",
    [int]$TestTimeout = 300,
    [switch]$Verbose,
    [switch]$NoMemoryTests,
    [switch]$NoPerformanceTests,
    [switch]$StressTests,
    [switch]$Help
)

# Show help if requested
if ($Help) {
    Write-Host "Usage: .\run_tests.ps1 [options]" -ForegroundColor Cyan
    Write-Host "Options:" -ForegroundColor Cyan
    Write-Host "  -BuildDir DIR       Build directory (default: build)" -ForegroundColor White
    Write-Host "  -TestTimeout SEC    Test timeout in seconds (default: 300)" -ForegroundColor White
    Write-Host "  -Verbose            Enable verbose output" -ForegroundColor White
    Write-Host "  -NoMemoryTests      Skip memory tests" -ForegroundColor White
    Write-Host "  -NoPerformanceTests Skip performance tests" -ForegroundColor White
    Write-Host "  -StressTests        Enable stress tests" -ForegroundColor White
    Write-Host "  -Help               Show this help message" -ForegroundColor White
    exit 0
}

# Test results
$script:TotalTests = 0
$script:PassedTests = 0
$script:FailedTests = 0
$script:SkippedTests = 0

# Helper functions
function Write-Header {
    param([string]$Message)
    Write-Host "`n========================================" -ForegroundColor Blue
    Write-Host " $Message" -ForegroundColor Blue
    Write-Host "========================================" -ForegroundColor Blue
    Write-Host ""
}

function Write-Section {
    param([string]$Message)
    Write-Host "`n--- $Message ---" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "✅ $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "⚠️  $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "❌ $Message" -ForegroundColor Red
}

function Write-Info {
    param([string]$Message)
    Write-Host "ℹ️  $Message" -ForegroundColor Blue
}

# Check if build directory exists
function Test-BuildDirectory {
    if (-not (Test-Path $BuildDir)) {
        Write-Error "Build directory '$BuildDir' not found!"
        Write-Info "Please run '.\scripts\build.ps1' first"
        exit 1
    }
    
    if (-not (Test-Path "$BuildDir\environet_tests.exe")) {
        Write-Error "Test executable not found in '$BuildDir'!"
        Write-Info "Please run '.\scripts\build.ps1' first"
        exit 1
    }
}

# Run basic tests
function Run-BasicTests {
    Write-Section "Running Basic Tests"
    
    Push-Location $BuildDir
    
    try {
        Write-Info "Running all tests..."
        $result = & ".\environet_tests.exe" "--gtest_output=xml:test_results.xml"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "All basic tests passed!"
            $script:PassedTests++
        } else {
            Write-Error "Some basic tests failed!"
            $script:FailedTests++
        }
    }
    finally {
        Pop-Location
    }
}

# Run individual test suites
function Run-TestSuites {
    Write-Section "Running Individual Test Suites"
    
    $suites = @("Sensor", "Config", "Time", "Integration")
    
    Push-Location $BuildDir
    
    try {
        foreach ($suite in $suites) {
            Write-Info "Running ${suite} tests..."
            
            $result = & ".\environet_tests.exe" "--gtest_filter=*${suite}*" "--gtest_output=xml:${suite}_tests.xml"
            
            if ($LASTEXITCODE -eq 0) {
                Write-Success "${suite} tests passed!"
                $script:PassedTests++
            } else {
                Write-Error "${suite} tests failed!"
                $script:FailedTests++
            }
        }
    }
    finally {
        Pop-Location
    }
}

# Run mock mode tests
function Run-MockModeTests {
    Write-Section "Running Mock Mode Tests"
    
    Push-Location $BuildDir
    
    try {
        Write-Info "Testing mock sensor functionality..."
        $result = & ".\environet.exe" "--test-sensors"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Mock sensor tests passed!"
            $script:PassedTests++
        } else {
            Write-Error "Mock sensor tests failed!"
            $script:FailedTests++
        }
        
        Write-Info "Testing mock network functionality..."
        $result = & ".\environet.exe" "--test-network"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Mock network tests passed!"
            $script:PassedTests++
        } else {
            Write-Error "Mock network tests failed!"
            $script:FailedTests++
        }
    }
    finally {
        Pop-Location
    }
}

# Run performance tests
function Run-PerformanceTests {
    if ($NoPerformanceTests) {
        Write-Warning "Performance tests skipped (-NoPerformanceTests)"
        return
    }
    
    Write-Section "Running Performance Tests"
    
    Push-Location $BuildDir
    
    try {
        Write-Info "Running performance tests..."
        $result = & ".\environet_tests.exe" "--gtest_filter=*Performance*" "--gtest_repeat=3"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Performance tests passed!"
            $script:PassedTests++
        } else {
            Write-Error "Performance tests failed!"
            $script:FailedTests++
        }
    }
    finally {
        Pop-Location
    }
}

# Run stress tests
function Run-StressTests {
    if (-not $StressTests) {
        Write-Warning "Stress tests skipped (use -StressTests to enable)"
        return
    }
    
    Write-Section "Running Stress Tests"
    
    Push-Location $BuildDir
    
    try {
        Write-Info "Running stress tests (this may take a while)..."
        $result = & ".\environet_tests.exe" "--gtest_filter=*Stress*" "--gtest_repeat=10"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Stress tests passed!"
            $script:PassedTests++
        } else {
            Write-Error "Stress tests failed!"
            $script:FailedTests++
        }
    }
    finally {
        Pop-Location
    }
}

# Generate test report
function Generate-TestReport {
    Write-Section "Generating Test Report"
    
    $script:TotalTests = $script:PassedTests + $script:FailedTests + $script:SkippedTests
    
    Write-Host "`nTest Summary:" -ForegroundColor Blue
    Write-Host "  Total Test Suites: $($script:TotalTests)" -ForegroundColor White
    Write-Host "  Passed: $($script:PassedTests)" -ForegroundColor Green
    Write-Host "  Failed: $($script:FailedTests)" -ForegroundColor Red
    Write-Host "  Skipped: $($script:SkippedTests)" -ForegroundColor Yellow
    
    if ($script:FailedTests -eq 0) {
        Write-Host "`n🎉 All tests passed!" -ForegroundColor Green
        return 0
    } else {
        Write-Host "`n❌ Some tests failed!" -ForegroundColor Red
        return 1
    }
}

# Cleanup function
function Cleanup-TestArtifacts {
    Write-Info "Cleaning up test artifacts..."
    
    # Remove test output files
    if (Test-Path $BuildDir) {
        Push-Location $BuildDir
        
        try {
            Remove-Item -Path "test_results.xml" -ErrorAction SilentlyContinue
            Remove-Item -Path "*_tests.xml" -ErrorAction SilentlyContinue
            Remove-Item -Path "valgrind.log" -ErrorAction SilentlyContinue
        }
        finally {
            Pop-Location
        }
    }
}

# Main function
function Main {
    Write-Header "EnviroNet Analyzer - Comprehensive Test Suite"
    
    # Check prerequisites
    Test-BuildDirectory
    
    # Print configuration
    Write-Info "Build directory: $BuildDir"
    Write-Info "Test timeout: ${TestTimeout}s"
    Write-Info "Memory tests: $(-not $NoMemoryTests)"
    Write-Info "Performance tests: $(-not $NoPerformanceTests)"
    Write-Info "Stress tests: $StressTests"
    
    # Set up cleanup trap
    try {
        # Run tests
        Run-BasicTests
        Run-TestSuites
        Run-MockModeTests
        Run-PerformanceTests
        Run-StressTests
        
        # Generate report
        $exitCode = Generate-TestReport
        return $exitCode
    }
    finally {
        Cleanup-TestArtifacts
    }
}

# Run main function
try {
    $exitCode = Main
    exit $exitCode
}
catch {
    Write-Error "Test execution failed: $($_.Exception.Message)"
    exit 1
}
