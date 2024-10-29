# Input arguments for two-level
TEST_IDX=0

.PHONY: default

default: build_os
	cd apps/example; make

build_os:
	cd os; make

clean:
	cd os; make clean
	cd apps/example; make clean

run:
	cd bin; dlxsim -x os.dlx.obj -a -D m -u makeprocs.dlx.obj $(TEST_IDX); ee469_fixterminal
