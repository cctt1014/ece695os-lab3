# Input arguments for one-level
TEST_IDX=0


.PHONY: one two

one:
	cd one-level; make

two:
	cd two-level; make

run_one:
	cd one-level; make run TEST_IDX=${TEST_IDX}

run_two:
	cd two-level; make run TEST_IDX=${TEST_IDX}

clean:
	cd one-level; make clean
	cd two-level; make clean

# run:
# 	cd ../lab2/bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj $(NUM_PC_PAIR); ee469_fixterminal

# run_q5:
# 	cd ../lab2/bin; dlxsim -x os.dlx.obj -a -u krypton.dlx.obj $(NUM_N) $(NUM_O) $(T); ee469_fixterminal