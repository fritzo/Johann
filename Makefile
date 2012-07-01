#Johann's makefile

current_target: run_johann

kernel: FORCE
	$(MAKE) -C cpp kernel
server: FORCE
	$(MAKE) -C cpp server
jmapper: FORCE
	$(MAKE) -C cpp/mapper jmapper
doc: FORCE
	$(MAKE) -C scripts/doc all
pdf: FORCE
	$(MAKE) -C scripts pdf
html: pdf FORCE
	$(MAKE) -C doxygen all
	$(MAKE) -C html all
	cp scripts/main.pdf html/fritz-thesis-draft.pdf
	chmod 644 html/fritz-thesis-draft.pdf

all: kernel server jmapper html pdf

#=======[ running johann ]=====================================================

run_johann: kernel
	echo "" > log/default.log
	bin/johann
test: kernel
	echo "" > log/test.log
	bin/johann -l test.log scripts/test/main.jcode
time: kernel
	bash -c "time bin/johann scripts/test/build.jcode"
profile: kernel
	echo "" > log/test.log
	gcov cpp/lambda_theories.h
	gcov cpp/measures.C
	gcov cpp/complexity.C
	gcov cpp/axiom_enforcement.C
	gcov cpp/combinatory_structure.C
	bin/johann -l test.log scripts/test/profile.jcode
	gprof -I cpp -l -b bin/johann | ./profile_subs > johann.prof
	gvim johann.prof &
memtest: kernel
	echo "" > log/test.log
	valgrind --leak-check=yes bin/johann -l test.log scripts/test/memtest.jcode
cachetest: kernel
	echo "" > log/tests.log
	valgrind --tool=cachegrind bin/johann -l test.log scripts/test/cachetest.jcode

#=======[ building cores ]=====================================================

default: kernel
	echo "" > log/build.log
	bin/johann scripts/build/default.jcode -l default.log
skr: kernel
	echo "" > log/build.log
	bin/johann scripts/build/skr.jcode -l skr.log
skj: kernel
	echo "" > log/build.log
	bin/johann scripts/build/skj.jcode -l skj.log
skrj: kernel
	echo "" > log/build.log
	bin/johann scripts/build/skrj.jcode -l skrj.log
skjo: kernel
	echo "" > log/build.log
	bin/johann scripts/build/skjo.jcode -l skjo.log
literate: kernel
	echo "" > log/build.log
	bin/johann scripts/build/literate.jcode -l literate.log
godels_T: kernel
	echo "" > log/build.log
	bin/johann scripts/build/godels_T.jcode -l godels_T.log

#=======[ utilities ]==========================================================

unit_test:
	echo "" > log/test.log
	$(MAKE) -C cpp test

clean:
	rm -f core core.* vgcore.* gmon.out *.prof temp.diff
	$(MAKE) -C cpp clean
	$(MAKE) -C scripts clean
	$(MAKE) -C data clean
	$(MAKE) -C html clean
	$(MAKE) -C doxygen clean

cleaner: clean
	$(MAKE) -C data cleaner
	$(MAKE) -C stats clean
	$(MAKE) -C log clean

cleanest: cleaner
	rm -rf tool_config/example.subversion/auth

FORCE: 

