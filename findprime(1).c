#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)	((a)<(b)?(a):(b))
int block_low(int id,int p,int n)
{
	int low;
	low=(id*n)/p;
	return low;
}
int block_high(int id,int p,int n)
{
	int high;
	high=((id+1)*n)/p-1;
	return high;
}
int block_size(int id,int p,int n)
{
	int size;
	size=block_high(id,p,n)-block_low(id,p,n)+1;
	return size;
}
int main(int argc,char *argv[])
{
	int id,p,n;
	int low_value,high_value;
	int size,proc0_size;
	int index;
	int prime;
	int first;
	int i;
	int count,global_count;
	char* marked;
	double elapsed_time;
	MPI_Init(&argc,&argv);
	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time= -MPI_Wtime();
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	if (argc !=2){
		if (!id)printf ("Command line: %s <m>\n",argv[0]);
		MPI_Finalize();
	}
	n=atoi(argv[1]);
	low_value = 2+block_low(id,p,n-1);
	high_value = 2+block_high(id,p,n-1);
	size = block_size(id,p,n-1);
	proc0_size = (n-1)/p;
	if ((2+proc0_size)<(int)sqrt((double)n))
	{
		if (!id) printf("Too many processes\n");
		MPI_Finalize();
		exit(1);
	}
	marked=(char *)malloc(size);
	if(marked==NULL)
	{
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		exit(1);
	}
	for (i=0;i<size;i++)
	{
		marked[i]=0;
	}
	if(!id)
		index=0;
	prime=2;
	do
	{
		if(prime*prime>low_value)
			first=prime*prime-low_value;
		else
		{
			if(!(low_value%prime))
				first=0;
			else
				first=prime-(low_value%prime);
		}
		for (i=first;i<size;i+=prime)
			marked[i]=1;
		if(!id)
		{
			while(marked[++index]);
			prime=index+2;
		}
		MPI_Bcast(&prime,1,MPI_INT,0,MPI_COMM_WORLD);
	}while(prime*prime<=n);
	count=0;
	for (i=0;i<size;i++){
		if(!marked[i])
			count++;
	}
	MPI_Reduce(&count,&global_count,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	elapsed_time+=MPI_Wtime();
	if(!id){
		printf("%d primes are less than or equal to %d\n",global_count,n);
		printf("Total elapesd time: %10.6f\n",elapsed_time);
	}
	MPI_Finalize();
	return 0;
	
}
