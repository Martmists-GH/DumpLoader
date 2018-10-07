make clean
make all
if [[ "$TARGET" != "CITRA" ]]; then
    rm dumploader.3dsx
fi
rm -rf build dumploader.elf dumploader.smdh banner.bnr icon.icn