# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   
Name:      mpc8xx
Version:   0.0.1
Release:   0.1
Copyright: GPL
Vendor:    Erwin Rol <erwin@muffin.org>
Url:       

Packager:  Erwin Rol <erwin@muffin.org>
Group:     
Source:    mpc8xx-0.0.1.tar.gz
BuildRoot: 

%description


%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                \s--build=i386-linux --host=i386-linux --target=i386-linux\s \
                $LOCALFLAGS
%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.mpc8xx
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.mpc8xx
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.mpc8xx

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/mpc8xx
rm -rf ../file.list.mpc8xx


%files -f ../file.list.mpc8xx
