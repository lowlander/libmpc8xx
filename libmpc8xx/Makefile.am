lib_LTLIBRARIES = libmpc8xx.la

INCLUDES = $(all_includes)
AM_LDFLAGS =          

libmpc8xx_la_LDFLAGS = -version-info 0:3:0

libmpc8xx_la_METASOURCES = AUTO

libmpc8xx_la_SOURCES =	mpc8xxmem.c mpc8xxtarget.c mpc8xxspr.c \
			mpc8xxmisc.c mpc8xxflash.c mpc8xxbdm.c lptbdm.c 


EXTRA_DIST = 	lptbdm.c lptbdm.h mpc8xxbdm.c mpc8xxbdm.h mpc8xxflash.c \
		mpc8xxflash.h mpc8xxmisc.c mpc8xxmisc.h mpc8xxspr.c \
		mpc8xxspr.h mpc8xxtarget.c mpc8xxtarget.h mpc8xx.h \
		mpc8xxmem.h mpc8xxmem.c 

install-data-local:
	$(mkinstalldirs) $(includedir)/
	$(INSTALL_DATA) $(srcdir)/mpc8xx.h $(includedir)/mpc8xx.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/lptbdm.h $(includedir)/mpc8xx/lptbdm.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxbdm.h $(includedir)/mpc8xx/mpc8xxbdm.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxflash.h $(includedir)/mpc8xx/mpc8xxflash.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxmisc.h $(includedir)/mpc8xx/mpc8xxmisc.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxspr.h $(includedir)/mpc8xx/mpc8xxspr.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxtarget.h $(includedir)/mpc8xx/mpc8xxtarget.h
	$(mkinstalldirs) $(includedir)/mpc8xx/
	$(INSTALL_DATA) $(srcdir)/mpc8xxmem.h $(includedir)/mpc8xx/mpc8xxmem.h

uninstall-local:
	-rm -f $(includedir)/mpc8xx.h
	-rm -f $(includedir)/mpc8xx/lptbdm.h
	-rm -f $(includedir)/mpc8xx/mpc8xxbdm.h
	-rm -f $(includedir)/mpc8xx/mpc8xxflash.h
	-rm -f $(includedir)/mpc8xx/mpc8xxmisc.h
	-rm -f $(includedir)/mpc8xx/mpc8xxspr.h
	-rm -f $(includedir)/mpc8xx/mpc8xxtarget.h
	-rm -f $(includedir)/mpc8xx/mpc8xxmem.h
