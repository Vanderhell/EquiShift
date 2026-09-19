CC ?= cc
AR ?= ar
CLANG ?= clang
PYTHON ?= python3
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
BUILD := build
ifeq ($(OS),Windows_NT)
EXE := .exe
ifeq ($(MSYSTEM),)
MKDIR_BUILD := if not exist "$(BUILD)" mkdir "$(BUILD)"
MKDIR_SANITIZE := if not exist "$(BUILD)/sanitize" mkdir "$(BUILD)/sanitize"
MKDIR_TARGETS := if not exist "$(BUILD)/targets" mkdir "$(BUILD)/targets"
else
MKDIR_BUILD := mkdir -p $(BUILD)
MKDIR_SANITIZE := mkdir -p $(BUILD)/sanitize
MKDIR_TARGETS := mkdir -p $(BUILD)/targets
endif
else
EXE :=
MKDIR_BUILD := mkdir -p $(BUILD)
MKDIR_SANITIZE := mkdir -p $(BUILD)/sanitize
MKDIR_TARGETS := mkdir -p $(BUILD)/targets
endif
CPPFLAGS ?= -Iinclude -Isrc/internal
TESTS := test_transfer test_residual test_balanced test_overflow test_invariants
TEST_BINS := $(addprefix $(BUILD)/,$(addsuffix $(EXE),$(TESTS)))
LIB := $(BUILD)/libequishift.a

.PHONY: all library test sanitize clean cross-build \
	experimental-test experimental-benchmark \
	experimental-balanced-benchmark experimental-balanced-example \
	balanced-evaluation

all: library test
library: $(LIB)

$(BUILD):
	$(MKDIR_BUILD)

$(BUILD)/transfer.o: src/transfer.c include/constrained_probe/transfer.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(LIB): $(BUILD)/transfer.o
	$(AR) rcs $@ $^

$(BUILD)/test_transfer$(EXE): tests/test_transfer.c src/transfer_score.c src/internal/transfer_score.h $(LIB) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_transfer.c src/transfer_score.c $(LIB)
$(BUILD)/test_residual$(EXE): tests/test_residual.c src/residual.c src/internal/residual.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_residual.c src/residual.c
$(BUILD)/test_balanced$(EXE): tests/test_balanced.c src/balanced.c src/internal/balanced.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_balanced.c src/balanced.c
$(BUILD)/test_overflow$(EXE): tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c
$(BUILD)/test_invariants$(EXE): tests/test_invariants.c src/balanced.c src/internal/balanced.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/test_invariants.c src/balanced.c -lm

test: $(LIB) $(TEST_BINS)
	$(BUILD)/test_transfer$(EXE)
	$(BUILD)/test_residual$(EXE)
	$(BUILD)/test_balanced$(EXE)
	$(BUILD)/test_overflow$(EXE)
	$(BUILD)/test_invariants$(EXE)
	@echo "all toolkit tests passed"

sanitize: | $(BUILD)
	$(MKDIR_SANITIZE)
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_transfer$(EXE) tests/test_transfer.c src/transfer.c src/transfer_score.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_residual$(EXE) tests/test_residual.c src/residual.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_balanced$(EXE) tests/test_balanced.c src/balanced.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_overflow$(EXE) tests/test_overflow.c src/transfer.c src/transfer_score.c src/residual.c
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o $(BUILD)/sanitize/test_invariants$(EXE) tests/test_invariants.c src/balanced.c -lm
	$(BUILD)/sanitize/test_transfer$(EXE)
	$(BUILD)/sanitize/test_residual$(EXE)
	$(BUILD)/sanitize/test_balanced$(EXE)
	$(BUILD)/sanitize/test_overflow$(EXE)
	$(BUILD)/sanitize/test_invariants$(EXE)

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

$(BUILD)/balanced_probe_benchmark: experiments/balanced/balanced_probe_benchmark.c src/balanced.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
experimental-balanced-benchmark: $(BUILD)/balanced_probe_benchmark
	./$(BUILD)/balanced_probe_benchmark

$(BUILD)/plant_models: experiments/balanced/plant_models.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<
experimental-balanced-example: $(BUILD)/plant_models
	./$(BUILD)/plant_models

balanced-evaluation:
	$(PYTHON) experiments/balanced/decision_metrics.py

cross-build: | $(BUILD)
	$(MKDIR_TARGETS)
	$(CLANG) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Os -ffreestanding --target=arm-none-eabi -mcpu=cortex-m0 -mthumb -c src/transfer.c -o $(BUILD)/targets/transfer_m0.o
	$(CLANG) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Os -ffreestanding --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -c src/transfer.c -o $(BUILD)/targets/transfer_m4.o
	$(CLANG) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Os -ffreestanding --target=riscv32-unknown-elf -march=rv32imc -mabi=ilp32 -c src/transfer.c -o $(BUILD)/targets/transfer_rv32imc.o

clean:
	rm -rf $(BUILD)
