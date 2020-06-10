
RUSSIAN AUTO LICPLATE RECOGNITION

Видеоролики для отладки были получены при помощи https://github.com/vmrepo/vsearch_licplates

Для разметки обучения детектора использовался https://github.com/vmrepo/markup


пример команды распознания изображений:
licplate.exe -rotateon -image pic1.jpg pic2.jpg -out out/pic1.jpg out/pic2.jpg

-out - необязательный
-rotateon - необязательный, включение анализа поворотов, по-умолчанию - отключён (сильно замедляет)

пример команды распознания видео:
licplate.exe -video movie.avi -showon -logfile licplate.log -framepath ./frames

необязательные ключи для видео:

-frameskip - количество пропускаемых кадров для распознаний, по умолчанию - 5
-buffersize - размер буфера для накопления кадров, по умолчанию - 300
-logfile - путь и имя файла для записи лога, по умолчанию - лог на экран
-framepath - путь для сохранения кадров с номерами, по умолчанию - текущая папка
-showon - включение окна видео, по умолчанию - не показывается
-kalmanoff - отключение фильтра Калмана для позиций рамок номеров, по умолчанию - включён



How build


Windows

Install OpenCV 4.1 - http://opencv.org/releases.html
After installing, must defined environment variable OPENCV_DIR
Open project or solution file in VS2015 and build solution.


Linux

Install OpenCV 3.0

***************************************************************************************
How to install Opencv 3.2 on Ubuntu 16.04

sudo apt-get update

sudo apt-get upgrade

sudo apt-get -y install libopencv-dev build-essential cmake git libgtk2.0-dev pkg-config python-dev python-numpy libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev libjasper-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev libtbb-dev libqt4-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils unzip

sudo apt-get install libtiff5-dev
sudo apt-get install libatlas-base-dev gfortran

mkdir opencv

cd opencv

wget https://github.com/Itseez/opencv/archive/3.2.0.zip -O opencv-3.2.0.zip

unzip opencv-3.2.0.zip

cd opencv-3.2.0

mkdir build

cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..

make -j $(nproc)

sudo make install

sudo /bin/bash -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'

sudo ldconfig
***************************************************************************************

Used Makefile, run make


***************************************************************************************
How to install Opencv 3.2 on Ubuntu 18.04

sudo apt-get update
sudo apt-get upgrade
sudo apt install build-essential cmake git pkg-config libgtk-3-dev
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev
sudo apt install libjpeg-dev libpng-dev libtiff-dev gfortran openexr libatlas-base-dev
sudo apt install python3-dev python3-numpy libtbb2 libtbb-dev libdc1394-22-dev
sudo apt-get install qt4-default

mkdir opencv

cd opencv

wget https://github.com/Itseez/opencv/archive/3.2.0.zip -O opencv-3.2.0.zip

unzip opencv-3.2.0.zip

cd opencv-3.2.0

mkdir build

cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..

make -j $(nproc)

sudo make install

sudo /bin/bash -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'

sudo ldconfig



***************************************************************************************
How as had been prepared tensorflow (for info)

Install TensorFlow for C
https://www.tensorflow.org/install/lang_c

official build
https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-gpu-windows-x86_64-1.12.0.zip
(cudnn mininmum 7.2.1)

all official builds
https://storage.googleapis.com/tensorflow

tensoflow.dll - for x64 only

possible find *tensorflow*.pyd for python and rename it to tensoflow.dll (gpu or cpu any version)

tensorflow.lib need for link applications
creation lib file from dll file
https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

commands for creattion lib file
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\dumpbin" /exports tensorflow.dll > f.txt
(edit content f.txt to tensorflow.def)
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\lib" /MACHINE:x64 /def:tensorflow.def /OUT:tensorflow.lib

samples:
https://stackoverflow.com/questions/44378764/hello-tensorflow-using-the-c-api
https://stackoverflow.com/questions/41688217/how-to-load-a-graph-with-tensorflow-so-and-c-api-h-in-c-language/41688506
https://github.com/Neargye/hello_tf_c_api
