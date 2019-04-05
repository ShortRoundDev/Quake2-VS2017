s/^\(libxml1\)_OBJECTS.*=/& \1-rc.o/
s/^\(testHTML\)_OBJECTS.*=/& \1-rc.o/
s/^\(testSAX\)_OBJECTS.*=/& \1-rc.o/
s/^\(testXPath\)_OBJECTS.*=/& \1-rc.o/
s/^\(tester\)_OBJECTS.*=/& \1-rc.o/
/^.c.o:/i\
# Rule to make compiled resource (Windows)\
%-rc.o: %.rc\
	windres -i $< -o $@
