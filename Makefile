CC = g++
LIBRARIES = -lglut -lGLU -lGL
OBJECTS = project.o triangulate.o


project: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBRARIES) -o project

project.o: project.cpp triangulate.h
	$(CC) -c project.cpp $(LIBRARIES)

triangulate.o: triangulate.cpp triangulate.h
	$(CC) -c triangulate.cpp

clean:
	-rm $(OBJECTS) 

clear:
	-rm $(OBJECTS) project
