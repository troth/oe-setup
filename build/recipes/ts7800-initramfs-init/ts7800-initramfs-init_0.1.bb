DESCRIPTION = "Init scripts for a custom initramfs for TS7800 SBC."
LICENSE = "BSD"

PR = "r0"

SRC_URI = "\
    file://linuxrc-nfsboot \
"

FILES_${PN} = "/"

do_install() {
	install -m 0755 -d ${D}
	install -m 0755 ${WORKDIR}/linuxrc-nfsboot ${D}/linuxrc-nfsboot

	# linuxrc
	rm -f ${D}/linuxrc
	ln -s linuxrc-nfsboot ${D}/linuxrc
}

