all:
	for dir in src examples; do (cd $$dir && $(MAKE)); done

docs:
	(cd doc && $(MAKE))

test:
	(cd tests && $(MAKE))

clean:
	for dir in doc src examples; \
	  do (cd $$dir && $(MAKE) clean); done

install:
	umask 022
	(cd src && $(MAKE) install)

distclean: clean
	rm -rf config.log config.status config.h autom4te.cache >/dev/null 2>&1
	find . -name Makefile -exec rm -f {} \;
	find . -name \*~      -exec rm -f {} \;
	find . -name \*.bak   -exec rm -f {} \;

indent:
	find src examples -name \*.c -exec indent -gnu {} \;
	find src examples -name \*.h -exec indent -gnu {} \;

release: all docs indent distclean
