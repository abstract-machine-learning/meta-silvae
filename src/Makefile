########################################################################
# Configuration
CC = gcc
CCOPT = -Wall -Wextra -pedantic -O2 -ansi -g
LDOPT = -lm
INSTALL_FOLDER = ../bin
DOXYFILE_PATH = ../doc/Doxyfile
DOC_PATH = ../doc/html/
TARGETS = meta-silvae tree-info tree-similarity forest-tools


########################################################################
# Dependencies
all: $(TARGETS)

meta-silvae: \
    data_structures/binary_heap.o \
    data_structures/list.o \
    data_structures/priority_queue.o \
    data_structures/set.o \
    data_structures/stack.o \
    data_structures/stopwatch.o \
    data/dataset.o \
    decision_tree/decision_tree.o \
    decision_tree/mapper/silva.o \
    decision_tree/performance/performance.o \
    decision_tree/training/genetic_algorithm.o \
    forest/forest.o \
    forest/mapper/silva.o \
    meta-silvae.o

tree-info: \
	data_structures/stack.o \
    data_structures/distribution.o \
	data/dataset.o \
    decision_tree/decision_tree.o \
    decision_tree/mapper/silva.o \
    tree_info.o

tree-similarity: \
    data_structures/stack.o \
    data/dataset.o \
    decision_tree/decision_tree.o \
    decision_tree/mapper/silva.o \
    tree_similarity.o

forest-tools: \
    data_structures/stack.o \
    data/dataset.o \
    decision_tree/decision_tree.o \
    decision_tree/mapper/silva.o \
    forest/forest.o \
    forest/mapper/silva.o \
    forest_tools.o

install: $(TARGETS)

.PHONY: clean, doc


########################################################################
# Receipes
%.o: %.c
	@echo "Compiling $@..."
	@$(CC) $(CCOPT) -c -o $@ $^

$(TARGETS):
	@echo "Linking $^ into $@..."
	@$(CC) $(CCOPT) -o $@ $^ $(LDOPT)

install:
	@echo "Moving into installation folder $(INSTALL_FOLDER)..."
	@mkdir -p $(INSTALL_FOLDER)
	@mv $(TARGETS) $(INSTALL_FOLDER)/

clean:
	@echo "Cleaning..."
	@find . -name "*.o" -type f -delete

doc:
	@echo "Generating documentation..."
	@rm -fR $(DOC_PATH)
	@doxygen $(DOXYFILE_PATH)
