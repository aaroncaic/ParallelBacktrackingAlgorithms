#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <string>
#include <time.h>
#include <chrono>

int level;


void printSolution(std::vector<int> board, int n) {
  std::string solution = "Solution -> ";
  for(int i = 0; i < n; i++) {
    solution = solution + std::to_string(i) + ":" + std::to_string(board[i]) + " ";
  }
  solution = solution + "\n";
  std::cout << solution << std::endl;
  return;
}

bool isValidBoard(std::vector<int> board, int row) {
  for(int i = 0; i < row; i++) {
    if(board[i] == board[row]) {
      return false;
    }
  }

  int leftDiag = board[row] - 1;
  int rightDiag = board[row] + 1;
  for(int i = row - 1; i >=0; i--) {
    if(board[i] == leftDiag || board[i] == rightDiag) {
      return false;
    }
    leftDiag--;
    rightDiag++;
  }

  return true;
}

void NQueens(std::vector<int> board, int row, int n) {
  if(row == n) {
    //printSolution(board, n);
  } else {
    for(int i = 0; i < n; i++) {
      board[row] = i;
      if(isValidBoard(board, row)) {
        NQueens(board, row+1, n);
      }
    }
  }
  return;
}

void NQueensParallel(std::vector<int> board, int row, int n) {
  std::vector<std::thread> threads;

  for(int i = 0; i < n; i++) {
    board[row] = i;
    if(isValidBoard(board, row)) {
      if(row >= level) {
        NQueens(board, row+1, n);
      } else {
        std::vector<int> newBoard(n);
        for(int j = 0; j <= row; j++) { newBoard[j] = board[j]; }

        threads.push_back(std::thread(NQueensParallel, newBoard, row+1, n));
      }
    }
  }

  for (auto& th : threads) th.join();

  return;
}

int main ( int argc, char *argv[] ) {
  if(argc != 2) {
    std::cout << "Invalid command line arguments" << std::endl;
    return 0;
  }

  level = 2;
  int n = std::atoi(argv[1]);
  std::vector<int> board(n);



  auto t1 = std::chrono::high_resolution_clock::now();
  NQueensParallel(board, 0, n);
  auto t2 = std::chrono::high_resolution_clock::now();

  auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

  std::cout << "Total time: " << fp_ms.count() << " ms" <<std::endl;

  return 0;
}
