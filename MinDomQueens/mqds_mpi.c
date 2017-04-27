/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<mpi.h>
#include<pthread.h>

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/
#define MASTER 0
#define WORKER 1
#define UPDATE 2
#define WORK 3
#define SOLUTION 4

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/
int** min_dom_set;
int min_dom_size;
int local_min_dom_size;
int n;

int mpi_myrank;
int mpi_commsize;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
//pthread_barrier_t barrier;

void * receive_min(void * arg) {

  //pthread_t mytid = pthread_self();   /* get my thread ID */

  /* pthread_detach() enables pthreads to reclaim the
     thread resources when this thread terminates
     (i.e., do not leave a "zombie thread" behind) */
  //int rc = pthread_detach( mytid );

    /* without the above pthread_detach() call, pthreads keeps
       this thread's resources intact such that the main thread
       can call pthread_join() to join this thread back in... */

  // if ( rc != 0 )
  // {
  //   fprintf( stderr, "pthread_detach() failed (%d): %s\n",
  //            rc, strerror( rc ) );
  //   return NULL;
  // }

  while(1) {
    MPI_Status status;
    int new_min_dom_size;
    // receive message from any source
    MPI_Recv(&new_min_dom_size, 1, MPI_INT, MPI_ANY_SOURCE, UPDATE, MPI_COMM_WORLD, &status);
    if(new_min_dom_size == -1) { break; }
    pthread_mutex_lock(&mtx);
    if(new_min_dom_size < min_dom_size) {
      min_dom_size = new_min_dom_size;
    }
    pthread_mutex_unlock(&mtx);
    // send reply back to sender of the message received above
    //MPI_send(buf, 32, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
  }

  pthread_exit (NULL);
}

void send_min_all() {
  // Initialize requests used for MPI_Isend
  MPI_Request send_request;
  int original_min_dom_size = local_min_dom_size;

  int i;
  for(i = 0; i < mpi_commsize; i++) {
    if(i == mpi_myrank) { continue; }

    int send_dest = i;
    int send_tag = UPDATE;
    // Send off the top and bottom rows
    MPI_Isend(&original_min_dom_size, 1, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD, &send_request);
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
    printf("Something went wrong\n");
    return -1;
  }

  int newNumZeros = numZeros;

  if(b[i][j] == 0) { newNumZeros--;}
  b[i][j] = numQ + 1;

  int k, l;
  // Attack entire column
  for(k = i + 1; k < n; k++) {
    if(b[k][j] == 0) { newNumZeros--; }
    if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }
  }
  for(k = i - 1; k >= 0; k--) {
    if(b[k][j] == 0) { newNumZeros--; }
    if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }

  }

  // Attack entire row
  for(k = j + 1; k < n; k++) {
    if(b[i][k] == 0) { newNumZeros--; }
    if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
  }
  for(k = j - 1; k >= 0; k--) {
    if(b[i][k] == 0) { newNumZeros--; }
    if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
  }

  // Attack all diagonals
  k = i + 1;
  l = j + 1;
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
 * Copy contents from src to dest.
 */
void deleteArray(int** src) {
  int i;
  for(i = 0; i < n; i++) {
    free(src[i]);
  }
  free(src);
}

/*
 * Copy contents from src to dest.
 */
void copyArray(int** src, int** dest) {
  int i, j;
  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++) {
      dest[i][j] = src[i][j];
    }
  }
}


/*
 * Main algorithm V1 (Serial)
 * Notes: Contains memory leaks
 */
void QDBT(int** b, int i, int j, int numQ, int numZeros) {
  // Exeeded min number of queens, return
  if(numQ + 1 >= min_dom_size) {
    deleteArray(b);
    return;
  }

  // Place queen on (i,j) and update number of non-attacked spots
  int k = 0;
  int** newB = (int**)malloc(n * sizeof(int*));
  for(k = 0; k < n; k++) {
    newB[k] = (int*)malloc(n * sizeof(int));
    int l = 0;
    for(l = 0; l < n; l++) {
      newB[k][l] = b[k][l];
    }
  }
  //copyArray(b, newB);
  int newNumZeros = putQueens(newB, i, j, numQ, numZeros);

  // Found a new min solution, update solution.
  // Return since any solution other solutions
  // on this branch is not minimum.
  if(newNumZeros <= 0) {
    pthread_mutex_lock(&mtx);
    if(numQ + 1 < min_dom_size) {
      min_dom_size = numQ + 1;
      min_dom_set = newB;
      pthread_mutex_unlock(&mtx);
      deleteArray(b);
      local_min_dom_size = numQ + 1;
      send_min_all();
      return;
    }
    pthread_mutex_unlock(&mtx);
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
    deleteArray(newB);
    deleteArray(b);
    return;
  }

  QDBT(newB, newI, newJ, numQ+1, newNumZeros);
  QDBT(b, newI, newJ, numQ, numZeros);
}

