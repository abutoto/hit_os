cd /
cat unistd.h > /usr/include/unistd.h
cat sem.h > /usr/include/linux/sem.h
gcc -o pc pc.c -Wall > log 2>&1
./pc > log
#head -1000 /var/process.log >> log
sync
