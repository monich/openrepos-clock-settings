Name:       openrepos-clock-settings
Summary:    Alert settings
Version:    1.1.7
Release:    1
License:    BSD
URL:        https://github.com/monich/openrepos-clock-settings
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: qt5-qttools-linguist

%description
Settings for clock and calendar alarms.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%{_datadir}/%{name}
%{_libdir}/qt5/qml/openrepos/alert/settings
%{_datadir}/jolla-settings/entries/%{name}.json
%{_datadir}/translations/%{name}*.qm

%changelog
* Mon Mar 28 2022 Slava Monich <slava.monich@jolla.com> 1.1.7
- Added Slovak translation

* Sun Sep 26 2021 Slava Monich <slava.monich@jolla.com> 1.1.6
- Show version number in the pulley menu

* Sat Feb 27 2021 Slava Monich <slava.monich@jolla.com> 1.1.5
- Updated Polish translation

* Sun Feb 21 2021 Slava Monich <slava.monich@jolla.com> 1.1.4
- Updated Chinese and Russian translations

* Sat Feb 20 2021 Slava Monich <slava.monich@jolla.com> 1.1.3
- Added uninstall option

* Fri Jan 15 2021 Slava Monich <slava.monich@jolla.com> 1.1.2
- Added Portuguese translation

* Sat Jan 9 2021 Slava Monich <slava.monich@jolla.com> 1.1.1
- Updated and improved Swedish translation

* Sat Jan 9 2021 Slava Monich <slava.monich@jolla.com> 1.1.0
- Added calendar section
- Move the page from the Apps to System section

* Mon Nov 9 2020 Slava Monich <slava.monich@jolla.com> 1.0.8
- Chinese translations

* Mon Nov 9 2020 Slava Monich <slava.monich@jolla.com> 1.0.7
- Improved page initialization
- Require jolla-clock
- Finnish translations

* Thu Feb 1 2018 Slava Monich <slava.monich@jolla.com> 1.0.6
- Spanish translations

* Wed Jan 31 2018 Slava Monich <slava.monich@jolla.com> 1.0.5
- Swedish translations

* Tue Jan 30 2018 Slava Monich <slava.monich@jolla.com> 1.0.4
- Dutch translations
- Hungarian translations

* Tue Jan 30 2018 Slava Monich <slava.monich@jolla.com> 1.0.3
- French translations

* Mon Jan 29 2018 Slava Monich <slava.monich@jolla.com> 1.0.2
- Polish translations

* Mon Sep 19 2016 Slava Monich <slava.monich@jolla.com> 1.0.1
- Allow to configure alarm volume

* Wed Sep 14 2016 Slava Monich <slava.monich@jolla.com> 1.0.0
- Initial release
