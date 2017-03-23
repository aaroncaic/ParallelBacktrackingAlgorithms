#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>
#include <chrono>
#include <limits>

int qRecord;      // Min number of queens to dominate board
int** sol;		  // Final solution board
int n;						// Board dimensions
int level;
int numTasks;
std::mutex sol_mutex;

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

/*
 * Copy contents from src to dest.
 */
void deleteArray(int** src) {
  for(int i = 0; i < n; i++) {
    delete[] src[i];
  }
  delete[] src;
}

/*
 * Copy contents from src to dest.
 */
void copyArray(int** src, int** dest) {
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      dest[i][j] = src[i][j];
    }
  }
}

/*
 * Place a queen at (i,j) on board and update queen's
 * attacking positions on board. Keep track and return number of
 * number of spots on board that does not contain a queen
 * and isn't attacked.
 */
 int putQueens(int** b, int i, int j, int numQ, int numZeros) {
  if(b[i][j] > 0) {
    std::cerr << "Something went wrong" << std::endl;
    return -1;
  }

  int newNumZeros = numZeros;

  if(b[i][j] == 0) { newNumZeros--;}
  b[i][j] = numQ + 1;

  // Attack entire column
  for(int k = i + 1; k < n; k++) {
    if(b[k][j] == 0) { newNumZeros--; }
    if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }
  }
  for(int k = i - 1; k >= 0; k--) {
    if(b[k][j] == 0) { newNumZeros--; }
    if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }

  }

  // Attack entire row
  for(int k = j + 1; k < n; k++) {
    if(b[i][k] == 0) { newNumZeros--; }
    if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
  }
  for(int k = j - 1; k >= 0; k--) {
    if(b[i][k] == 0) { newNumZeros--; }
    if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
  }

  // Attack all diagonals
  int k = i + 1;
  int l = j + 1;
  while(k < n && l < n) {
    if(b[k][l] == 0) { newNumZeros--; }
    if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
    k++;
    l++;
  }
  k = i - 1;
  l = j - 1;
  while(k >= 0 && l >= 0) {
    if(b[k][l] == 0) { newNumZeros--; }
    if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
    k--;
    l--;
  }
  k = i + 1;
  l = j - 1;
  while(k < n && l >= 0) {
    if(b[k][l] == 0) { newNumZeros--; }
    if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
    k++;
    l--;
  }
  k = i - 1;
  l = j + 1;
  while(k >= 0 && l < n) {
    if(b[k][l] == 0) { newNumZeros--; }
    if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
    k--;
    l++;
  }

  return newNumZeros;
}

/*
 * Main algorithm V4 (Parallel)
 * Notes: Eliminated memory leaks.
 *        Contains some optimizations.
 */
void QDBT4(int** b, int i, int j, int numQ, int numZeros) {
  if(numQ >= qRecord) {
    deleteArray(b);
    return;
  }

  std::vector<std::thread> threads;
  int k = i;
  int l = j;
  int counter = 0;
  while(k < n) {
    if(k != i) { l = 0; }
    while(l < n) {

      int** newB = new int*[n];
      for(int h = 0; h < n; h++) {
        newB[h] = new int[n];
      }
      copyArray(b, newB);
      int newNumZeros = putQueens(newB, k, l, numQ, numZeros);

      if(newNumZeros <= 0 && numQ < qRecord) {
        sol_mutex.lock();
        qRecord = numQ;
        //std::cout << qRecord << std::endl;
        sol = newB;
        sol_mutex.unlock();
        deleteArray(b);
        for (auto& th : threads) th.join();
        return;
      }

      // Compute position of next queen
      int newI = -1;
      int newJ = -1;
      if(l < n - 1) {
        newJ = l + 1;
        newI = k;
      } else if (k < n - 1){
        newI = k + 1;
        newJ = 0;
      } else {
        deleteArray(newB);
        deleteArray(b);
        for (auto& th : threads) th.join();
        return;
      }

      if(numQ < level && counter < numTasks) {
        threads.push_back(std::thread(QDBT4, newB, newI, newJ, numQ+1, newNumZeros));
      } else {
        QDBT4(newB, newI, newJ, numQ+1, newNumZeros);
      }

      l++;
      counter++;
    }
    k++;
  }

  deleteArray(b);
  for (auto& th : threads) th.join();
  return;
}

/*
 * Main algorithm V2.1 (Serial)
 * Notes: Attempting to eliminate memory leaks in serial version
 */
void QDBT3(int** b, int i, int j, int numQ, int numZeros) {
  if(numQ >= qRecord) {
    deleteArray(b);
    return;
  }

  int k = i;
  int l = j;
  while(k < n) {
  //for(int k = i; k < n; k++) {
    if(k != i) { l = 0; }
    while(l < n) {
    //for(int l = j; l < n; l++) {
      int** newB = new int*[n];
      for(int h = 0; h < n; h++) {
        newB[h] = new int[n];
      }
      copyArray(b, newB);
      int newNumZeros = putQueens(newB, k, l, numQ, numZeros);

      if(newNumZeros <= 0 && numQ < qRecord) {
        qRecord = numQ;
        //std::cout << qRecord << std::endl;
        sol = newB;
        deleteArray(b);
        return;
      }

      // Compute position of next queen
      int newI = -1;
      int newJ = -1;
      if(l < n - 1) {
        newJ = l + 1;
        newI = k;
      } else if (k < n - 1){
        newI = k + 1;
        newJ = 0;
      } else {
        deleteArray(newB);
        deleteArray(b);
        return;
      }


      QDBT3(newB, newI, newJ, numQ+1, newNumZeros);
      //QDBT2(newB, newI, newJ, numQ+1, newNumZeros);
      l++;
    }
    k++;
  }
  deleteArray(b);
  return;
}

