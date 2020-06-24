cd linux-0.11
make
cd ..
sudo ./mount-hdc
sudo cp linux-0.11/include/linux/sem.h hdc/
sudo cp ~/Documents/backup/test/pc.c hdc/
sudo cp ~/Documents/backup/test/run.sh hdc/
sudo cp /mnt/hgfs/share/HIT-Linux-0.11/5-semaphore/linux-0.11/pc-new.c hdc/pc_1.c
sudo cat hdc/log > ~/Documents/backup/test/log
sudo rm hdc/pc
sudo rm hdc/buffer.txt
sudo umount hdc
