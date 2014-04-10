Name:		mediarepeater
Version:	0.1.1
Release:	1%{?dist}
Summary:	Repeat playback media files to learning language

Group:		Applications/Multimedia/Education
License:	GPLv3
URL:		https://github.com/liuyug/mediarepeater
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)


%description
Repeat playback media files to learning language


%prep
%setup -q


%build
cmake ./ -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%doc
%{_bindir}
%{_datadir}



%changelog

