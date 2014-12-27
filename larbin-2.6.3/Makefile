.PHONY : all crash clean distclean

BASEDIR=src
include src/larbin.make

dep:
	(cd src; $(MAKE) dep)

all debug prof:
	(cd adns; $(MAKE))
	(cd src; $(MAKE) $@)
	cp src/larbin .

clean: cleanhere
	(cd src; $(MAKE) clean)
	(cd adns; $(MAKE) clean)

distclean: cleanhere
	(cd src; $(MAKE) distclean)
	(cd adns; $(MAKE) distclean)
	$(RM) config.h config.make .depend
	$(RM) larbin

cleanhere:
	$(RM) *~ doc/*~
	$(RM) core gmon.out
	$(RM) fifo*
	$(RM) hashtable.* dupfile.*
