CC := gcc
CFLAGS := -O3

PROGS := envsubst envsubst_go
all: $(PROGS)

$(PROGS): %: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(PROGS)

TESTS_DIR := tests
EXPECTED_DIR := $(TESTS_DIR)/expected
INPUT_DIR := $(TESTS_DIR)/input

TESTS := $(notdir $(wildcard $(INPUT_DIR)/*))

check: $(addprefix check-,$(PROGS))

check-%: % all
	@echo Checking $<...
	@set -e;$(foreach t,$(TESTS),printf $(t)...' '; diff -q <(TEST=a ./$< < $(INPUT_DIR)/$(t) 2>&1) <(cat $(EXPECTED_DIR)/$(t)); echo pass;)
	@echo
