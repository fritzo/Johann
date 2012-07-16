
current_target: run_johann

kernel: FORCE
	$(MAKE) -C pomagma install
	$(MAKE) -C src kernel
server: FORCE
	$(MAKE) -C src server
jmapper: FORCE
	$(MAKE) -C mapper jmapper
doc: FORCE
	$(MAKE) -C scripts/doc all
pdf: FORCE
	$(MAKE) -C scripts pdf
html: pdf FORCE
	$(MAKE) -C html all
	cp scripts/main.pdf html/fritz-thesis-draft.pdf
	chmod 644 html/fritz-thesis-draft.pdf

all: kernel server jmapper html pdf

#-----------------------------------------------------------------------------
# Testing

test: kernel
	echo '' > log/test.log
	$(MAKE) -C pomagma test
	$(MAKE) -C src test
	@#bin/johann -l test.log scripts/test/main.jcode
	(bin/johann -l test.log < scripts/test/validate.jcode && \
	echo 'Kernel Test Passed') || (echo 'Kernel Test Failed' && false)
	@echo '----( All Tests Passed )----'

#-----------------------------------------------------------------------------
# Running johann

run_johann: kernel
	echo '' > log/default.log
	bin/johann
time: kernel
	bash -c "time bin/johann scripts/test/build.jcode"
profile: kernel
	echo '' > log/test.log
	gcov src/lambda_theories.h
	gcov src/measures.C
	gcov src/complexity.C
	gcov src/axiom_enforcement.C
	gcov src/combinatory_structure.C
	bin/johann -l test.log scripts/test/profile.jcode
	gprof -I src -l -b bin/johann | ./profile_subs > johann.prof
	gvim johann.prof &
memtest: kernel
	echo '' > log/test.log
	valgrind --leak-check=yes bin/johann -l test.log scripts/test/memtest.jcode
cachetest: kernel
	echo '' > log/tests.log
	valgrind --tool=cachegrind bin/johann -l test.log scripts/test/cachetest.jcode

#-----------------------------------------------------------------------------
# Building cores

default: kernel
	echo '' > log/default.log
	bin/johann scripts/build/default.jcode -l default.log
skr: kernel
	echo '' > log/skr.log
	bin/johann scripts/build/skr.jcode -l skr.log
skj: kernel
	echo '' > log/skj.log
	bin/johann scripts/build/skj.jcode -l skj.log
skrj: kernel
	echo '' > log/skrj.log
	bin/johann scripts/build/skrj.jcode -l skrj.log
skjo: kernel
	echo '' > log/skjo.log
	bin/johann scripts/build/skjo.jcode -l skjo.log
literate: kernel
	echo '' > log/literate.log
	bin/johann scripts/build/literate.jcode -l literate.log
godels_T: kernel
	echo '' > log/godels_T.log
	bin/johann scripts/build/godels_T.jcode -l godels_T.log

#-----------------------------------------------------------------------------
# Utilities

clean:
	rm -f core core.* vgcore.* gmon.out *.prof temp.diff
	$(MAKE) -C src clean
	$(MAKE) -C pomagma clean
	$(MAKE) -C mapper clean
	$(MAKE) -C scripts clean
	$(MAKE) -C html clean
	$(MAKE) -C data clean

cleaner: clean
	$(MAKE) -C data cleaner
	$(MAKE) -C stats clean
	$(MAKE) -C log clean

cleanest: cleaner
	rm -rf tool_config/example.subversion/auth

FORCE: 
