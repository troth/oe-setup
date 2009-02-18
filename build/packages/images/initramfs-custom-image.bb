# Sample initramfs image, very minimal

IMAGE_INSTALL = "\
    initramfs-module-nfs \
    busybox \
    base-files \
    sysvinit \
    netbase \
    "

export IMAGE_BASENAME = "initramfs-custom-image"
IMAGE_LINGUAS = ""

# Install only ${IMAGE_INSTALL}, not even deps
PACKAGE_INSTALL_NO_DEPS = "0"

inherit image
