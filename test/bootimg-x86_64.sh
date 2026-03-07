cmake --build build --target pbkern
sudo -E ./test/bootimg-x86_64/mkimg-x86_64.sh
bash ./test/bootimg-x86_64/vm/qemu-x86_64.sh
