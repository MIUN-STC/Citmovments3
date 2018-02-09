sudo apt-get update && sudo apt-get upgrade
sudo apt-get install libgles2-mesa-dev libgbm-dev libudev-dev libasound2-dev liblzma-dev git build-essential
sudo apt-get install gir1.2-ibus-1.0 libdbus-1-dev libegl1-mesa-dev libibus-1.0-5 libibus-1.0-dev libice-dev libsm-dev libsndio-dev libwayland-bin libwayland-dev libxi-dev libxinerama-dev libxkbcommon-dev libxrandr-dev libxss-dev libxt-dev libxv-dev x11proto-randr-dev x11proto-scrnsaver-dev x11proto-video-dev x11proto-xinerama-dev

cd ~
wget https://www.libsdl.org/release/SDL2-2.0.7.tar.gz
tar zxvf SDL2-2.0.7.tar.gz
cd SDL2-2.0.7
./configure --disable-video-rpi --enable-video-kmsdrm; make -j 4; sudo make install

cd ~
wget https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.2.tar.gz
tar zxvf SDL2_image-2.0.2.tar.gz
cd SDL2_image-2.0.2 && mkdir build && cd build
../configure
make -j 4
sudo make install

cd ~
wget https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz
tar zxvf SDL2_ttf-2.0.14.tar.gz
cd SDL2_ttf-2.0.14 && mkdir build && cd build
../configure
make -j $(nproc --all)
sudo make install
