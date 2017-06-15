Name: fingerterm
Version: 1.3.5
Release: 1
Summary: A terminal emulator with a custom virtual keyboard
Group: System/Base
License: GPLv2
Source0: %{name}-%{version}.tar.gz
URL: https://git.merproject.org/mer-core/fingerterm
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt0Feedback)
BuildRequires: pkgconfig(nemonotifications-qt5) >= 1.0.4
Requires: qt5-qtdeclarative-import-xmllistmodel
Requires: qt5-qtdeclarative-import-window2
Obsoletes: meego-terminal <= 0.2.2
Provides: meego-terminal > 0.2.2

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_datadir}/applications/*.desktop
%{_datadir}/%{name}/*.qml
%{_datadir}/%{name}/data/*
%{_datadir}/%{name}/icons/*

%prep
%setup -q -n %{name}-%{version}


%build
sed -i 's,/opt/fingerterm/,/usr/,' fingerterm.pro
qmake -qt=5 MEEGO_EDITION=nemo CONFIG+=enable-feedback CONFIG+=enable-nemonotifications
# Inject version number from RPM into source
sed -i -e 's/PROGRAM_VERSION="[^"]*"/PROGRAM_VERSION="%{version}"/g' version.h
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make INSTALL_ROOT=%{buildroot} install
