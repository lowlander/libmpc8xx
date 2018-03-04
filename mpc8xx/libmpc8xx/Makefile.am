####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libmpc8xx.la


INCLUDES = $(all_includes)


LDFLAGS =     


libmpc8xx_la_LDFLAGS = -version-info 0:1:0

libmpc8xx_la_METASOURCES = AUTO

libmpc8xx_la_SOURCES = mpc8xxmem.c mpc8xxtarget.c mpc8xxspr.c mpc8xxmisc.c mpc8xxflash.c mpc8xxbdm.c lptbdm.c 


EXTRA_DIST = lptbdm.c lptbdm.h mpc8xxbdm.c mpc8xxbdm.h mpc8xxflash.c mpc8xxflash.h mpc8xxmisc.c mpc8xxmisc.h mpc8xxspr.c mpc8xxspr.h mpc8xxtarget.c mpc8xxtarget.h mpc8xx.h mpc8xxmem.h mpc8xxmem.c 

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/lptbdm.h $(DESTDIR)$(includedir)/mpc8xx/lptbdm.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxbdm.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxbdm.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxflash.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxflash.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxmisc.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxmisc.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxspr.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxspr.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxtarget.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxtarget.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/
	$(INSTALL_DATA) $(srcdir)/mpc8xx.h $(DESTDIR)$(includedir)/mpc8xx.h
	$(mkinstalldirs) $(DESTDIR)$(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxmem.h $(DESTDIR)$(includedir)/mpc8xx/mpc8xxmem.h

uninstall-local:
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/lptbdm.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxbdm.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxflash.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxmisc.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxspr.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxtarget.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx.h
	-rm -f $(DESTDIR)$(includedir)/mpc8xx/mpc8xxmem.h

####### kdevelop will overwrite this part!!! (end)############
