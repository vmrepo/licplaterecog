CC:=g++
CFLAGS:=-I/usr/local/include -I../tensorflow/include -L/usr/local/lib -L../tensorflow/lib
LIBRARIES:=-lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_objdetect -lopencv_video -lopencv_videoio -ltensorflow

all: licplate

licplate:
	$(CC) $(CFLAGS) -o licplate licplate.cpp imageplate.cpp videoplate.cpp recogplate.cpp detectplate.cpp affineplate.cpp cropplate.cpp optionsplate.cpp ocrplate.cpp $(LIBRARIES)

clean:
	rm -f *.o licplate