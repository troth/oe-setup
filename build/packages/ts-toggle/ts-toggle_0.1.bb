DESCRIPTION = "User space program toggle LCD or DIO pins on TS-7800"
PR = "r0"

PRG_NAME = "ts-toggle"

SRC_URI = "\
    file://${PRG_NAME}.c \
    file://README.txt"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/${PRG_NAME}.c -o ${PRG_NAME}
}

do_install() {
    install -m 0755 -d ${D}${bindir} ${D}${docdir}/${PRG_NAME}
    install -m 0755 ${S}/${PRG_NAME} ${D}${bindir}
    install -m 0644 ${WORKDIR}/README.txt ${D}${docdir}/${PRG_NAME}
}

