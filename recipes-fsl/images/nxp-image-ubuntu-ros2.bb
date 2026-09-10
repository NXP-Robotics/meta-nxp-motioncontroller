# A more complex image with ROS2 elements
def get_rosdistro(d):
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '22.04':
        return 'humble'
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '24.04':
        return 'jazzy'
    return 'undefined'
ROS2_DISTRO = "${@get_rosdistro(d)}"

require nxp-image-ubuntu.bb

fakeroot do_aptget_user_update:append() {
 	echo "Add Universe repository for ROS2 install"
 	aptget_run_aptget install software-properties-common
 	chroot "${APTGET_CHROOT_DIR}" /usr/bin/add-apt-repository universe
	chroot "${APTGET_CHROOT_DIR}" ${APTGET_EXECUTABLE} ${APTGET_DEFAULT_OPTS} update
}

# ROS2 packages, to be installed after ROS2 setup
ROS2_PACKAGES += " ros-${ROS2_DISTRO}-desktop ros-dev-tools"

# Added ROS2 packages for Motion Control
ROS2_PACKAGES += " \
    ros-${ROS2_DISTRO}-moveit-* ros-${ROS2_DISTRO}-pinocchio \
"

# Added ROS2 packages for SLAM and Navigation
ROS2_PACKAGES += " \
    ros-${ROS2_DISTRO}-nav2-* ros-${ROS2_DISTRO}-cartographer-* ros-${ROS2_DISTRO}-rtabmap-* \
"

ADD_ROS_PACKAGES ?= "${ROS2_PACKAGES}"
APTGET_EXTRA_PACKAGES_LAST += " \
    ${ADD_ROS_PACKAGES} \
"

# 5 Gbytes of free space to root fs partition
IMAGE_ROOTFS_EXTRA_SPACE = "5000000"
