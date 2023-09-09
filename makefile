CC = cc

LDLIBS = -framework GLUT -framework OpenGL

all:
	$(CC) solarscape.c $(LDLIBS) -o solarscape 
