qemu-system-x86_64 \
-m 512 \
-s \
-S \
-no-reboot \
-no-shutdown \
-drive file=./build/boot.raw,format=raw \
-drive if=pflash,format=raw,file="./build/ovmf-code-x86_64.fd",readonly=on \
-monitor stdio \
-debugcon vc:640x480
