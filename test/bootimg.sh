cmake --build build --target initcar
sudo bash ./test/bootimg/mkimg.sh
bash ./test/bootimg/vm/bochs.sh
