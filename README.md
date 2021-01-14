# MP3-Fragmentation
The program, which will be invoked as defrag will have to traverse through a directory tree looking for pieces of an mp3, each piece's filename would be made up of a non-negative integer and the file-extension '.mp3'. The file contents of those pieces will be concatenated into an output file in the order the numbers imply.

There will be two arguments, a path to the directory tree to explore, and the name of the output file. The starting directory in the traversal will always only contain subdirectories. The program will explore those subdirectories using asynchronous threads.

File numbers will always start at 0, each integer only used once, and the order the files are discovered is not necessarily (rarely) in numeric order.

## Intended Implementation:
The intended implementation of this program is that there is a dynamically resized global array of file contents shared by all threads. When a thread finds an mp3 file, it will read the file contents and store it in the array in the index of the mp3 piece number. Since the total number of pieces are not known, the array will be realloc'd, therefore, it will be protected by a mutex.

## Example:
The defrag would be run as: 
```
$ ./defrag dirs output.mp3
```
