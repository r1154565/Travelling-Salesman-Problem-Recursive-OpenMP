#include <omp.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>

int n;
int p;
double** adjacencyMatrix;

int startVertex;
int startVertexBitMask;
double globalMinCost = 1e18;
#define CLK CLOCK_MONOTONIC
struct timespec diff(struct timespec start, struct timespec end){
	struct timespec temp;
	if((end.tv_nsec-start.tv_nsec)<0){
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	}
	else{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

double tsp_dp(int currentVertex, int currentBitmask)
{
	if(currentBitmask == (1 << n) - startVertexBitMask)
		return adjacencyMatrix[currentVertex][startVertex];
	double minAns = 1e18;
	int nextVertex;
	for(nextVertex = 0; nextVertex < n; nextVertex++)
	{
		if(currentVertex != nextVertex && ((currentBitmask & (1 << nextVertex)) == 0))
		{
			double tempAns = adjacencyMatrix[currentVertex][nextVertex] + tsp_dp(nextVertex, currentBitmask | (1 << nextVertex));
			if(tempAns < minAns)
				minAns = tempAns;
		}
	}
	return minAns;
}

int main(int argc, char* argv[])
{

	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;

	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3)
	{
		printf( "Usage: %s n p \n", argv[0] );
		return -1;
	}

	n = atoi(argv[1]);	/* Number of cities */
	p = atoi(argv[2]);	/* Number of processors */

	int i, j;
	adjacencyMatrix = malloc(n * sizeof(double *));
	
	for(i = 0; i < n; i++) 
	{
		adjacencyMatrix[i] = malloc(n * sizeof(double));
		for(j = 0; j < n; j++)	
			scanf("%lf", &adjacencyMatrix[i][j]);
	}

	startVertex = 0;
	startVertexBitMask = 1 << startVertex;
	char *problem_name = "travelling_salesman_problem";
	char *approach_name = "dp";

  	char outputFileName[50];		
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);	

	clock_gettime(CLK, &start_alg);	/* Start the algo timer */
	double start_time = omp_get_wtime();

	/*----------------------Core algorithm starts here----------------------------------------------*/
	#pragma omp parallel private(i) num_threads(p)
	{
		int id = omp_get_thread_num();
		double minAns = 1e18;
		for(i = id; i < n; i += p) 
		{
			if(i != startVertex && (startVertexBitMask & (1 << i)) == 0) 
			{
				double tempAns = adjacencyMatrix[startVertex][i] + tsp_dp(i, startVertexBitMask | (1 << i));
				if(tempAns < minAns) 
					minAns = tempAns;
			}
		}
		#pragma omp critical
		{
			if(minAns < globalMinCost)
				globalMinCost = minAns;
		}
	}
	/*----------------------Core algorithm finished--------------------------------------------------*/
	// printf("Minimum Path: %lf\n", globalMinCost);
	double end_time = omp_get_wtime();
	// printf("%lf\n", end_time - start_time);
	clock_gettime(CLK, &end_alg);	/* End the algo timer */

	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);
	// printf("Total Cities:%d \nNumber of processors used:%d \nMinimum cost Hamiltonian Cycle:%lf \n
	// 	Time taken by serial code:%lf\n", n, p, globalMinCost, end_time - start_time);
	// printf("Following cycle has the minimum cost\n");
	// printArray(globalMinPath);
	return 0;
}
