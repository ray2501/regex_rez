%{!?directory:%define directory /usr}

%define buildroot %{_tmppath}/%{name}-%{version}

Name:          regex_rez
Summary:       Tcl bindings for RE2
Version:       0.5
Release:       1
License:       MIT
Group:         Development/Libraries/Tcl
Source:        %name-%version.tar.gz
URL:           https://github.com/ray2501/tclregex
BuildRequires: autoconf
BuildRequires: make
BuildRequires: gcc-c++
BuildRequires: libicu-devel
BuildRequires: re2-devel
BuildRequires: libstdc++-devel
BuildRequires: tcl-devel >= 8.5
Requires:      tcl >= 8.5
BuildRoot:     %{buildroot}

%description
RE2 is a fast, safe, thread-friendly alternative to backtracking regular
expression engines like those used in PCRE, Perl, and Python.
It is a C++ library.

This package is Tcl bindings for RE2.

%prep
%setup -q -n %{name}-%{version}

%build
export CC=g++
./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make 

%install
make DESTDIR=%{buildroot} pkglibdir=%{tcl_archdir}/%{name}%{version} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%{tcl_archdir}

