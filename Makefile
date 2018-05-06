
all: tetris

tetris: tetris.cpp
	g++ -o tetris tetris.cpp -lGL -lglut -lGLEW

clean:
	rm tetris

