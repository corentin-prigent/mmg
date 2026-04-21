# Makefile pour les tests EGADS
# Place ce fichier dans ~/Desktop/mmg/mon-mmg/src/
#
# Usage :
#   make          -> compile les 3 tests
#   make run      -> compile + execute les 3 tests
#   make clean    -> supprime les binaires

ESP     = /home/eelleb/ESP/EngSketchPad
INC     = -I$(ESP)/include
LIB     = -L$(ESP)/lib
RPATH   = -Wl,-rpath,$(ESP)/lib
LDFLAGS = $(LIB) -legads -lm $(RPATH)
CFLAGS  = -O2 -Wall

EGADS_FILE = $(ESP)/data/basic/demo0c.egads

TARGETS = test1_topo test2_evaluate test3_project

all: $(TARGETS)

test1_topo: test1_topo.c
	gcc $(CFLAGS) $(INC) $< -o $@ $(LDFLAGS)

test2_evaluate: test2_evaluate.c
	gcc $(CFLAGS) $(INC) $< -o $@ $(LDFLAGS)

test3_project: test3_project.c
	gcc $(CFLAGS) $(INC) $< -o $@ $(LDFLAGS)

run: all
	@echo "============ test1_topo ============"
	./test1_topo $(EGADS_FILE)
	@echo ""
	@echo "============ test2_evaluate ============"
	./test2_evaluate $(EGADS_FILE)
	@echo ""
	@echo "============ test3_project ============"
	./test3_project $(EGADS_FILE)

clean:
	rm -f $(TARGETS)
