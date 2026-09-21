# A more complex image with ROS2 elements
def get_rosdistro(d):
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '22.04':
        return 'humble'
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '24.04':
        return 'jazzy'
    return 'undefined'
ROS2_DISTRO = "${@get_rosdistro(d)}"

def get_ubuntu_name(d):
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '22.04':
        return 'jammy'
    if d.getVar('UBUNTU_TARGET_BASEVERSION', True) == '24.04':
        return 'noble'
    return 'noble'
UBUNTU_NAME = "${@get_ubuntu_name(d)}"

require nxp-image-ubuntu.bb

fakeroot do_aptget_user_update:append() {
 	echo "Add Universe repository for ROS2 install"
 	aptget_run_aptget install curl software-properties-common
 	chroot "${APTGET_CHROOT_DIR}" /usr/bin/add-apt-repository universe
	export ROS_APT_SOURCE_VERSION=$(chroot "${APTGET_CHROOT_DIR}" /usr/bin/curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F'"' '{print $4}')
	chroot "${APTGET_CHROOT_DIR}" /usr/bin/curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${UBUNTU_NAME}_all.deb"
	chroot "${APTGET_CHROOT_DIR}" /usr/bin/dpkg -i /tmp/ros2-apt-source.deb
	chroot "${APTGET_CHROOT_DIR}" ${APTGET_EXECUTABLE} ${APTGET_DEFAULT_OPTS} update
}

# ROS2 packages, to be installed after ROS2 setup
ROS2_PACKAGES += " ros-${ROS2_DISTRO}-desktop ros-dev-tools"

# Added ROS2 packages for Motion Control
ROS2_PACKAGES += " \
    ros-${ROS2_DISTRO}-moveit-chomp-*        \
    ros-${ROS2_DISTRO}-moveit-common         \
    ros-${ROS2_DISTRO}-moveit-configs-utils  \
    ros-${ROS2_DISTRO}-moveit-core-*         \
    ros-${ROS2_DISTRO}-moveit-hybrid-*       \
    ros-${ROS2_DISTRO}-moveit-kinematics-*   \
    ros-${ROS2_DISTRO}-moveit-msgs-*         \
    ros-${ROS2_DISTRO}-moveit-planners-*     \
    ros-${ROS2_DISTRO}-moveit-plugins        \
    ros-${ROS2_DISTRO}-moveit-py-*           \
    ros-${ROS2_DISTRO}-moveit-resources-*    \
    ros-${ROS2_DISTRO}-moveit-ros-*          \
    ros-${ROS2_DISTRO}-moveit-runtime        \
    ros-${ROS2_DISTRO}-moveit-servo-*        \
    ros-${ROS2_DISTRO}-moveit-setup-*        \
    ros-${ROS2_DISTRO}-moveit-simple-*       \
    ros-${ROS2_DISTRO}-moveit-visual-*       \
    ros-${ROS2_DISTRO}-pinocchio \
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
# IMAGE_ROOTFS_EXTRA_SPACE = "5000000"
