DESCRIPTION = "User space program to read/write directly to memory on ts7xxx systems."
PR = "r0"

PRG_NAME = "ts-peekpoke"

SRC_URI = "\
    file://${PRG_NAME}.c \
    "

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/${PRG_NAME}.c -o ${PRG_NAME}
}

do_install() {
    install -m 0755 -d ${D}${bindir} ${D}${docdir}/${PRG_NAME}
    install -m 0755 ${S}/${PRG_NAME} ${D}${bindir}
}

