#!/bin/bash

# Our sysctl call may be run as normal user,
# but /sbin is not always in normal users' PATH.
if [[ -f /sbin/sysctl && -x /sbin/sysctl ]]; then sysctl_bin=/sbin/sysctl
else sysctl_bin=sysctl  # If not found in sbin, hope
fi

export ROOT=`pwd`
export NCORES=`"$sysctl_bin" -n hw.ncpu`

while getopts ":ih" opt; do
  case ${opt} in
    i )
        export BUILD_IMAGE="1"
        ;;
    h ) echo "Usage: ./BuildLinuxImage.sh [-i]"
        echo "   -i: Generate Appimage (optional)"
        exit 0
        ;;
  esac
done

echo -n "[9/9] Generating Linux app..."
#{
    # create directory and copy into it
    if [ -d "package" ]
    then
        rm -rf package/*
        rm -rf package/.* 2&>/dev/null
    else
        mkdir package
    fi
    mkdir package/bin

    # copy Resources
    cp -Rf ../resources package/resources
    cp -f src/prusa-slicer package/bin/prusa-slicer
    # remove unneeded po from resources
    find package/resources/localization -name "*.po" -type f -delete

    # create bin
    echo -e '#!/bin/bash\nDIR=$(readlink -f "$0" | xargs dirname)\nexport LD_LIBRARY_PATH="$DIR/bin"\nexec "$DIR/bin/pleccer" "$@"' >pleccer
    chmod ug+x prusa-slicer
    cp -f prusa-slicer package/prusa-slicer
    pushd package
    tar -cvf ../Pleccer.tar .  &>/dev/null
    popd
#} &> $ROOT/Build.log # Capture all command output
echo "done"

if [[ -n "$BUILD_IMAGE" ]]
then
echo -n "Creating Appimage for distribution..."
#{
    pushd package
    chmod +x ../build_appimage.sh
    ../build_appimage.sh
    popd
    mv package/"Pleccer_ubu64.AppImage" "Pleccer_ubu64.AppImage"
#} &> $ROOT/Build.log # Capture all command output
echo "done"
fi
