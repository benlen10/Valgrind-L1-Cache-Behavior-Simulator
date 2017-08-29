# UW Mail Server Simulator

NOTICE: This project was recently moved to its own repository. It was originally developed in the ‘Current’ repository (Now renamed to [UniCade](https://github.com/benlen10/UniCade)) which contains the previous commits and versions.  

# Project Description
- A cache simulator that can replay traces from Valgrind
 
 - This program outputs statistics including number of cache hits, misses, and evictions  

- The replacement policy is LRU (Least recently used)
 
- This program was originally developed as a school project at UW Madison

# Usage & Supported Arguments
 - Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n
 - -h         Print this help message
 - -v         Optional verbose flag
 - -s <num>   Number of set index bits.
 - -E <num>   Number of lines per set.
 - -b <num>   Number of block offset
 - -t <file>  Trace file.

 # Example Usage
 -  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n

 - linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n




# Implementation Details
 - Each load/store can cause at most one cache miss

 - Instruction loads (I) are ignored

 - Data modify (M) operations are treated as a load followed by a store to the same
 address


