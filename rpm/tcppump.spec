Name: tcppump
Version: x.y.z
Release: 1%{?dist}
Summary: A simple ethernet packet generator
License: GPL-3.0-only
URL: https://github.com/amartin755/tcppump
Source: %{name}_%{version}.tar.gz
BuildRequires: cmake
BuildRequires: libpcap-devel
Requires: libpcap

%description
tcppump is a simple ethernet packet generator that allows users to create and send custom packets over the network.

%prep
%setup -q

%build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DWITH_UNITTESTS=OFF
cmake --build build --verbose

%install
mkdir -p %{buildroot}/usr/bin
install -m 0755 build/tcppump %{buildroot}/usr/bin/

%post
if [ -x /usr/sbin/setcap ]; then
    /usr/sbin/setcap cap_net_raw+eip /usr/bin/tcppump || :
fi

%files
/usr/bin/tcppump

