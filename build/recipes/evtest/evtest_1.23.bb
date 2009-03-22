DESCRIPTION = "User space program for testing event device drivers."
PR = "r0"

PRG_NAME = "evtest"

SRC_URI = "file://${PRG_NAME}.c"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/${PRG_NAME}.c -o ${PRG_NAME}
}

do_install() {
    install -m 0755 -d ${D}${bindir}
    install -m 0755 ${S}/${PRG_NAME} ${D}${bindir}
}

