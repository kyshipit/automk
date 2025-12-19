# AutoMLOS Test Suite Makefile
# Simple wrapper for root build system

# Include root Makefile configuration
include ../Makefile

# Test-specific convenience targets
test-local: $(LOG_TEST_TARGET) $(INTEGRATION_TEST_TARGET)
	@echo "=== Running AutoMLOS Test Suite ==="
	@echo "Running log tests..."
	$(LOG_TEST_TARGET)
	@echo "Running integration tests..."
	$(INTEGRATION_TEST_TARGET)

# Clean local test artifacts
clean-local:
	rm -f $(LOG_TEST_TARGET) $(INTEGRATION_TEST_TARGET)

# Override default test target
test: test-local

.PHONY: test-local clean-local test