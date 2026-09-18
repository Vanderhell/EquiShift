CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
BUILD := build
TESTS := test_transfer test_residual test_balanced test_overflow test_invariants
TEST_BINS := $(addprefix $(BUILD)/,$(TESTS))

.PHONY: all test clean sanitize benchmark example experimental-test experimental-benchmark
all: test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_transfer: tests/test_transfer.c src/transfer.c src/transfer_score.c include/constrained_probe/transfer.h include/constrained_probe/transfer_score.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_transfer.c src/transfer.c src/transfer_score.c
$(BUILD)/test_residual: tests/test_residual.c src/residual.c include/constrained_probe/residual.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_residual.c src/residual.c
$(BUILD)/test_balanced: tests/test_balanced.c src/balanced.c include/constrained_probe/balanced.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_balanced.c src/balanced.c
$(BUILD)/test_overflow: tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c src/balanced.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c src/balanced.c
$(BUILD)/test_invariants: tests/test_invariants.c src/balanced.c include/constrained_probe/balanced.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_invariants.c src/balanced.c -lm

test: $(TEST_BINS)
	@set -e; for t in $(TEST_BINS); do ./$$t; done
	@echo "all toolkit tests passed"

$(BUILD)/balanced_probe_benchmark: benchmarks/balanced_probe_benchmark.c src/balanced.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
benchmark: $(BUILD)/balanced_probe_benchmark
	./$(BUILD)/balanced_probe_benchmark

$(BUILD)/balanced_probe_models: examples/balanced_probe_models.c src/balanced.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
example: $(BUILD)/balanced_probe_models
	./$(BUILD)/balanced_probe_models

$(BUILD)/simplex_test: experiments/implicit_simplex/simplex_test.c experiments/implicit_simplex/simplex.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<
experimental-test: $(BUILD)/simplex_test
	./$(BUILD)/simplex_test

$(BUILD)/baseline_comparison: experiments/implicit_simplex/baseline_comparison.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $< -lm
$(BUILD)/use_case_benchmark: experiments/implicit_simplex/use_case_benchmark.c experiments/implicit_simplex/simplex.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<
experimental-benchmark: $(BUILD)/baseline_comparison $(BUILD)/use_case_benchmark
	./$(BUILD)/baseline_comparison
	./$(BUILD)/use_case_benchmark

sanitize: | $(BUILD)
	mkdir -p $(BUILD)/sanitize
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_transfer tests/test_transfer.c src/transfer.c src/transfer_score.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_residual tests/test_residual.c src/residual.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_balanced tests/test_balanced.c src/balanced.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_overflow tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c src/balanced.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_invariants tests/test_invariants.c src/balanced.c -lm
	@set -e; for t in $(addprefix $(BUILD)/sanitize/,$(TESTS)); do ./$$t; done

clean:
	rm -rf $(BUILD)
