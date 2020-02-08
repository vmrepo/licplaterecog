CC:=g++
CFLAGS:=-I/usr/local/include -I./tensorflow -L/usr/local/lib -L./tensorflow/cpu_1.5.0
LIBRARIES:=-lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_objdetect -lopencv_video -lopencv_videoio -ltensorflow

all: licplate

licplate:
	$(CC) $(CFLAGS) -o licplate licplate.cpp imageplate.cpp markupplate.cpp matplate.cpp rectplate.cpp videoplate.cpp $(LIBRARIES)

clean:
	rm -f *.o licplate