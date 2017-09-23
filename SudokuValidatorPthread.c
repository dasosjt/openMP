#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define GRIDN 9

int fd_sudoku;
char *addr_sudoku, *sudoku_grid[GRIDN][GRIDN];
struct stat s;
int matrix_index[3] = {0, 3, 6};

// this fn is called to check columns
int check_columns_fn(void *coord_columns_void_ptr)
{
  // coord x to GRIDN
  int *coord_columns_ptr = (int *)coord_columns_void_ptr, column[GRIDN] = { 0 }, sum = 0;

  #pragma omp parallel for shared(column)
  for(int i = 0; i < GRIDN; i++){
    printf("%d \n", (*(sudoku_grid[(int)*coord_columns_ptr][i]) - '0') - 1);

    int temp_index = (*(sudoku_grid[(int)*coord_columns_ptr][i]) - '0') - 1;

    //if column has repeated values
    if(column[temp_index] == 1){
      printf("repetead value, index %d \n", temp_index);
    }

    column[temp_index] = 1;
  }

  //check sum
  for(int i = 0; i<GRIDN; i++){
    sum += column[i];
  }

  if(sum == GRIDN){
    //TODO: return success
    printf("check_column result %d\n", sum);
    return 1;
  }else{
    //TODO: return error
    printf("error, check_column result %d\n", sum);
    return -1;
  }
}

// this fn is called to check rows
int check_rows_fn(void *coord_rows_void_ptr)
{
  // coord x to GRIDN
  int *coord_rows_ptr = (int *)coord_rows_void_ptr, row[GRIDN] = { 0 }, sum = 0;

  #pragma omp parallel for shared(row)
  for(int i = 0; i < GRIDN; i++){
    printf("%d \n", (*(sudoku_grid[(int)*coord_rows_ptr][i]) - '0') - 1);

    int temp_index = (*(sudoku_grid[(int)*coord_rows_ptr][i]) - '0') - 1;

    //if row has repeated values
    if(row[temp_index] == 1){
      printf("repetead value, index %d \n", temp_index);
    }

    row[temp_index] = 1;
  }

  //check sum
  for(int i = 0; i<GRIDN; i++){
    sum += row[i];
  }

  if(sum == GRIDN){
    //TODO: return success
    printf("check_row result %d\n", sum);
    return 1;
  }else{
    //TODO: return error
    printf("error, check_row result %d\n", sum);
    return -1;
  }
}

// this fn is called to check matrix
int check_matrix_fn(void* coord_x_void_ptr, void* coord_y_void_ptr)
{
  //printf("check matrix \n");
  int mapped_matrix[GRIDN] = { 0 }, sum = 0;

  int *coord_x_ptr = (int *)coord_x_void_ptr;
  int *coord_y_ptr = (int *)coord_y_void_ptr;

  #pragma omp parallel for shared(mapped_matrix)
  for(int i = (int)*coord_y_ptr; i < (int)*coord_y_ptr + 3; i++){
    for(int j = (int)*coord_x_ptr; j < (int)*coord_x_ptr + 3; j++){
      int temp_index = (*(sudoku_grid[i][j]) - '0') - 1;

      if(mapped_matrix[temp_index] == 1){
        //TODO: return error
        printf("repetead value, index %d \n", temp_index);
      }

      mapped_matrix[temp_index] = 1;
    }
  }

  for(int i = 0; i<GRIDN; i++){
    sum += mapped_matrix[i];
  }

  if(sum == GRIDN){
    //TODO: return success
    printf("check_row result %d\n", sum);
    return 1;
  }else{
    //TODO: return error
    printf("error, check_row result %d\n", sum);
    return -1;
  }

  return 1;
}


// this function is run by the second thread 
void *check_columns_thread_fn(void *n){
  for(int i = 0; i<GRIDN; i++){
    check_columns_fn(&i);
  }
  printf("CHECK COLUMNS THREAD FN\n");
  return NULL;
}

// this function is run by the first thread 
void *check_rows_thread_fn(void *n){
  for(int i = 0; i<GRIDN; i++){
    check_rows_fn(&i);
  }
  printf("CHECK ROWS THREAD FN\n");
  return NULL;
}

// this function is run by the first thread 
void *check_matrix_thread_fn(void *n){
  for(int i = 0; i<(int)GRIDN/3; i++){
    for(int j = 0; j<(int)GRIDN/3; j++){
      printf("%d, %d\n", matrix_index[i],matrix_index[j]);
      check_matrix_fn(&matrix_index[i],&matrix_index[j]);
    }
  }
  printf("CHECK MATRIX THREAD FN\n");
  return NULL;
}

int main(int argc, char *argv[])
{
  // check 2 arguments
  if(argc != 2)
    return -1;

  // open file
  if((fd_sudoku = open(argv[1], O_RDONLY)) < 0)
    return -1;

  // info about the file
  if(fstat(fd_sudoku, &s) < 0)
    return -1;

  // memory map to adrress and then into sudoku_grid
  addr_sudoku = (char *)mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd_sudoku, 0);
  for(int i = 0; i<s.st_size; i++){
    sudoku_grid[(int)floor(i/GRIDN)][i%GRIDN] = &addr_sudoku[i];
  }

  // this variable is our reference to the second thread
  pthread_t check_columns_thread;

  // create a second thread which executes check_columns_thread(&colum)
  if(pthread_create(&check_columns_thread, NULL, check_columns_thread_fn, NULL)) {
    fprintf(stderr, "Error creating thread\n");
    // TODO: return error
    return -1;
  }

  check_rows_thread_fn(NULL);

  // wait for the second thread to finish
  if(pthread_join(check_columns_thread, NULL)){
    fprintf(stderr, "Error joining thread\n");
    // TODO: return error
    return -1;
  }

  check_matrix_thread_fn(NULL);

  close(fd_sudoku);
}
