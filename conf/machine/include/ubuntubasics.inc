# We currently support recipies for:
# - 22.04.5
UBUNTU_TARGET_VERSION ?= "22.04.5"

UBUNTU_TARGET_MAJORVERSION ?= "${@d.getVar('UBUNTU_TARGET_VERSION', True).rsplit('.')[0]}"
UBUNTU_TARGET_MINORVERSION ?= "${@d.getVar('UBUNTU_TARGET_VERSION', True).rsplit('.')[1]}"
UBUNTU_TARGET_SUBVERSION ?= "${@d.getVar('UBUNTU_TARGET_VERSION', True).rsplit('.')[2]}"
UBUNTU_TARGET_BASEVERSION ?= "${UBUNTU_TARGET_MAJORVERSION}.${UBUNTU_TARGET_MINORVERSION}"
PREFERRED_VERSION_ubuntu-base = "${UBUNTU_TARGET_VERSION}"

# I am sure there are smarter ways to map the architecture. This works
# initially. FIX
UBUNTU_TARGET_ARCH="${@d.getVar('TRANSLATED_TARGET_ARCH', True).replace("aarch64", "arm64")}"

# Downconfigure Yocto environment to not pull in local things
# We do not want any default post intercepts or special default
# packages added.
IMAGE_LINGUAS = ""
POKY_DEFAULT_EXTRA_RDEPENDS:remove = "packagegroup-core-boot"
INITRAMFS_IMAGE = ""
POSTINST_INTERCEPTS_DIR = "<--InVaLiD-->"

# Ubuntu does not use /var/volatile
VOLATILE_LOG_DIR = "no"

# Removal of flash image filesystem support
# for Ubuntu images due to an issue regarding 'cpio'
# utility when seeking on large files (> 2 GB).
# As a Ubuntu based rootfs can easily go beyond 2GB and are even in
# the smallest form larger than what is good for a boot flash, we
# do not generate a .flashimage with Ubuntu.
IMAGE_FSTYPES:remove = "flashimage"
IMAGE_TYPES_MASKED += "cpio cpio.gz cpio.gz.u-boot"



GCCVERSION = '${@ \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "20.04", "9.3.0", \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "22.04", "11.2.0", \
    "unsupportedubuntuversion" \
    , d) \
    , d)}'

# The Yocto glibc version must not exceed the Ubuntu glibc version
# or Yocto-created executables will fail on the Ubuntu image.
# We want to be as close as possible to the respective Ubuntu glibc
# version to avoid compatibility issues.
# The glibc version for a given Ubuntu release is a-priori knowledge,
# so the suitable Yocto version can be selected here.
# Note that the glibc is not compiled in Ubuntu identical fashion.
# It remains a glibc that works for Yocto, which is good enough for
# our purposes.
# Ubuntu 20.x: max glibc 2.31...
# Ubuntu 22.x: max glibc 2.35...
GLIBCVERSION = '${@ \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "20.04", "2.31%", \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "22.04", "2.35", \
    "unsupportedubuntuversion" \
    , d) \
    , d)}'
PREFERRED_VERSION_cross-localedef-native = "${GLIBCVERSION}"
BINUVERSION="2.38"

# More recent Ubuntu versions work with symlinks for the base
# directories. We can't refer to symlink based files
DISTRO_FEATURES:append = '${@ \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "20.04", " usrmerge", \
    oe.utils.conditional("UBUNTU_TARGET_BASEVERSION", "22.04", " usrmerge", \
   "unsupportedubuntuversion" \
    , d) \
    , d)}'

PREFERRED_PROVIDER_bison-native = "bison"

# Suppress harmless do_rootfs log_check matches caused by installing
# certain packages inside the pseudo+qemu chroot:
#
#   - NetworkManager postinst calls nmcli to reload connections via
#     D-Bus, but the NM daemon isn't running in the chroot. Noble's
#     NM 1.46 prints
#         "Error: Could not create NMClient object: ..."
#         "WARNING: NetworkManager could not reload connections ..."
#     which trips Yocto's rpm-rootfs error/warning regexes. On jammy
#     (NM 1.36) the same failure logs softer text that doesn't match.
#
#   - logger(1) prints
#         "logger: send message failed: Operation not permitted"
#     because syslogd isn't running in the chroot during install.
#
# All of these are post-install warnings about chroot-only limitations,
# not real failures: on the booted target each service starts normally.
#
# IMAGE_LOG_CHECK_EXCLUDES values are space-separated regexes; we use
# \s+ for whitespace since a literal space would be treated as a
# token separator.
IMAGE_LOG_CHECK_EXCLUDES = "Could\s+not\s+create\s+NMClient \
                            NetworkManager\s+could\s+not\s+reload \
                            logger:\s+send\s+message\s+failed"

# This is to be able to pull in the prebuilt Ubuntu rootfs
MACHINEOVERRIDES =. "ubuntu:"

# When creating images, we would like to preserve any extended
# attributes that are part of our rootfs
IMAGE_CMD_TAR = "tar --xattrs --xattrs-include=*"
do_image_tar[depends] += "tar-replacement-native:do_populate_sysroot"
EXTRANATIVEPATH += "tar-native"

# We influence the setup to match Ubuntu more properly as otherwise
# Yocto behavior may conflict with the Ubuntu rootfs.
INIT_MANAGER = "systemd"

# Turns out that we want the Yocto recipes as default provider, but
# then we must ensure they do not get deployed to avoid pollution of
# the target rootfs.
# Rationale is that Yocto builds need to be internally consistent,
# and we need the sysroots info for Yocto recipes, but the Ubuntu rootfs
# provides things, too. We can't use that for building though, so Yocto
# packages conflicting must not reach the package manager.
# Whenever a Yocto package is installed, all dependencies which are
# to be provided by Ubuntu must be added to the list. Check
# recipe-depends.dot from bitbake -g.
# The respective Ubuntu image must then install the Ubuntu version of
# the recipe.
require conf/machine/include/nodistropackage.inc
DISTRO_NEVER_WRITE_PACKAGE:class-target += "\
    alsa-conf \
    alsa-lib \
    update-alternatives \
    bash \
    base-files \
    bc \
    bison \
    bluez5 \
    bzip2 \
    ca-certificates \
    coreutils \
    db \
    diffutils \
    dpkg \
    dtc \
    flex \
    glibc \
    glibc-locale \
    gcc-runtime \
    gdbm \
    initscripts \
    libc6 \
    libc6-utils \
    libffi \
    libgcc \
    liblzma \
    libpam \
    libsqlite3 \
    libxml2 \
    m4 \
    ncurses \
    netbase \
    opkg-utils \
    perl \
    python3 \
    qemuwrapper-cross \
    shared-mime-info \
    sqlite3 \
    readline \
    util-linux \
    xz \
    zlib \
"

# We have some packages which we do not want to have in a target
# image because they are available as Ubuntu-native packages.
# Given that we can't set a PREFERRED_RPROVIDER in an image recipe
# we need to isolate those packages globally.
DISTRO_NEVER_WRITE_PACKAGE:class-target += "\
    openssl \
    openssl-bin \
    openssl-conf \
    openssl-dev \
"

# All the runtime above should come from ubuntu-base or a derived image
# but only for the target, not for native elements.
# The trick is that we need to retarget everything to one recipe because
# at configuration time we cannot predict algorithmically which derived
# images would add the final apt packages. Any "missing" dependencies
# would only show up then at image build time.
DISTRO_DEFAULT_RPROVIDER:class-target = "ubuntu-base (= ${UBUNTU_TARGET_VERSION})"
