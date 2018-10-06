make clean
make
makerom -elf dumploader.elf -rsf app.rsf -icon icon.png -f cia -o dumploader.cia
rm -rf build dumploader.3dsx dumploader.elf dumploader.smdh