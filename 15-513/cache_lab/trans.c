/* 
 * Name:Hongyi Liang
 * Andrew ID: hongyil
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
 * A is the source matrix, B is the destination
 * tmp points to a region of memory able to hold TMPCOUNT (set to 256) doubles as temporaries
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 2KB direct mapped cache with a block size of 64 bytes.
 *
 * Programming restrictions:
 *   No out-of-bounds references are allowed
 *   No alterations may be made to the source array A
 *   Data in tmp can be read or written
 *   This file cannot contain any local or global doubles or arrays of doubles
 *   You may not use unions, casting, global variables, or 
 *     other tricks to hide array data in other forms of local or global memory.
 */ 
#include <stdio.h>
#include <stdbool.h>
#include "cachelab.h"
#include "contracts.h"

/* Forward declarations */
bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N]);
void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";


void transpose_submit(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    /*
     * This is a good place to call your favorite transposition functions
     * It's OK to choose different functions based on array size, but
     * your code must be correct for all values of M and N
     */

    /* 
    *  
    *cache parameters:
    *s=5;sets=2**5=32 sets
    *E=1
    *b=6;block size=2**6=64 bits
    *
    */

    int block;       //block size or the partition of matrix                   
    size_t i,j;      //outer row/col index
    size_t row,col;  //inner block row/col index

    //case 32*32
    if(M==32 && N==32){

        //8*8 blocking        
        block=8;        
        
        //loop over blocks
        for (i=0;i<M-7;i+=block){
            for (j=0;j<N-7;j+=block){
                //loop over inner block
                for (row=i;row<i+block;row++){
                    //diagonal elements,in case double count
                    if (i==j){
                        B[row][row]=A[row][row];
                    }
                    for (col=j;col<j+block;col++){
                        if(row!=col){
                            B[row][col]=A[col][row];
                        }
                    }
                }
            }
        }
    }else if (M==64 && N==64){

        int di,dj,dk,dl;
        //8*8 blocking
        block=8;

        //loop over outer block and divide thems into 4*4 blocks
        for(i=0;i<N;i+=block){
            for(j=0;j<M;j+=block){
                if (i==j){
                    //find empty place to temporal store data
                    if (j==0){
                        dl=72;
                        dk=8;
                    }else{
                        dl=64;
                        dk=0;
                    }             

                    /*
                    *
                    * loop over each 4*4 blocks,note that the order can significantly 
                    * affect cpu cycles.
                    *
                    * be careful to figure which elements will count multiple times.
                    *
                    * main point:try to ensure that the next "for-loop" will use the 
                    * same rows as that in its previous "for-loop"
                    * 
                    */

                    //starts from block[0][0]
                    for(row=i;row<i+4;row++){
                        for(col=j;col<j+4;col++){
                            di=row%4;
                            dj=col%4;
                            //store temporal data
                            tmp[4*di+dj+dk]=A[row][col];
                        }
                    }  

                    //load block[0][1]
                    for(row=i;row<i+4;row++){
                        for(col=j+4;col<j+8;col++){
                            di=row%4;
                            dj=col%4;
                            //store block[0][1] data (and in cache)
                            tmp[4*di+dj+dl]=A[row][col];
                        }
                    }                    

                    //block[1][0]
                    //load cache(tmp) data to block[1][0] in B
                    for(row=j+4;row<j+8;row++){
                        for(col=i;col<i+4;col++){
                            di=col%4;
                            dj=row%4;
                            //store the cache(tmp) data to destination
                            B[row][col]=tmp[4*di+dj+dl];
                        }
                    }                    

                    //block[1][0]
                    for(row=i+4;row<i+8;row++){
                        for(col=j;col<j+4;col++){
                            di=row%4;
                            dj=col%4;
                            //store data
                            tmp[4*di+dj+dl]=A[row][col];
                        }
                    }                    

                    //block[1][1] in B
                    //the row has been loaded to cache in last step(block[1][])
                    for(row=j+4;row<j+8;row++){
                        //store diagonal elements,in case double count
                        B[row][row]=A[row][row];
                        for(col=i+4;col<i+8;col++){
                            if (row!=col){
                                B[row][col]=A[col][row];
                            }
                        }
                    }                   
                    
                    //load data to block[0][1] in B
                    for(row=j;row<j+4;row++){
                        for(col=i+4;col<i+8;col++){
                            di=col%4;
                            dj=row%4;
                            B[row][col]=tmp[4*di+dj+dl];
                        }
                    }
                    
                    //load data to block[0][0] in B
                    for(row=j;row<j+4;row++){
                        for(col=i;col<i+4;col++){
                            di=col%4;
                            dj=row%4;
                            B[row][col]=tmp[4*di+dj+dk];
                        }
                    }
                }else{
                    //the top right and down left blocks
                    //empty space to store data
                    if(i==0){
                        dl=8;
                    }else{
                        dl=0;
                    }

                    //starts from block [0][0]
                    //load data to block[0][0] in B
                    for(row=i;row<i+4;row++){
                        for(col=j;col<j+4;col++){
                            B[row][col]=A[col][row];
                        }
                    }
                    
                    //blcok[0][1],same row
                    for(row=j;row<j+4;row++){
                        for(col=i+4;col<i+8;col++){  
                            di=row%4;
                            dj=col%4;
                            //store data (also in cache)
                            tmp[4*di+dj+dl]=A[row][col];
                        }
                    }
                    
                    //load data to block[0][1] in B
                    for(row=i;row<i+4;row++){
                        for(col=j+4;col<j+8;col++){
                            if (row!=col){
                                B[row][col]=A[col][row];
                            }
                        }
                    }

                    //load data to block[1][1] in B
                    for(row=i+4;row<i+8;row++){
                        for(col=j+4;col<j+8;col++){
                            if (row!=col){
                                B[row][col]=A[col][row];
                            }
                        }
                    }
                    
                    //load data to block[1][0] in B
                    for(row=i+4;row<i+8;row++){
                        for(col=j;col<j+4;col++){
                            di=col%4;
                            dj=row%4;
                            B[row][col]=tmp[4*di+dj+dl];
                        }
                    }
                }
            }
        }       
    }else if (M==63 && N==65){

        //blocking 4*4 
        block=4;

        for (i=0;i<M;i+=block){
            for (j=0;j<N;j+=block){
                for (col=j;col<(j+block)&&(col<N);col++){
                    for (row=i;row<(i+block)&&(row<M);row++){
                        //diagonal element:in case double count
                        if (row==col){
                            if (i==j){
                                B[col][col]=A[row][row];
                            }
                        }else{
                            B[row][col]=A[col][row];
                        }
                    }   
                }
            }
        }
    }else if (M == N){  //other trivival cases
        trans(M, N, A, B, tmp);
    }else{
        trans_tmp(M, N, A, B, tmp);
    }

    return;
}
/* 
 * You can define additional transpose functions below. We've defined
 * some simple ones below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";

/*
 * The following shows an example of a correct, but cache-inefficient
 *   transpose function.  Note the use of macros (defined in
 *   contracts.h) that add checking code when the file is compiled in
 *   debugging mode.  See the Makefile for instructions on how to do
 *   this.
 *
 *   IMPORTANT: Enabling debugging will significantly reduce your
 *   cache performance.  Be sure to disable this checking when you
 *   want to measure performance.
 */
void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    size_t i, j;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            B[j][i] = A[i][j];
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * This is a contrived example to illustrate the use of the temporary array
 */

char trans_tmp_desc[] =
    "Simple row-wise scan transpose, using a 2X2 temporary array";

void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    size_t i, j;
    /* Use the first four elements of tmp as a 2x2 array with row-major ordering. */
    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int di = i%2;
            int dj = j%2;
            tmp[2*di+dj] = A[i][j];
            B[j][i] = tmp[2*di+dj];
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_tmp, trans_tmp_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N])
{
    size_t i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return false;
            }
        }
    }
    return true;
}

