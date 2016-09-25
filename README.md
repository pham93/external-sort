# external-sort
#Sorting gigabytes size files.

Often times, you can sort everything in main memory, but what happen if your data exceeds main memory? 
External sort fixes the problem by reading in data from disk to main memory, sorts the file, then writes it back to disk. 
The disavantage of using external sort is the amount of reads and writes. It can slow down performance, but with multithreads, we can speed it up 2x to 4x times. 

