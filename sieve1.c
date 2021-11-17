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
   unsigned long int    global_count = 0; /* Global prime count */
   unsigned long long int    high_value;   /* Highest value on this proc */
   unsigned long int    i;
   int    id;           /* Process ID number */
   unsigned long int    index;        /* Index of current prime */
   unsigned long long int    low_value;    /* Lowest value on this proc */
   char  *marked;       /* Portion of 2,...,'n' */
   unsigned long long int    n;            /* Sieving from 2, ..., 'n' */
   int    p;            /* Number of processes */
   unsigned long int    proc0_size;   /* Size of proc 0's subarray */
   unsigned long int    prime;        /* Current prime */
   unsigned long int    size;         /* Elements in 'marked' */


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
   /* Stop the timer */

   /* Add you code here  */
   
   low_value = 3 +  2 * (id*((n/2)-1)/p);
   high_value = 1 + 2 * ((id+1)*((n/2)-1)/p);
   size = ((high_value - low_value)/2) + 1;
   
   printf("\n siz of the proc:%d is :%lld",id, size);
   printf("\n low_value of the proc:%d is :%lld",id, low_value);
   printf("\n high_value of the proc:%d is :%lld",id, high_value);

   proc0_size = ((n/2)-1)/p;

   // Checking if the to be identified primes are in the first processor
   // for example: in case of 100 - to be identifies and used to for sieving are 
   // 2 3 5 7. We have to find the multiples of these only. Rest of the automatically 
   // ll be unmarked
   if (( (2 * proc0_size) + 3) < (long long) sqrt((double) n)) {
      if (!id) printf ("Too many processes\n");
      MPI_Finalize();
      exit (1);
   }

   // Per processor's share of marked array
   marked = (char *) malloc (size);

   if (marked == NULL) {
      printf ("Cannot allocate enough memory\n");
      MPI_Finalize();
      exit (1);
   }

   for (i = 0; i < size; i++) marked[i] = 0;
   if (!id) index = 0;
   prime = 3;
   do {
      if (prime * prime > low_value)
         first = (prime * prime - low_value)/2;
      else {
         if (!(low_value % prime)) first = 0;
         else  {
            first = prime - (low_value % prime);
            if ( (low_value + first) % 2 == 0  ) {
               first = first + prime;
            }
            first = first / 2 ;
         }
      }
      for (i = first; i < size; i += prime) marked[i] = 1;
      if (!id) {
         while (marked[++index]);
         prime = 2 * index + 3;
      }
	  // Required primes broadcasted by processor 1 to all the processors
      if (p > 1) 
		  MPI_Bcast (&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
	  
   } while (prime * prime <= n);
   
   count = 0;
   for (i = 0; i < size; i++)
      if (!marked[i]) 
		  count++;
   printf("\ncount of primes in %d: %lld", id,count);
   if (p > 1) MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM,
       0, MPI_COMM_WORLD);

   elapsed_time += MPI_Wtime();


   if (!id) 
   {
	   global_count++; // To include 2
      printf("\nThe total number of prime: %d, total time: %10.6f, total node %d\n", global_count, elapsed_time, p);
   }
   MPI_Finalize();
   return 0;

}