/*
 * Main algorithm V1 (Serial)
 * Notes: Contains memory leaks
 */
void QDBT_start(int** b, int i, int j, int numQ, int numZeros) {
  // Exeeded min number of queens, return
  if(numQ + 1 >= min_dom_size) {
    deleteArray(b);
    return;
  }

  // Place queen on (i,j) and update number of non-attacked spots
  int k = 0;
  int** newB = (int**)malloc(n * sizeof(int*));
  for(k = 0; k < n; k++) {
    newB[k] = (int*)malloc(n * sizeof(int));
    int l = 0;
    for(l = 0; l < n; l++) {
      newB[k][l] = b[k][l];
    }
  }
  //copyArray(b, newB);
  int newNumZeros = putQueens(newB, i, j, numQ, numZeros);

  // Found a new min solution, update solution.
  // Return since any solution other solutions
  // on this branch is not minimum.
  if(newNumZeros <= 0) {
    pthread_mutex_lock(&mtx);
    if(numQ + 1 < min_dom_size) {
      min_dom_size = numQ + 1;
      min_dom_set = newB;
      pthread_mutex_unlock(&mtx);
      deleteArray(b);
      local_min_dom_size = numQ + 1;
      send_min_all();
      return;
    }
    pthread_mutex_unlock(&mtx);
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
    deleteArray(newB);
    deleteArray(b);
    return;
  }

  QDBT(newB, newI, newJ, numQ+1, newNumZeros);
}




