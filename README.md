# Distributed_programing
## dp1
1. (MPI – Primitives) Finding prefix sums is a generalization of global sum. Rather than simply finding the sum of n values, X0 + X1 + X2 + ... + Xn-1, the prefix sums are the n partial sums X0, X0 + X1, X0 + X1 + X2, ..., X0 + X1 + ... + Xn-1. MPI provides a collective communication function, MPI_Scan, that can be used to compute prefix sums. 
(1) Understand the semantics of MPI_Scan operation and devise at least two parallel prefix sum algorithms (i.e., explain the algorithms without MPI notation). 
(2) Implement this operation using only MPI send and receive (blocking and non blocking) calls. When implementing your solutions, make sure that your implementation is not dependent on the number of processors used. Verify your results by generating n random integers and compare the performance with that of original MPI_Scan as you increase the number of nodes involved. Discuss the results.

2. (MPI - Image Processing) It is generally agreed that topics in image processing have the high potential for significant parallelism. In this question, you are to read in a PPM (Portable Pix Map) file in P6 format (full color), and write sequential and parallel program written in MPI to
	(a) flip an image horizontally (mirroring) and
	(b) reduce the image to grayscale by taking the average of the red, green, and blue 	values for each pixel and
	(c) smooth the image by calculating the mean of each pixel’s value and its eight 	neighbours (some algorithms consider only the values from the diagonal neighbours 	or the horizontal and vertical neighbours).
When implementing your MPI programs, try to use MPI derived data types as much as you can. Compare the performance of sequential and parallel versions of the program and discuss the results over a cluster of workstations. Use different PPM files with various data sizes and discuss the scalability aspects of your code as you increase the number of nodes. When submitting your code, include the short report on the questions above, the sample PPM files used and your programs (sequential and parallel versions). Also include the name of the PPM viewer(Linux version) in a readme file.

## dp2
1. RPC programing -SUN RPC 
2. Multithreaded Programming -pthread read-write lock
3. Multithreaded Server
  1. Test your web server using your client program.
  2. Vary the number of threads in both client and server and report your observations as well as the performance of the server
  3. Vary also other parameters (e.g., size of file, interval, etc) and report your observations as well as the performance of the server
  4. Be careful of the caching effect. Since you are making repeated requests, your files are likely to be cached. Check the performance of your server when the files are cached, thereby a little disk I/O may occur. How much is the performance benefit in this environment ? Try to make your server having substantial I/O and check the performance benefits. Report your observations. 
  5. Try to modify the client and server codes so that they maintain the connections (persistent connections) as long as each client has something to send to the server. In this case, the server should be modified to follow peer model with thread pool and each worker uses epoll mechanism (https://en.wikipedia.org/wiki/Epoll) to monitor multiple connections. Compare the performance with original implementation by varying various parameters and report the results as well as how you have implemented client/server codes.
