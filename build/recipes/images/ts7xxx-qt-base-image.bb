# This image is intended to be the base for further ts7xxx images

inherit image

export IMAGE_BASENAME = "ts7xxx-qt-base-image"

PR = "r1"

DISTRO_DEV_MANAGER = "mdev"
PREFERRED_PROVIDER_hotplug = "mdev"

# Include a timestamp that initscripts can use to set the time to a
# more sane value after a reboot
IMAGE_PREPROCESS_COMMAND = "create_etc_timestamp"

IMAGE_INSTALL = "task-boot \
                 util-linux-mount \
                 util-linux-umount \
                 e2fsprogs \
                 dosfstools \
                 dbus \
                 qt4-embedded \
                 ts-peekpoke \
                "

IMAGE_LINGUAS = ""
