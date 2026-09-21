# A simple image with a Ubuntu rootfs
#
# Note that we have a tight dependency to ubuntu-base
# and that we cannot just install arbitrary Yocto packages to avoid
# rootfs pollution or destruction.

IMAGE_LINGUAS = ""
IMAGE_INSTALL = ""
DEPENDS += "shadow-native"
inherit core-image image nativeaptinstall
export PACKAGE_INSTALL = "${IMAGE_INSTALL}"

# We must permit network access during rootfs creation for apt to work
do_rootfs[network] = "1"

APTGET_CHROOT_DIR = "${IMAGE_ROOTFS}"
# Run apt-get upgrade during do_rootfs so every package from the base
# tarball is refreshed to the latest available security/bug-fix
# version, matching what users get from a "fresh install + apt upgrade"
# on a real Ubuntu system.
APTGET_SKIP_UPGRADE = "0"

ROOTFS_POSTPROCESS_COMMAND:append = "do_fix_ldconfig; do_aptget_update; do_update_host; do_config_netplan;"

# Without the kernel and modules, we can't really use the Linux
IMAGE_INSTALL += "\
	kernel-devicetree \
	kernel-image \
	kernel-modules \
"

# We want to have an itb to boot from in the /boot directory to be flexible
# about U-Boot behavior
IMAGE_INSTALL += "\
   linux-kernelitb-norootfs-image \
"

#####
IMAGE_FEATURES += " \
    tools-sdk \
    debug-tweaks \
"
#######
APTGET_EXTRA_PACKAGES_SERVICES_DISABLED += "\
"

APTGET_EXTRA_PACKAGES += "\
	console-setup locales \
	apt vim \
	ethtool wget ftp iputils-ping \
	net-tools \
	nfs-common \
	openssh-server \
	libtool autoconf pkg-config \
	bluez \
	python-is-python3 \
	netplan.io \
	network-manager \
"

APTGET_EXTRA_SOURCE_PACKAGES += "\
"

# Add user with password user and default shell bash
USER_SHELL_BASH ?= "/bin/bash"
# Password: user
USER_PASSWD_USER ?= "zHipsIr46vtZk"
APTGET_ADD_USERS ?= "user:${USER_PASSWD_USER}:${USER_SHELL_BASH}"

HOST_NAME = "${MACHINE_ARCH}"

fakeroot do_update_host() {
	echo >"${APTGET_CHROOT_DIR}/etc/hostname" "${HOST_NAME}"

	echo  >"${APTGET_CHROOT_DIR}/etc/hosts" "127.0.0.1 localhost"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "127.0.1.1 ${HOST_NAME}"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" ""
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "# The following lines are desirable for IPv6 capable hosts"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "::1 ip6-localhost ip6-loopback"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "fe00::0 ip6-localnet"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "ff00::0 ip6-mcastprefix"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "ff02::1 ip6-allnodes"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "ff02::2 ip6-allrouters"
	echo >>"${APTGET_CHROOT_DIR}/etc/hosts" "ff02::3 ip6-allhosts"
}

fakeroot do_config_netplan() {
	echo >>"${APTGET_CHROOT_DIR}/etc/netplan/network-manage-all.yaml" "network:"
	echo >>"${APTGET_CHROOT_DIR}/etc/netplan/network-manage-all.yaml" "  version: 2"
	echo >>"${APTGET_CHROOT_DIR}/etc/netplan/network-manage-all.yaml" "  renderer: NetworkManager"
}

#We need to add Yocto libraries to LD path and remove conflicting libraries
fakeroot do_fix_ldconfig() {
	#Ld config mises /usr/lib path
	echo >>"${APTGET_CHROOT_DIR}/etc/ld.so.conf.d/01-yocto.conf" "/usr/lib"
}

IMAGE_ROOTFS_SIZE ?= "8192"
# 2GB of free space to root fs partition (at least 1.5 GB needed during the Bazel build)
IMAGE_ROOTFS_EXTRA_SPACE ?= "2000000"


# Add sja1110 driver for RDB boards
IMAGE_INSTALL:append:s32g274ardb2 = " sja1110"
IMAGE_INSTALL:append:s32g399ardb3 = " sja1110"

# optee
IMAGE_INSTALL:append:s32cc = "${@bb.utils.contains('DISTRO_FEATURES', 'optee', ' optee-client optee-examples optee-test ', '', d)}"

COMPATIBLE_MACHINE ="(.*ubuntu)"

python do_rootfs:prepend() {
    d.setVar('IMAGE_LOG_CHECK_EXCLUDES', 'Failed')
}
