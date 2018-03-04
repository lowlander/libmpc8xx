####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = mpc8xx

## INCLUDES were found outside kdevelop specific part

mpc8xx_SOURCES = main.c 
mpc8xx_LDADD   = ./libmpc8xx/libmpc8xx.la 

SUBDIRS = libmpc8xx 

EXTRA_DIST = main.c 

####### kdevelop will overwrite this part!!! (end)############
# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
mpc8xx_LDFLAGS = $(all_libraries) 

CFLAGS := -I./libmpc8xx/
                                                                                