/*
 * Main algorithm V3 (Parallel)
 * Notes: Different approach, while loops.
 *        Very slow.
 */
void QDBT2(int** b, int i, int j, int numQ, int numZeros) {
  if(numQ >= qRecord) {
    deleteArray(b);
    return;
  }

  std::vector<std::thread> threads;
  int k = i;
  int l = j;
  while(k < n) {
    if(k != i) { l = 0; }
    while(l < n) {
      int** newB = new int*[n];
      for(int h = 0; h < n; h++) {
        newB[h] = new int[n];
      }
      copyArray(b, newB);
      int newNumZeros = putQueens(newB, k, l, numQ, numZeros);

      if(newNumZeros <= 0 && numQ < qRecord) {
        sol_mutex.lock();
        qRecord = numQ;
        sol = newB;
        sol_mutex.unlock();
        deleteArray(b);
        for (auto& th : threads) th.join();
        return;
      }

      // Compute position of next queen
      int newI = -1;
      int newJ = -1;
      if(l < n - 1) {
        newJ = l + 1;
        newI = k;
      } else if (k < n - 1){
        newI = k + 1;
        newJ = 0;
      } else {
        deleteArray(newB);
        deleteArray(b);
        for (auto& th : threads) th.join();
        return;
      }

      if(numQ >= level) {
        QDBT2(newB, newI, newJ, numQ+1, newNumZeros);
      } else {
        threads.push_back(std::thread(QDBT2, newB, newI, newJ, numQ+1, newNumZeros));
      }
      l++;
    }
    k++;
  }

  for (auto& th : threads) th.join();
  deleteArray(b);
  return;
}

/*
 * Main algorithm V2 (Serial)
 * Notes: Different approach, for loops.
 *        Contains bugs.
 *        Still contains memory leaks.
 */
void QDBT1(int** b, int i, int j, int numQ, int numZeros) {
  if(numQ >= qRecord) {
    return;
  }

  for(int k = i; k < n; k++) {
    for(int l = j; l < n; l++) {
      // Place queen on (i,j) and update number of non-attacked spots
      int** newB = new int*[n];
      for(int h = 0; h < n; h++) {
        newB[h] = new int[n];
      }
      copyArray(b, newB);
      int newNumZeros = putQueens(newB, k, l, numQ, numZeros);

      // Found a new min solution, update solution.
      // Return since any solution other solutions
      // on this branch is not minimum.
      //sol_mutex.lock();
      if(newNumZeros <= 0 && numQ < qRecord) {
        qRecord = numQ;
        sol = newB;
        return;
      }

      // Compute position of next queen
      int newI = -1;
      int newJ = -1;
      if(l < n - 1) {
        newJ = l + 1;
        newI = k;
      } else if (k < n - 1){
        newI = k + 1;
        newJ = 0;
      } else {
        return;
      }

      QDBT1(newB, newI, newJ, numQ+1, newNumZeros);
    }
  }

}

/*
 * Main algorithm V1 (Serial)
 * Notes: Contains memory leaks
 */
void QDBT(int** b, int i, int j, int numQ, int numZeros) {
  // Exeeded min number of queens, return
  if(numQ >= qRecord) {
    return;
  }

  // Place queen on (i,j) and update number of non-attacked spots
  int** newB = new int*[n];
  for(int i = 0; i < n; i++) {
    newB[i] = new int[n];
  }
  copyArray(b, newB);
  int newNumZeros = putQueens(newB, i, j, numQ, numZeros);

  // Found a new min solution, update solution.
  // Return since any solution other solutions
  // on this branch is not minimum.
  if(newNumZeros <= 0 && numQ < qRecord) {
    qRecord = numQ;
    sol = newB;
    return;
  }

  // Compute position of next queen
  int newI = -1;
  int newJ = -1;
  if(j < n - 1) {
    newJ = j + 1;
    newI = i;
  } else if (i < n - 1){
    newI = i + 1;
    newJ = 0;
  } else {
    return;
  }

  QDBT(newB, newI, newJ, numQ+1, newNumZeros);
  QDBT(b, newI, newJ, numQ, numZeros);

}

int main ( int argc, char *argv[] ) {
  if(argc != 2) {
    std::cout << "Invalid command line arguments" << std::endl;
    return 0;
  }

  n = std::atoi(argv[1]);
  numTasks = n;
  level = 1;

  // Set minimum number of queens needed to dominate board
  qRecord = n;

  // Create initial board
  int** b = new int*[n];
  for(int i = 0; i < n; i++) {
    b[i] = new int[n];
    for(int j = 0; j < n; j++) {
      b[i][j] = 0;
    }
  }

  // Number of spots on board that are not attacked
  int numZeros = n*n;

  auto t1 = std::chrono::high_resolution_clock::now();
	QDBT4(b, 0, 0, 0, numZeros);
  auto t2 = std::chrono::high_resolution_clock::now();

  if(sol == NULL) {
    std::cerr << "no solution found" << std::endl;
    return 0;
  }

  // integral duration: requires duration_cast
  auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  // fractional duration: no duration_cast needed
  std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

  std::cout << ("Solution:") << std::endl;
  for(int i = 0; i < n; i++) {
    std::string row = "";
    for(int j = 0; j < n; j++) {
      if(sol[i][j] <= 0) {
        row = row + "[" + patch::to_string(sol[i][j]) + "]";
      } else {
        row = row + "[ " + patch::to_string(sol[i][j]) + "]";
      }
    }
    std::cout << row << std::endl;
  }

  std::cout << qRecord << "Execution time: " << fp_ms.count() << " ms" <<std::endl;

  return 0;
}
