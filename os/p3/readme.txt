Project 3 
Batuhan Başkaya 64240

compilation:
gcc -o part1 part1.c
gcc -o part2 part2.c

running:
./part1 BACKING_STORE.bin addresses.txt
./part2 BACKING_STORE.bin addresses.txt [n]
where n is 0 for FIFO, 1 for LRU

in search_tlb, my program just linearly searches the tlb. if it finds the logical page, it returns the physical page. else it refers to the page table. if the page was not in page table, it moves the page from backing to main_memory and updates the page table. finally it updates the tlb (by FIFO) and continues.

in part2, when checking if the page is in memory, it first checks validtable[] to see if the page is in memory. it also keeps a counter for each page to find the last accessed page later on. 
in FIFO, the next page in order (according to first in first out), is replaced from main memory. 
for LRU, first the page with minimum counter is found. then that page is replaced.

maintologic[] array is used to find which logical page is in a physical location.
validtable[] array is used to find if a logical page is in main memory currently.
