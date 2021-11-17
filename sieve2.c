/*
 *   Sieve of Eratosthenes
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification: 7 September 2001
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)  ((a)<(b)?(a):(b))

int main (int argc, char *argv[])
{
   unsigned long int    count;        /* Local prime count */
   double elapsed_time; /* Parallel execution time */
   unsigned long int    first;        /* Index of first multiple */
   unsigned long int    prime_first;
   int   local_first;
   unsigned long int    global_count = 0; /* Global prime count */
   unsigned long long int    high_value;   /* Highest value on this proc */
   unsigned long int    i;
   int    id;           /* Process ID number */
   unsigned long int    index;        /* Index of current prime */
   unsigned long long int    low_value;    /* Lowest value on this proc */
   char  *marked;       /* Portion of 2,...,'n' */
   char  *local_prime_marked;
   unsigned long long int    n;            /* Sieving from 2, ..., 'n' */
   int    p;            /* Number of processes */
   unsigned long int    proc0_size;   /* Size of proc 0's subarray */
   unsigned long int    prime;
   unsigned long int  local_prime;        /* Current prime */
   unsigned long int    size;         /* Elements in 'marked' */
   unsigned long int  local_prime_size;


   MPI_Init (&argc, &argv);

   /* Start the timer */

   MPI_Comm_rank (MPI_COMM_WORLD, &id);
   MPI_Comm_size (MPI_COMM_WORLD, &p);
   MPI_Barrier(MPI_COMM_WORLD);
   elapsed_time = -MPI_Wtime();

   if (argc != 2) {
      if (!id) printf ("Command line: %s <m>\n", argv[0]);
      MPI_Finalize();
      exit (1);
   }

   n = atoll(argv[1]);

   /* Figure out this process's share of the array, as
      well as the integers represented by the first and
      last array elements */

   /* Add you code here  */

   low_value = 3 +  2 * (id*((n/2)-1)/p);
   high_value = 1 + 2 * ((id+1)*((n/2)-1)/p);
   size = ((high_value - low_value)/2) + 1;
   
   // the primes whose multiples we have to find and mark
   // will be within root n
   // just like for 100 it ll be within 10.. its enough to mark the 
   // mltiples of 2 3 5 7. Rest of the primes ll just remain unmarked
   local_prime_size = (long long) sqrt((double) n);


   proc0_size = ((n/2)-1)/p;

   if (( (2 * proc0_size) + 3) < (long long) sqrt((double) n)/2) {
      if (!id) printf ("Too many processes\n");
      MPI_Finalize();
      exit (1);
   }

   /* Allocate this process's share of the array. */

   marked = (char *) malloc (size);
	
   // all the processed find the required primes as we are not using broadcast
   // required primes are within root n
   local_prime_marked = (char *) malloc (local_prime_size);
   
   if (marked == NULL || local_prime_marked == NULL) {
      printf ("Cannot allocate enough memory\n");
      MPI_Finalize();
      exit (1);
   }

   for (i = 0; i < size; i++) 
	   marked[i] = 0;

   for (i = 0; i < local_prime_size; i++) 
	   local_prime_marked[i] = 0;   

   // markings in the first root n numbers done by all processors
   // as we are not using Broadcast
   // AS BROADCASTING IS COSTLY
   prime = 3;
   do {
      if (prime * prime > low_value)
         first = (prime * prime - low_value)/2;
      else {
         if (!(low_value % prime)) first = 0;
         else  {
            first = prime - (low_value % prime);
            if ( (low_value + first)%2 == 0  ) {
               first = first + prime;
            }
            first = first / 2 ;
         }
      }

      prime_first = (prime * prime -3)/2; 
      
	  // usual marking
      for (i = first; i < size; i += prime) 
		  marked[i] = 1;
	  
	  // All processors marking the numbers in first root n
      for (i = prime_first; i<local_prime_size; i += prime) 
		  local_prime_marked[i] = 1;  


      while (local_prime_marked[++index]);
         prime = 2 * index + 3;

   } while (prime * prime <= n);

   count = 0;
   global_count = 0;
   for (i = 0; i < size; i++)
      if (!marked[i]) count++;

   MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,
                          0, MPI_COMM_WORLD);


   /* Stop the timer */

   elapsed_time += MPI_Wtime();


   /* Print the results */

   if (!id) {
      printf("The total number of prime: %ld, total time: %10.6f, total node %d\n", global_count, elapsed_time, p);

   }
   MPI_Finalize ();
   return 0;
}

