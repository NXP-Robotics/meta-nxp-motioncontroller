source nxp-setup-alb.sh $@

if [ -f conf/local.conf ]; then
    echo ""                                                                       >> conf/local.conf
    echo "# Switch to rpm packaging to avoid rootfs build break"                  >> conf/local.conf
    echo "PACKAGE_CLASSES = \"package_rpm\""                                      >> conf/local.conf
    echo ""                                                                       >> conf/local.conf
    echo "# Save lots of disk space"                                              >> conf/local.conf
    echo "INHERIT += \"rm_work\""                                                 >> conf/local.conf
    echo ""                                                                       >> conf/local.conf
    echo "# Set your proxy if necessary"                                          >> conf/local.conf
    echo "#ENV_HOST_PROXIES = \"http_proxy=\""                                    >> conf/local.conf
    echo ""                                                                       >> conf/local.conf

	echo "BSPDIR := \"\${@os.path.abspath(os.path.dirname(d.getVar('FILE', True)) + '/../..')}\""    >> conf/bblayers.conf
    echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-motioncontroller/meta-jammy-humble\""            >> conf/bblayers.conf
	echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-motioncontroller/meta-middleware\""              >> conf/bblayers.conf

    echo ""
    echo "NXP motion controller setup complete!"
    echo ""
    echo "You can now build the following Ubuntu images:"
    echo ""
    echo "$ bitbake -k nxp-image-ubuntu"
    echo "$ bitbake -k nxp-image-ubuntu-ros2"
    echo ""
fi
