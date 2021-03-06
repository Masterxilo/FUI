Name: 		foobillard
Version: 	3.0
Release: 	1

Group:          Amusements/Games       
Group(cs): 	Z�bava/Hry
Summary: 	A free OpenGL game of playing billard.
Summary(cs): 	Voln� �i�iteln� OpenGL simuluj�c� billiard.

Vendor:		Florian Berger (florian.berger@aec.at,harpin_floh@yahoo.de)
Packager:       Michal Ambroz (o_o) (rebus@seznam.cz)
License: 	GPL
URL: 		http://www.geocities.com/harpin_floh/foobillard_page.html
Source0: 	%{name}-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root
Prefix: /usr
Prefix: /etc

%description
FooBillard is an attempt to create a free OpenGL-billard for Linux. Why foo?
Well, actually I had this logo (F.B.-Florian Berger) and then foo sounds a
bit like pool (Somehow I wasn't quite attracted by the name FoolBillard)
Actually FooBillard is still under development but the main physics is
implemented. If you are a billard-pro and you're missing some physics,
please tell me. Cause I've implemented it like I think it should work,
which might differ from reality.


%prep
#Unpack package
%setup

%build
./configure --prefix=%{_prefix} --enable-glut
make 


%install
make DESTDIR=%{buildroot} install

#Install application link for X-Windows
mkdir -p %{buildroot}%{_mandir}/man6
install -c -m 644 foobillard.6 %{buildroot}%{_mandir}/man6/foobillard.6
install -d %{buildroot}/etc/X11/applnk/Games
echo -e "[Desktop Entry]
Name=FooBillard
Comment=OpenGL billard game
Exec=foobillard
Icon=foobillard.xpm
Terminal=0
Type=Application" > %{buildroot}/etc/X11/applnk/Games/"FooBillard".desktop




%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc AUTHORS COPYING INSTALL NEWS README ChangeLog TODO README.FONTS foobillardrc.example foobillard.6
#%doc doc/*
/etc/X11/applnk/*
%{_bindir}/*
%{_prefix}/share/foobillard/*
%{_mandir}/man6/foobillard.6*



#%changelog
#* Sun Jan 18 2003 Florian Berger <harpin_floh@yahoo.de>
#- added manpage

#* Thu May 16 2002 Michal Ambroz (O_O) <rebus@seznam.cz>
#- spring cleanup

#* Fri Mar 08 2002 Michal Ambroz (O_O) <rebus@seznam.cz>
#- initial foobillard specfile
