# This image is intended to be the base for further ts7xxx images

inherit image

export IMAGE_BASENAME = "ts7xxx-qt-base-image"

PR = "r1"

# Include a timestamp that initscripts can use to set the time to a
# more sane value after a reboot
IMAGE_PREPROCESS_COMMAND = "create_etc_timestamp"

IMAGE_INSTALL = "task-boot \
                 util-linux-mount \
                 util-linux-umount \
                 e2fsprogs \
                 dosfstools \
                 dbus \
                 strace \
                 evtest \
                 tslib \
                 tslib-dbg \
                 tslib-calibrate \
                 tslib-tests \
                 qt4-embedded \
                 qt4-embedded-dbg \
                 qt4-embedded-plugin-mousedriver-tslib \
                 qt4-embedded-plugin-mousedriver-tslib-dbg \
                 ts-peekpoke \
                 ts-fbtest \
                 gdb \
                 nfs-utils \
                "

IMAGE_LINGUAS = ""
