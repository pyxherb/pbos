# PbOS x86-64 HAL

Here is the x86-64 HAL of the PbOS.

## Known Issues

### Strange Interrupt When Scheduling in QEMU

Occasionally we found that the kernel triggers a suspicious interrupt during the scheduling of massive processes, fix it.
