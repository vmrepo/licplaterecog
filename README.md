
Программа распознавания автономеров.

Видеоролики для отладки были получены при помощи https://github.com/vmrepo/vsearch_licplates

Для разметки обучения детектора использовался https://github.com/vmrepo/markup

Пример команды распознавания изображений:
licplate.exe -image pic1.jpg pic2.jpg -out out/pic1.jpg out/pic2.jpg

-out - необязательный

Пример команды распознавания видео:
licplate.exe -video movie.avi -showon -logfile licplate.log -framepath ./frames

Необязательные ключи для видео:

-frameskip - количество пропускаемых кадров для распознаний, по умолчанию - 5
-buffersize - размер буфера для накопления кадров, по умолчанию - 300
-logfile - путь и имя файла для записи лога, по умолчанию - лог на экран
-framepath - путь для сохранения кадров с номерами, по умолчанию - текущая папка
-scorethreshold - порого вероятности детектирования номеров, по умолчанию - 0.8
-showon - включение окна видео, по умолчанию - не показывается
-kalmanoff - отключение фильтра Калмана для позиций рамок номеров, по умолчанию - включён

How build

TensorFlow 2.3 library for C (Windows, Linux) downloaded from https://www.tensorflow.org/install/lang_c

OpenCV for Windows

Install OpenCV 4.1 - http://opencv.org/releases.html
After installing, must defined environment variable OPENCV_DIR
Open project or solution file in VS2015 and build solution.

OpenCV for Linux

Install OpenCV 4.1 (likely 3.2)

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
