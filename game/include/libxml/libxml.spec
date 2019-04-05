Name:		libxml
Summary:	libXML library
Version:	1.8.17
Release:	1
License:	LGPL
Group:		Development/Libraries
Source:         ftp://ftp.gnome.org/pub/GNOME/sources/%{name}/%{name}-%{version}.tar.gz
BuildRoot:	/var/tmp/%{name}-%{version}-root

URL:		http://rpmfind.net/veillard/XML/
Prereq:		/sbin/install-info

%description
This library allows you to manipulate XML files.

%package devel
Summary: Libraries, includes, etc to develop libxml applications
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Libraries, include files, etc you can use to develop libxml applications.

%prep
%setup -q

%build

%ifarch alpha
    MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

if [ ! -f configure ]; then
    CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $MYARCH_FLAGS \
	--prefix=%{_prefix} --libdir=%{_libdir} \
	--includedir=%{_includedir} --bindir=%{_bindir}
fi

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%{_prefix} \
    --libdir=%{_libdir} --includedir=%{_includedir} \
    --bindir=%{_bindir}

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} libdir=$RPM_BUILD_ROOT%{_libdir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    bindir=$RPM_BUILD_ROOT%{_bindir} install 

#
# hack to get libxml.so.0 too !
# Get rid of it once deps to libxml.so.0 have disapeared.
#
#if [ -f $RPM_BUILD_ROOT/%{prefix}/lib/libxml.so.1.8.17 ]
#then
#   (cd $RPM_BUILD_ROOT/%{prefix}/lib/ ; cp libxml.so.1.8.17 libxml.so.0.99.0 ; ln -sf libxml.so.0.99.0 libxml.so.0)
#fi
#
# another hack to get /usr/include/gnome-xml/libxml/
#
if [ -d $RPM_BUILD_ROOT/%{prefix}/include/gnome-xml ]
then
    (cd $RPM_BUILD_ROOT/%{prefix}/include/gnome-xml ; ln -sf . libxml)
fi

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-, root, root)

%doc AUTHORS ChangeLog NEWS README COPYING COPYING.LIB TODO
%{_libdir}/lib*.so.*

%files devel
%defattr(-, root, root)

%{_libdir}/lib*.so
%{_libdir}/*a
%{_libdir}/*.sh
%{_includedir}/*
%{_bindir}/*
%{_datadir}/gnome-xml/*
%{_libdir}/pkgconfig/libxml.pc


%changelog
* Sun Aug 05 2001 Jens Finke <jens@triq.net>
- Merged GPP spec file with spec.in file from CVS.
- Added {_datadir}/gnome-xml to devel package to include
documentation.
- Added libxml.pc file.

* Wed Jul 18 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- removed unncessary %defines 
- made %setup -quiet
- fixed buildroot
- fixed source line

* Thu Feb 22 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- fixed macros, removed hard-coded paths, that sort of thing.

* Thu Sep 23 1999 Daniel Veillard <Daniel.Veillard@w3.org>
- corrected the spec file alpha stuff
- switched to version 1.7.1
- Added validation, XPath, nanohttp, removed memory leaks
- Renamed CHAR to xmlChar

* Wed Jun  2 1999 Daniel Veillard <Daniel.Veillard@w3.org>
- Switched to version 1.1: SAX extensions, better entities support, lots of
  bug fixes.

* Sun Oct  4 1998 Daniel Veillard <Daniel.Veillard@w3.org>
- Added xml-config to the package

* Thu Sep 24 1998 Michael Fulbright <msf@redhat.com>
- Built release 0.30
