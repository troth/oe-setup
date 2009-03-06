# Sample initramfs image, very minimal

#IMAGE_INSTALL = "\
#    initramfs-module-nfs \
#    busybox \
#    base-files \
#    sysvinit \
#    netbase \
#    ts7800-initramfs-init \
#    ts-keypad \
#    "

IMAGE_INSTALL = "\
    busybox \
    uclibc \
    ts-keypad \
    ts7800-initramfs-init \
    "

export IMAGE_BASENAME = "initramfs-custom-image"
IMAGE_LINGUAS = ""

# Install only ${IMAGE_INSTALL}, not even deps
PACKAGE_INSTALL_NO_DEPS = "1"

inherit image
