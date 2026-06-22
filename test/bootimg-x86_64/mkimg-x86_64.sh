#!/bin/bash
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

	# Create original image.
	dd if=/dev/zero bs=256M count=1 > $img_path

	# Grant privileges.
	chmod 777 $img_path

	# Create partitions.
	parted $img_path << EOF
mktable gpt
mkpart EFI fat32 1MiB 63MiB
mkpart PbOS fat32 64MiB 160MiB
toggle 1 esp
EOF
else
	imgInited=1
fi

# Loop device path.
loop_dev=$(losetup --show -f ${img_path})

# Boot partition path.
boot_fs=$(losetup --show -f ${img_path} -o 1048576 --sizelimit `expr 1024 \* 1024 \* 63`)

# System partition path.
system_fs=$(losetup --show -f ${img_path} -o `expr 1024 \* 1024 \* 64` --sizelimit `expr 1024 \* 1024 \* 160`)

if [ ! "$imgInited" ]; then
	# Format partitions.
	mkfs.vfat "$boot_fs"
	mkfs.vfat "$system_fs"

	# Mount boot partition.
	mount "$boot_fs" /media
		# Limine Installation.
		mkdir "/media/EFI"
		mkdir "/media/EFI/BOOT"

		cp "$LIMINE_PATH/BOOTX64.EFI" "/media/EFI/BOOT"
		cp "$LIMINE_PATH/BOOTIA32.EFI" "/media/EFI/BOOT"
	# Unmount the partition.
	umount /media
fi

# Mount system partition.
mount "$system_fs" /media
	# Copy files.
	cp "test/bootimg-x86_64/config/limine.conf" "/media"

mkdir /media/sys
# mkdir /media/sys/boot
# mkdir /media/sys/kernel

# cp ./build/initcar /media/sys/boot
cp ./build/kernel/pboskrnl /media/sys/pboskrnl
cp ./build/initcar /media/sys/initcar

# Unmount the partition.
umount /media

# Delete loop devices.
losetup -d "$system_fs"
losetup -d "$boot_fs"
losetup -d "$loop_dev"
