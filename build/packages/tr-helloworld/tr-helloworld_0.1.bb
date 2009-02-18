DESCRIPTION = "My hello world program."
PR = "r0"

SRC_URI = "\
    file://tr-helloworld.c \
    file://README.txt"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/tr-helloworld.c -o tr-helloworld
}

do_install() {
    install -m 0755 -d ${D}${bindir} ${D}${docdir}/tr-helloworld
    install -m 0755 ${S}/tr-helloworld ${D}${bindir}
    install -m 0644 ${WORKDIR}/README.txt ${D}${docdir}/tr-helloworld
}

