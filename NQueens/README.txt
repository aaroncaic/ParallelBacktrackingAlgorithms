Instructions:

NOTE: This program does NOT print the solution.
      It only prints the amount of time it took to find the solution.
      Mainly because I/O adds to execution time.

1. Compile with:
    g++ nqueens_parallel_backtrack.cpp -o nqueens -pthread -std=c++11

2. Run with:
    ./nqueens [n]

    For example, find maximum number of queens that can be placed on a 5x5 chessboard:

    ./nqueens 5
