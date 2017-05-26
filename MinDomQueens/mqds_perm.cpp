#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <chrono>
#include <limits>
#include <math.h>
#include <algorithm>

int qRecord;      // Min number of queens to dominate board
int** sol;		  // Final solution board
int n;						// Board dimensions
int level;
int max_degree;
int numTasks;
int first_queen_i;
int first_queen_j;
int reached_end;
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

unsigned long long binomialCoeff(int m, int k)
{
    unsigned long long C[k+1];
    //memset(C, 0, sizeof(C));
    for(int i = 0; i < k+1; i++) {
      C[i] = 0;
    }

    C[0] = 1;  // nC0 is 1

    for (int i = 1; i <= m; i++)
    {
        // Compute next row of pascal triangle using
        // the previous row
        for (int j = std::min(i, k); j > 0; j--){
            C[j] = C[j] + C[j-1];
        }
    }
    return C[k];
}

void permutation_at(int* perm, int m, int nq, unsigned long long index) {
  //int perm[n];
  //int* perm = new int[n];
  for(int i = 0; i < m; i++) {
    perm[i] = 0;
  }

  int curr = nq;
  unsigned long long sum_index = 0;
  for(int i = 0; i < m; i++) {
    if((binomialCoeff(m-i-1, curr))+sum_index > index) {
      continue;
    }
    sum_index += binomialCoeff(m-i-1, curr);
    if((binomialCoeff(m-i-1, curr-1))+sum_index > index) {
      perm[i] = 1;
      curr--;
    }
  }
  /*for(int i = 0; i < n; i++) {
    std::cout << perm[i] << ' ';
  }*/
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

void QDBT7(int myrank, int numQ, int* perm, unsigned long long perm_index, unsigned long long num_perm_per_rank) {
  int board_size = n*n;
  //int* perm = new int[board_size];
  //unsigned long long perm_index = (unsigned long long)myrank * num_perm_per_rank;
  unsigned long long max_perm_index = perm_index + num_perm_per_rank;

  //permutation_at(perm, board_size, numQ, perm_index);
  unsigned long long count = 0;

  do {
    if(numQ >= qRecord) {
      return;
    }
    int** newB = new int*[n];
    for(int h = 0; h < n; h++) {
      newB[h] = new int[n];
      for(int l = 0; l < n; l++) {
        newB[h][l] = 0;
      }
    }

    int newNumZeros = n*n;
    int queen_index = 0;
    for(int i = 0; i < board_size; i++) {
      if(perm[i] == 1) {
        int row = i/n;
        int col = i%n;

        newNumZeros = putQueens(newB, row, col, queen_index, newNumZeros);
        queen_index++;
      }
    }

    if(newNumZeros <= 0) {
      sol_mutex.lock();
      if(numQ < qRecord) {
        qRecord = numQ;
        //std::cout << qRecord << std::endl;
        sol = newB;
        //sol_mutex.unlock();
        //deleteArray(newB);
        //for (auto& th : threads) th.join();
        //return;
        sol_mutex.unlock();
      } else {
        sol_mutex.unlock();
        deleteArray(newB);
      }
      return;
    }

    deleteArray(newB);
    count++;
    //std::cout << myints[0] << ' ' << myints[1] << ' ' << myints[2] << '\n';
  } while ( count < max_perm_index && std::next_permutation(perm,perm+board_size) );

  //delete[] perm;
}

void start_up(int myrank) {
  int board_size = n*n;

  for(int numQ = 1; numQ < qRecord; numQ++) {
    int* perm = new int[board_size];
    unsigned long long num_perm = binomialCoeff(board_size, numQ);
    unsigned long long num_perm_per_rank = num_perm/numTasks;
    unsigned long long perm_index = (unsigned long long)myrank * num_perm_per_rank;
    if(myrank == numTasks - 1) {
      num_perm_per_rank += num_perm % numTasks;
    }
    //unsigned long long max_perm_index = perm_index + num_perm_per_rank;

    permutation_at(perm, board_size, numQ, perm_index);

    QDBT7(myrank, numQ, perm, perm_index, num_perm_per_rank);

    delete[] perm;

    if(numQ >= qRecord) {
      return;
    }
  }

}


int main ( int argc, char *argv[] ) {
  if(argc != 2) {
    std::cout << "Invalid command line arguments" << std::endl;
    return 0;
  }

  n = std::atoi(argv[1]);
  numTasks = n;


  // Set minimum number of queens needed to dominate board
  //qRecord = n-1;
  qRecord = n+1;//ceil(n/2.0) + 2;

  std::vector<std::thread> threads;

  auto t1 = std::chrono::high_resolution_clock::now();
  for(int i = 1; i < numTasks; i++) {
    threads.push_back(std::thread(start_up, i));
  }
  start_up(0);
  for (auto& th : threads) th.join();
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

  std::cout << "Execution time: " << fp_ms.count() << " ms" <<std::endl;
  free(sol);
  return 0;
}
