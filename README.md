# external-sort
Sorting gigabytes size files.
Often times, you can sort everything in main memory, but what happen if your data you're sorting exceeds the main memory? 
External sort fixes the problem by reading data from disk to main memory, sorting the file, then writing the sort data back to disk. 
The disavantage of using external sort is the amount of reads and writes. It can slows down performance, but with multithreads, we can speed it up 2x to 4x times. 

