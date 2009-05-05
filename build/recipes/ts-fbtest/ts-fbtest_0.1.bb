DESCRIPTION = "User space program to the frame buffer and touch screen on ts7390 systems."
PR = "r0"

PRG_NAME = "ts-fbtest"

SRC_URI = "\
    file://tstest.c \
    file://tagmem.c \
    file://tagmem.h \
    "

do_compile() {
    ${CC} ${CFLAGS} -I${WORKDIR} -c -o ${WORKDIR}/tagmem.o ${WORKDIR}/tagmem.c
    ${CC} ${CFLAGS} -I${WORKDIR} -c -o ${WORKDIR}/tstest.o ${WORKDIR}/tstest.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o ${PRG_NAME} ${WORKDIR}/tagmem.o ${WORKDIR}/tstest.o
}

do_install() {
    install -m 0755 -d ${D}${bindir} ${D}${docdir}/${PRG_NAME}
    install -m 0755 ${S}/${PRG_NAME} ${D}${bindir}
}

