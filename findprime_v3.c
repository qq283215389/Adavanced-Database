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
int isprime(int p)
{
	if(p%2==0)
	{
		return 0;
	}
	int a=3;
	while(a<(int)sqrt(p))
	{
		if(p%a==0)
			return 0;
		else
			a=a+2;
	}
	return 1;
}
int main(int argc,char *argv[])
{
	int id; //进程编号
	int p;	//总的进程数
	int n0,n;	//质数上限
	int low_value;	//本进程负责处理数据块的开端
	int high_value;	//本进程负责处理数据块的末端
	int size;	//本进程负责处理数据块的大小
	int index;
	int prime;
	int first;
	int i;
	int count; //本进程的质数数量
	int global_count;	//总的质数数量
	char* marked;	//标记质数与非质数
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
	n0=atoi(argv[1]);
	if (n0%2==0)
	{
		n=n0/2-1;
	}
	else
		n=(n0-1)/2;
	low_value = block_low(id,p,n);	
	high_value = block_high(id,p,n);
	//printf("low value: %d  I am %d of %d\n", low_value, id, p);
	//printf("high value: %d  I am %d of %d\n", high_value, id, p);
	size = block_size(id,p,n);
	//printf("size: %d  I am %d of %d\n", size, id, p);
	int proc0_size = (n-1)/p;	//进程0负责处理数据块的大小

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
	prime=3;
	do
	{	
		int tmp=(low_value+2)*2-1;
		//printf("Prime value: %d  I am %d of %d\n", prime, id, p);
		if(prime*prime>tmp)
			first=prime*prime/2-1-low_value;
		else
		{
			if(!(tmp%prime))
				first=0;
			else
			{
				int t=tmp/prime;
				if(t%2==0)
					first=(prime*(t+1))/2-1-low_value;
				else
					first=(prime*(t+2))/2-1-low_value;
			}
		}
		for (i=first;i<size;i+=prime)
		{
			marked[i]=1;
			//printf("Marked value: %d  I am %d of %d\n", i, id, p);
		}
			
		while(!isprime(++prime));
	}while(prime*prime<=n0);
	count=0;
	for (i=0;i<size;i++){
		if(!marked[i])
			count++;
	}
	MPI_Reduce(&count,&global_count,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	elapsed_time+=MPI_Wtime();
	if(!id){
		printf("%d primes are less than or equal to %d\n",global_count+1,n0);
		printf("Total elapesd time: %10.6f\n",elapsed_time);
	}
	MPI_Finalize();
	return 0;
	
}
