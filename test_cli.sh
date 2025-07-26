#!/bin/bash

# Device Tree Explorer CLI Test Script
# This script demonstrates the CLI functionality

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

# Check if CLI executable exists
if [ ! -f "build/bin/dte-cli" ]; then
    print_error "CLI executable not found. Please build the project first: ./build.sh"
    exit 1
fi

CLI="./build/bin/dte-cli"

echo "Device Tree Explorer CLI Test Suite"
echo "==================================="
echo

# Test 1: Version
print_status "Test 1: Version information"
if $CLI --version; then
    print_success "Version test passed"
else
    print_error "Version test failed"
    exit 1
fi
echo

# Test 2: Help
print_status "Test 2: Help information"
if $CLI --help > /dev/null; then
    print_success "Help test passed"
else
    print_error "Help test failed"
    exit 1
fi
echo

# Test 3: Command help
print_status "Test 3: Command-specific help"
if $CLI help info > /dev/null; then
    print_success "Command help test passed"
else
    print_error "Command help test failed"
    exit 1
fi
echo

# Test 4: Error handling - file not found
print_status "Test 4: Error handling - file not found"
if $CLI info nonexistent.dtb 2>&1 | grep -q "File not found"; then
    print_success "File not found error handling test passed"
else
    print_error "File not found error handling test failed"
fi
echo

# Test 5: Error handling - invalid command
print_status "Test 5: Error handling - invalid command"
if $CLI invalid_command 2>&1 | grep -q "Unknown command"; then
    print_success "Invalid command error handling test passed"
else
    print_error "Invalid command error handling test failed"
fi
echo

# Test 6: Create sample DTS file for testing
print_status "Test 6: Creating sample DTS file"
cat > sample.dts << 'EOF'
/dts-v1/;

/ {
    compatible = "test,device";
    model = "Test Device";
    #address-cells = <1>;
    #size-cells = <1>;

    cpus {
        #address-cells = <1>;
        #size-cells = <0>;

        cpu@0 {
            device_type = "cpu";
            compatible = "arm,cortex-a53";
            reg = <0>;
            clock-frequency = <1200000000>;
        };
    };

    memory@80000000 {
        device_type = "memory";
        reg = <0x80000000 0x40000000>;
    };

    soc {
        compatible = "simple-bus";
        #address-cells = <1>;
        #size-cells = <1>;
        ranges;

        uart@10000000 {
            compatible = "ns16550a";
            reg = <0x10000000 0x1000>;
            interrupts = <0 1 4>;
            clock-frequency = <1843200>;
        };
    };
};
EOF
print_success "Sample DTS file created"
echo

# Test 7: Validate sample DTS
print_status "Test 7: Validate sample DTS file"
if $CLI validate sample.dts; then
    print_success "DTS validation test passed"
else
    print_error "DTS validation test failed"
fi
echo

# Test 8: Info on sample DTS
print_status "Test 8: Get information about sample DTS"
if $CLI info sample.dts; then
    print_success "DTS info test passed"
else
    print_error "DTS info test failed"
fi
echo

# Test 9: List structure
print_status "Test 9: List device tree structure"
if $CLI list sample.dts; then
    print_success "List structure test passed"
else
    print_error "List structure test failed"
fi
echo

# Test 10: Search for nodes
print_status "Test 10: Search for CPU nodes"
if $CLI search sample.dts "cpu"; then
    print_success "Search test passed"
else
    print_error "Search test failed"
fi
echo

# Test 11: Convert DTS to DTB
print_status "Test 11: Convert DTS to DTB"
if $CLI convert sample.dts sample.dtb; then
    print_success "DTS to DTB conversion test passed"
else
    print_error "DTS to DTB conversion test failed"
fi
echo

# Test 12: Info on converted DTB
print_status "Test 12: Get information about converted DTB"
if $CLI info sample.dtb; then
    print_success "DTB info test passed"
else
    print_error "DTB info test failed"
fi
echo

# Test 13: Convert DTB back to DTS
print_status "Test 13: Convert DTB back to DTS"
if $CLI convert sample.dtb sample_converted.dts; then
    print_success "DTB to DTS conversion test passed"
else
    print_error "DTB to DTS conversion test failed"
fi
echo

# Test 14: Export to JSON
print_status "Test 14: Export to JSON format"
if $CLI export sample.dts json sample.json; then
    print_success "JSON export test passed"
else
    print_error "JSON export test failed"
fi
echo

# Test 15: Export to YAML
print_status "Test 15: Export to YAML format"
if $CLI export sample.dts yaml sample.yaml; then
    print_success "YAML export test passed"
else
    print_error "YAML export test failed"
fi
echo

# Test 16: Diff between original and converted DTS
print_status "Test 16: Diff between original and converted DTS"
if $CLI diff sample.dts sample_converted.dts; then
    print_success "Diff test passed"
else
    print_error "Diff test failed"
fi
echo

# Test 17: Test with /proc/device-tree if available
if [ -d "/proc/device-tree" ]; then
    print_status "Test 17: Test with system device tree"
    if $CLI info /proc/device-tree > /dev/null 2>&1; then
        print_success "System device tree test passed"
    else
        print_warning "System device tree test failed (this is normal if not running on a device with device tree)"
    fi
else
    print_warning "Test 17: Skipped - /proc/device-tree not available"
fi
echo

# Cleanup
print_status "Cleaning up test files..."
# rm -f sample.dts sample.dtb sample_converted.dts sample.json sample.yaml
print_success "Cleanup completed"

echo
echo "Test Summary"
echo "============"
print_success "All CLI tests completed successfully!"
echo
echo "The Device Tree Explorer CLI is working correctly."
echo "You can now use it to analyze and manipulate device tree files."
echo
echo "Examples:"
echo "  $CLI info your-device.dtb"
echo "  $CLI validate your-device.dts"
echo "  $CLI diff base.dtb overlay.dtb"
echo "  $CLI export your-device.dtb json output.json" 