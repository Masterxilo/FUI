SUBDIRS  = src data

RPMSPECS = foobillard.spec \
           foobillard-SDL.spec   



EXTRA_DIST = README.FONTS foobillardrc.example foobillard.6 $(RPMSPECS)



rpm: dist
	mkdir -p build
	PWD=`pwd` rpm --define "_sourcedir $$PWD" --define "_builddir $$PWD/build" \
	    --define "_rpmdir $$PWD/build" --define "_srcrpmdir $$PWD"  -bb foobillard.spec 
	PWD=`pwd` rpm --define "_sourcedir $$PWD" --define "_builddir $$PWD/build" \
	    --define "_rpmdir $$PWD/build" --define "_srcrpmdir $$PWD"  -bb foobillard-SDL.spec 
	mv `find build/ -name '*.rpm'` ./
	rm -Rf build
					    
srpm: dist     
	mkdir -p build
	PWD=`pwd` rpm --define "_sourcedir $$PWD" --define "_builddir $$PWD/build" \
	    --define "_rpmdir $$PWD/build" --define "_srcrpmdir $$PWD"  -ba foobillard.spec 
	PWD=`pwd` rpm --define "_sourcedir $$PWD" --define "_builddir $$PWD/build" \
	    --define "_rpmdir $$PWD/build" --define "_srcrpmdir $$PWD"  -ba foobillard-SDL.spec 
	mv `find build/ -name '*.rpm'` ./
	rm -Rf build
