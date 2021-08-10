
A sample program for interprocess communication using xpmem.  
Measure shared memory and xpmem execution time.

<pre>
$ gcc -Wall -lrt -lpthread -lxpmem -o sample sample.c
$ ./sample
xpmem_version=26005, buffer=2097152 byte
segid=200000668, ctrl=0x7f8bb36000, local=0x7f8b531000, share=0x7f8b731000
apid=100000669, xpmem=0x7f8b331000
type=shmem, time=9034 us
type=shmem, time=2452 us
type=shmem, time=2391 us
type=shmem, time=2416 us
type=shmem, time=2539 us
type=shmem, time=2431 us
type=shmem, time=2383 us
type=shmem, time=2386 us
type=shmem, time=2515 us
type=shmem, time=2382 us
type=shmem, time=2529 us
type=shmem, time=2397 us
type=shmem, time=2499 us
type=shmem, time=2430 us
type=shmem, time=2350 us
type=shmem, time=2455 us
type=xpmem, time=4074 us
type=xpmem, time=1713 us
type=xpmem, time=1675 us
type=xpmem, time=1697 us
type=xpmem, time=1742 us
type=xpmem, time=1653 us
type=xpmem, time=1678 us
type=xpmem, time=1659 us
type=xpmem, time=1728 us
type=xpmem, time=1663 us
type=xpmem, time=1693 us
type=xpmem, time=1705 us
type=xpmem, time=1705 us
type=xpmem, time=1700 us
type=xpmem, time=1641 us
type=xpmem, time=1738 us
</pre>