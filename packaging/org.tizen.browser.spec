Name:       org.tizen.browser
Summary:    Tizen TV Open Browser
Version:    1.6.4
Release:    0
License:    Apache-2.0
Group:      Applications/Web
Source0:    %{name}-%{version}.tar.gz

%if "%{?_with_wayland}" == "1"
BuildRequires: pkgconfig(ecore-wayland)
%else
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(utilX)
%endif

BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(libssl)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eeze)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libtzplatform-config)
%if "%{?profile}" == "mobile"
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-system-settings)
%endif
BuildRequires:  browser-provider-devel
BuildRequires:  pkgconfig(efl-extension)

BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  edje-tools
BuildRequires:  boost-devel
BuildRequires:  boost-thread
#BuildRequires:  boost-date_time
BuildRequires:  boost-filesystem
BuildRequires:  boost-system

%if "%{?profile}" == "mobile"
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-content-media-content)
BuildRequires:  pkgconfig(shortcut)
%endif

%define BUILD_UT  %{?build_ut:ON}%{!?build_ut:OFF}
%if %BUILD_UT == "ON"
BuildRequires:  boost-test
%endif
%ifarch armv7l
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:	pkgconfig(dlog)
%endif

BuildRequires:  pkgconfig(libtzplatform-config)

%define _appdir %{TZ_SYS_RO_APP}/%{name}
%define _bindir %{_appdir}/bin
%define COVERAGE_STATS %{?coverage_stats:ON}%{!?coverage_stats:OFF}

%define _manifestdir %{TZ_SYS_RO_PACKAGES}
%define _icondir %{TZ_SYS_RO_ICONS}/default/small

%description
WebKit browser with EFL for Tizen TV Platform.

%prep
%setup -q

%build
%define _build_dir build-tizen
mkdir -p %{_build_dir}
cd %{_build_dir}

export CFLAGS="$(echo $CFLAGS | sed 's/-Wl,--as-needed//g')"
export CXXFLAGS="$(echo $CXXFLAGS | sed 's/-Wl,--as-needed//g')"
export FFLAGS="$(echo $FFLAGS | sed 's/-Wl,--as-needed//g')"

cmake .. \
    -DCMAKE_BUILD_TYPE=%{?build_type}%{!?build_type:RELEASE} \
    -DCMAKE_INSTALL_PREFIX=%{_appdir} \
    -DPACKAGE_NAME=%{name} \
    -DBINDIR=%{_bindir} \
    -DVERSION=%{version} \
    -DMANIFESTDIR=%{_manifestdir} \
    -DICONDIR=%{_icondir} \
    -DBUILD_UT=%{BUILD_UT} \
    -DCOVERAGE_STATS=%{COVERAGE_STATS} \
    -DPROFILE=%{profile} \
    -DTZ_SYS_RO_PACKAGES=%{TZ_SYS_RO_PACKAGES} \
    -DTZ_SYS_RO_ICONS=%{TZ_SYS_RO_ICONS} \
%if "%{?_with_wayland}" == "1"
    -DWAYLAND_SUPPORT=On
%else
    -DWAYLAND_SUPPORT=Off
%endif

make %{!?verbose_make}%{?verbose_make:VERBOSE=1} -j%{?jobs}%{!?jobs:1}

%install
cd %{_build_dir}
%make_install

%post

%files
%manifest org.tizen.browser.manifest
%{_icondir}/org.tizen.browser.png
%{_manifestdir}/%{name}.xml
%defattr(-,root,root,-)
%{_appdir}/bin/browser
%{_appdir}/res/edje/*/*.edj
%{_appdir}/services/*
%{_appdir}/lib/*
%{_appdir}/res/certs/*
%{_appdir}/res/locale/*/*/browser.mo

#-----------------------------------
%if %BUILD_UT == "ON"
%package ut
Summary:    BrowserAPP Unit Tests
#Requires:	org.tizen.browser

%description ut
BrowserAPP Unit Tests.

%files ut
%defattr(-,root,root,-)
%{_appdir}/bin/browser-ut

%endif
