# Input arguments for one-level
TEST_IDX=0


.PHONY: one

one:
	cd one-level; make

run_one:
	cd one-level; make run TEST_IDX=${TEST_IDX}

# example: build_os
# 	cd apps/example; make

# q2: build_os
# 	cd apps/q2; make

# q3: build_os
# 	cd apps/q3; make

# q4: build_os
# 	cd apps/q4; make

# q5: build_os
# 	cd apps/q5; make

# build_os:
# 	cd os; make

clean:
	cd one-level; make clean
# 	cd os; make clean
# 	cd apps/example; make clean
# 	cd apps/q2; make clean
# 	cd apps/q3; make clean
# 	cd apps/q4; make clean
# 	cd apps/q5; make clean

# run:
# 	cd ../lab2/bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj $(NUM_PC_PAIR); ee469_fixterminal

# run_q5:
# 	cd ../lab2/bin; dlxsim -x os.dlx.obj -a -u krypton.dlx.obj $(NUM_N) $(NUM_O) $(T); ee469_fixterminal