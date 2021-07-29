Name:		pagecache
Version:	1.0
Release:	1%{?dist}
Summary:	module to analyze pagecache 

Group:		Kernel	
License:	GPL
URL:		http://192.168.157.40:8000/cgit/cgit.cgi/
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Requires:	systemtap

%define kversion 2.6.32-504.23.4.el6.centos.plus.x86_64
%description
a systemtap module to analyze pagecache
can't work standalone, use staprun -L to load it

%prep


%build


%install
mkdir -p %{buildroot}/lib/modules/%{kversion}/kernel/mm
mkdir -p %{buildroot}/etc/init.d/
install -m 744 pagecache.ko %{buildroot}/lib/modules/%{kversion}/kernel/mm
install -m 744 pagecache %{buildroot}/etc/init.d/

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%doc
/lib/modules/%{kversion}/kernel/mm/pagecache.ko
/etc/init.d/pagecache

%changelog
- started