int main(int argc, char *argv[])
{

  MPI_Init( &argc, &argv);
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
  MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);

  MPI_Barrier( MPI_COMM_WORLD );




  int i, j = 0;
  n = atoi(argv[1]);
  min_dom_size = n;
  local_min_dom_size = n;

  int** board;
  board = (int**)malloc(n * sizeof(int*));
  min_dom_set = (int**)malloc(n * sizeof(int*));
  for(i = 0; i < n; i++) {
    board[i] = (int*)malloc(n * sizeof(int));
    min_dom_set[i] = (int*)malloc(n * sizeof(int));
    for(j = 0; j < n; j++) {
      board[i][j] = 0;
      min_dom_set[i][j] = 0;
    }
  }

  int start_queen_row = mpi_myrank/n;
  int start_queen_col = mpi_myrank%n;
  //int num_available_workers = mpi_commsize/n;

  // Create thread
  pthread_t tid;
  int rc = pthread_create(&tid, NULL, receive_min, NULL);
  if ( rc != 0 ){
    fprintf( stderr, "pthread_create() failed (%d): %s", rc, strerror( rc ) );
    //free(thread_num);
  }

  MPI_Barrier( MPI_COMM_WORLD );
  double starttime, endtime;
  if(mpi_myrank == MASTER) {
    starttime = MPI_Wtime();
  }
  QDBT_start(board, start_queen_row, start_queen_col, 0, n*n);
  MPI_Barrier( MPI_COMM_WORLD );
  // Stop timer if rank 0 (master)
  if(mpi_myrank == MASTER) {
    endtime = MPI_Wtime();
  }

  // Output performance results if rank 0 (master)
  if(mpi_myrank == MASTER) {
    printf("The program took: %f seconds\n", endtime-starttime);
  }



  // Get solution from other ranks
  if(mpi_myrank == MASTER && local_min_dom_size == min_dom_size) {
    printf("1Solution: %d\n", min_dom_size);
    for(i = 0; i < n; i++) {
      //char row[(n*5)+1] = {'\0'};
      //std::string row = "";
      for(j = 0; j < n; j++) {
        if(min_dom_set[i][j] <= 0) {
          printf("[%d]", min_dom_set[i][j]);
          //strcat(row, "[\0");
          //row = row + "[" + patch::to_string(sol[i][j]) + "]";
        } else {
          printf("[ %d]", min_dom_set[i][j]);
          //strcat(row, "[ \0");
          //row = row + "[ " + patch::to_string(sol[i][j]) + "]";
        }
        //char num[4];
        //itoa(sol[i][j], num, 10)
        //strcat(row, num);
        //strcat(row, "]\0");
      }
      printf("\n");
    }
  } else if(mpi_myrank == MASTER) {
    MPI_Status status;

    // int** sol_board;
    // sol_board = (int**)malloc(n * sizeof(int*));
    // for(i = 0; i < n; i++) {
    //   sol_board[i] = (int*)malloc(n * sizeof(int));
    // }
    int sol_board[n][n];

    // receive message from any source
    MPI_Recv(&sol_board[0][0], n*n, MPI_INT, MPI_ANY_SOURCE, SOLUTION, MPI_COMM_WORLD, &status);

    printf("2Solution: %d\n", min_dom_size);
    for(i = 0; i < n; i++) {
      //char row[(n*5)+1] = {'\0'};
      //std::string row = "";
      for(j = 0; j < n; j++) {
        if(sol_board[i][j] <= 0) {
          printf("[%d]", sol_board[i][j]);
          //strcat(row, "[\0");
          //row = row + "[" + patch::to_string(sol[i][j]) + "]";
        } else {
          printf("[ %d]", sol_board[i][j]);
          //strcat(row, "[ \0");
          //row = row + "[ " + patch::to_string(sol[i][j]) + "]";
        }
        //char num[4];
        //itoa(sol[i][j], num, 10)
        //strcat(row, num);
        //strcat(row, "]\0");
      }
      printf("\n");
    }
    //deleteArray(sol_board);
    // for(i = 0; i < n; i++) {
    //   free(sol_board[i]);
    // }
    // free(sol_board);
  } else {
    if(local_min_dom_size == min_dom_size) {
      MPI_Request send_request;

      int send_dest = MASTER;
      int send_tag = SOLUTION;
      // Send off the top and bottom rows
      MPI_Isend(&min_dom_set[0][0], n*n, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD, &send_request);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  //deleteArray(min_dom_set);
  //free(min_dom_set);


  if(mpi_myrank == MASTER) {
    // Initialize requests used for MPI_Isend
    //MPI_Request send_request;
    int sig = -1;

    int i;
    for(i = 0; i < mpi_commsize; i++) {
      if(i == mpi_myrank) { continue; }

      int send_dest = i;
      int send_tag = UPDATE;
      // Send off the top and bottom rows
      MPI_Send(&sig, 1, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD);
    }
  } else if(mpi_myrank == 1) {
    // Initialize requests used for MPI_Isend
    //MPI_Request send_request;
    int sig = -1;

    int send_dest = MASTER;
    int send_tag = UPDATE;
    // Send off the top and bottom rows
    MPI_Send(&sig, 1, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD);
  }


  //Barrier
  pthread_join(tid, NULL);

  MPI_Finalize();
  return 0;

  /*if(mpi_myrank == MASTER) {

    int worker_num = 1;
    i = 0;
    j = 1;
    while(i < n) {
      while(j < n) {

        // Initialize requests used for MPI_Isend
        MPI_Request send_request;

        // Initialize tag and destination variables
        int send_dest = worker_num;
        int send_tag = WORK;

        // Size of ghost rows
        //int send_size = n*n;

        // Send off the top and bottom rows
        MPI_Isend(&i, 1, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD, &send_request);
        MPI_Isend(&j, 1, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD, &send_request);
        //MPI_Isend(&board[0][0], send_size, MPI_INT, send_dest, send_tag, MPI_COMM_WORLD, &send_request);

        //free(newB);

        worker_num++;
        j++;
      }

      j = 0;
      i++;
    }


    // Create thread
    pthread_t tid;
    int rc = pthread_create(&tid, NULL, receive_min, NULL);
    if ( rc != 0 ){
      fprintf( stderr, "pthread_create() failed (%d): %s", rc, strerror( rc ) );
      //free(thread_num);
    }

  } else {
    // Initialize requests used for MPI_Irecv
    MPI_Request recv_request;

    // Initialize tag and source variables
    int recv_source MASTER;
    int recv_tag = WORK;

    // Size of each ghost rows
    //int recv_size = cols;
    int k, l;

    // Start the receives
    MPI_Irecv(&k, 1, MPI_INT, recv_source, recv_tag, MPI_COMM_WORLD, &recv_request);
    MPI_Irecv(&l, 1, MPI_INT, recv_source, recv_tag, MPI_COMM_WORLD, &recv_request);


    QDBT(board, k, l, 0, n*n);
  }*/

}
