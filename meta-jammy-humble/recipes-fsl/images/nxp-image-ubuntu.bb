# A simple image with a Ubuntu rootfs
#
# Note that we have a tight dependency to ubuntu-base
# and that we cannot just install arbitrary Yocto packages to avoid
# rootfs pollution or destruction.
PV = "${@d.getVar('PREFERRED_VERSION_ubuntu-base', True) or '1.0'}"

REQUIRED_DISTRO_FEATURES = ""

require nxp-image-common.inc

APTGET_EXTRA_PACKAGES_MAIN = "1"

APTGET_EXTRA_PACKAGES += "\
	libvirt-daemon-system \
"

# This must be added first as it provides the foundation for
# subsequent modifications to the rootfs
IMAGE_INSTALL += "\
	ubuntu-base \
	ubuntu-base-dev \
	ubuntu-base-dbg \
	ubuntu-base-doc \
"

IMAGE_INSTALL:append = " \
	iperf2 \
"

IMAGE_INSTALL:append = " \
	canfestival \
	libmodbus \
	modbus-simulator \
"

# Installing Java is a bit of loaded topic because it is version
# dependent. We default to the Java version based on the Ubuntu version
JAVAVERSION = '${@ \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "22.04", "11", \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "24.04", "21", \
   "unknownjavaversion" \
    , d) \
    , d)}'
JAVALIBPATHSUFFIX = '${@ \
    oe.utils.conditional("JAVAVERSION", "8", "jre/lib/${TRANSLATED_TARGET_ARCH}/jli", \
    oe.utils.conditional("JAVAVERSION", "11", "lib/jli", \
    oe.utils.conditional("JAVAVERSION", "21", "lib/jli", \
   "unsupportedjavaversion" \
    , d) \
    , d) \
    , d)}'
APTGET_EXTRA_PACKAGES += " \
    openjdk-${JAVAVERSION}-jre \
"
# Instruct QEMU to append (inject) the path to the jdk library to LD_LIBRARY_PATH
# (required by openjdk-${JAVAVERSION}-jdk)
APTGET_EXTRA_LIBRARY_PATH += "/usr/lib/jvm/java-${JAVAVERSION}-openjdk-${UBUNTU_TARGET_ARCH}/${JAVALIBPATHSUFFIX}"

IMAGE_ROOTFS_SIZE ?= "6291456"
