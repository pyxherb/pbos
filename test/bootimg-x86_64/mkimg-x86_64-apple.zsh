function mount_loop_dev() {
	loop_dev_path=$(hdiutil attach -nomount -imagekey diskimage-class=CRawDiskImage $1 | head -1 | awk '{ print $1; }')
	loop_dev_name=$(echo $loop_dev_name | awk '{ split($1, a, "/"); print a[3]; }')
}

img_path="$PWD/build/boot.raw"

if [ ! -f "$img_path" ]; then
	if [ -z "$LIMINE_PATH" ]; then
		echo "LIMINE_PATH does not present"
		exit 1
	fi

	if [ ! -d "$LIMINE_PATH" ]; then
		echo "LIMINE_PATH \"$LIMINE_PATH\" does not exist"
		exit 1
	fi

	qemu-img create --format raw $img_path 256M

	chmod 777 $img_path

	mount_loop_dev $img_path

	sudo diskutil partitionDisk $loop_dev_name 2 GPT %C12A7328-F81F-11D2-BA4B-00A0C93EC93B% EFI 64M FAT32 PbOS 0
else
	mount_loop_dev $img_path
fi

diskutil unmount "$loop_dev_path"s2

sudo diskutil eraseVolume ms-dos EFI /dev/disk8s1
diskutil unmount "$loop_dev_path"s1

mkdir -p ./build/boot-efi
sudo mount -t msdos "$loop_dev_path"s1 ./build/boot-efi
	# Limine Installation.
	mkdir -p "./build/boot-efi/EFI"
	mkdir "./build/boot-efi/EFI/BOOT"

	cp "$LIMINE_PATH/BOOTX64.EFI" "./build/boot-efi/EFI/BOOT"
	cp "$LIMINE_PATH/BOOTIA32.EFI" "./build/boot-efi/EFI/BOOT"
diskutil unmount "$loop_dev_path"s1

hdiutil detach $loop_dev_path
