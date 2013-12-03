/***********************************************************

 Copyright Stephen G. Hartke, Derrick Stolee 2013.

 This file is part of SearchLib.

 SearchLib is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SearchLib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY{ return; } without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SearchLib.  If not, see <http://www.gnu.org/licenses/>.

 *************************************************************/

/*
 *
 * distance_cliquer.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: derrickstolee
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

double seconds_per_cayley_call;
clock_t start_time = 0;
clock_t total_time = 1800;

double cur_max_lower = 0; // best lower bound so far
double cur_min_upper = 0; // best upper bound so far
int best_lower_a = 0;
int best_lower_b = 1;
int best_upper_a = 1;
int best_upper_b = 1;

void initCliquerData(int N, int gsize, int* gens);
void freeCliquerData();
int getMaxIndepSizeIntervalUsing(int n);
int getMaxIndepSizeCyclicUsing(int N, int n);

/**
 * distance_cliquer.exe takes a given generator set and tries to find a maximum-density independent set in the distance graph
 * generated by that set. It uses an implementation of the cliquer algorithm.
 *
 * Input:
 *
 * -g # lists the number of generators. MUST appear before "--gens"!!!
 * --gens # # # lists the generators as numbers.
 * -n # gives the minimum value of n to use for testing circulant graphs.
 * -N # gives the maximum value of N to use for testing circulant graphs.
 * -t # gives the maximum number of seconds to spend in each call to the algorithm.
 * --IN # gives the maximum N to test the independence number of an interval. (This is used to help pruning!)
 * It uses a few methods to help.
 */
int main(int argc, char** argv)
{
	seconds_per_cayley_call = 10000000;

	int minN, maxN;
	int num_gens = 0;
	int* gens = 0;

	for ( int i = 0; i < argc; i++ )
	{
		if ( i < argc - 1 && strcmp(argv[i], "-g") == 0 )
		{
			num_gens = atoi(argv[i + 1]);
			gens = (int*) malloc(num_gens * sizeof(int));
			bzero(gens, num_gens * sizeof(int));
		}
		else if ( num_gens > 0 && i < argc - num_gens && strcmp(argv[i], "--gens") == 0 )
		{
			for ( int j = 0; j < num_gens; j++ )
			{
				gens[j] = atoi(argv[i + 1 + j]);
			}
		}
		if ( i < argc - 1 && strcmp(argv[i], "-n") == 0 )
		{
			minN = atoi(argv[i + 1]);
		}
		if ( i < argc - 1 && strcmp(argv[i], "-t") == 0 )
		{
			seconds_per_cayley_call = atof(argv[i + 1]);
		}
		if ( i < argc - 1 && strcmp(argv[i], "-k") == 0 )
		{
			total_time = atof(argv[i + 1]);
		}
		if ( i < argc - 1 && strcmp(argv[i], "-N") == 0 )
		{
			maxN = atoi(argv[i + 1]);
		}
	}

	start_time = time(NULL);

	initCliquerData(2 * maxN + 2, num_gens, gens);

	// Goal: Compute alpha values until the density is BEST...
	int n = 1;

	cur_min_upper = 1.0;
	cur_max_lower = 0.0;

	while ( cur_max_lower < cur_min_upper && n < maxN && (time(NULL) - start_time) < total_time )
	{
		bool is_divisor = false;		
	
		for ( int i = 0; i < num_gens; i++ )
		{
			if ( (gens[i] % n) == 0 )
			{
				is_divisor = true;
			}
		}
		
		if ( is_divisor == false )
		{
			for ( int i = 1; i < n; i++ )
			{
				int val = getMaxIndepSizeCyclicUsing(n, i);

				if ( val < 0 )
				{
					//ended early
					return 0;
				}
			}
			int val = getMaxIndepSizeCyclicUsing(n, n);

			if ( val < 0 )
			{
//ended early
				return 0;
			}

			double alpha = ((double) val) / (double) n;

			if ( alpha > cur_max_lower )
			{
				cur_max_lower = alpha;
				best_lower_a = val;
				best_lower_b = n;
			}

			printf("T MAX ALPHA_CYC_S");
			for ( int i = 0; i < num_gens; i++ )
			{
				printf("_%02d", gens[i]);
			}
			printf("_N_%03d %3d\n", n, val);
		}

		int val = getMaxIndepSizeIntervalUsing(2 * n - 1);
		if ( val < 0 )
		{
//ended early
			break;
		}

		double alpha = ((double) val) / (double) (2 * n - 1);

		if ( alpha < cur_min_upper )
		{
			cur_min_upper = alpha;
			best_upper_a = val;
			best_upper_b = 2 * n - 1;
		}

		printf("T MAX ALPHA_INT_S");
		for ( int i = 0; i < num_gens; i++ )
		{
			printf("_%02d", gens[i]);
		}
		printf("_N_%03d %3d\n", 2 * n - 1, val);

		val = getMaxIndepSizeIntervalUsing(2 * n);

		if ( val < 0 )
		{
//ended early
			break;
		}

		alpha = ((double) val) / (double) (2 * n);

		if ( alpha < cur_min_upper )
		{
			cur_min_upper = alpha;
			best_upper_a = val;
			best_upper_b = 2 * n;
		}

		printf("T MAX ALPHA_INT_S");
		for ( int i = 0; i < num_gens; i++ )
		{
			printf("_%02d", gens[i]);
		}
		printf("_N_%03d %3d\n", 2 * n, val);

		n++;
	}

	// upper bounds using intervals!
	printf("T MIN DALPHA_INT_S");
	for ( int i = 0; i < num_gens; i++ )
	{
		printf("_%02d", gens[i]);
	}
	printf(" %1.10lf # %3d / %3d \n", cur_min_upper, best_upper_a, best_upper_b);

	printf("T MAX DALPHA_CYC_S");
	for ( int i = 0; i < num_gens; i++ )
	{
		printf("_%02d", gens[i]);
	}
	printf(" %1.10lf # %3d / %3d \n", cur_max_lower, best_lower_a, best_lower_b);

	free(gens);
	gens = 0;

	freeCliquerData();

	return 0;
}

int N_size = 0;
int cur_cyclic_N = 0;
int num_gens = 0;
int* generators = 0;
int* alpha_int_table_max = 0; // values of alpha([n],S)
int* alpha_int_table_min = 0; // values of alpha([n],S)
int* alpha_cyc_table_max = 0; // values of alpha(Z_n,S)
int* alpha_cyc_table_min = 0; // values of alpha(Z_n,S)
int* alpha_cyc_values = 0;
int* blocked = 0;

int global_max_size = 0;
int global_goal_size = 0;
int global_N = 0;
int* global_c = 0; // stores the max clique for this size...

void initCliquerData(int N, int gsize, int* gens)
{
	num_gens = gsize;
	generators = gens;

	N_size = 4 * N;
	cur_cyclic_N = 0;

	alpha_int_table_max = (int*) malloc(N_size * sizeof(int));
	alpha_cyc_table_max = (int*) malloc(N_size * sizeof(int));
	alpha_int_table_min = (int*) malloc(N_size * sizeof(int));
	alpha_cyc_table_min = (int*) malloc(N_size * sizeof(int));
	alpha_cyc_values = (int*) malloc(N_size * sizeof(int));

	for ( int n = 0; n < N; n++ )
	{
		// upper bounds!
		alpha_int_table_max[n] = n + 1;
		alpha_cyc_table_max[n] = n + 1;
		alpha_int_table_min[n] = 1;
		alpha_cyc_table_min[n] = 1;
		alpha_cyc_values[n] = 1;
	}

	blocked = (int*) malloc(N_size * sizeof(int));
	for ( int i = 0; i < N_size; i++ )
	{
		blocked[i] = 0;
	}

	cur_max_lower = 0;
	cur_min_upper = 1.0;
}

void freeCliquerData()
{
	free(alpha_int_table_min);
	alpha_int_table_min = 0;

	free(alpha_cyc_table_min);
	alpha_cyc_table_min = 0;

	free(alpha_int_table_max);
	alpha_int_table_max = 0;

	free(alpha_cyc_table_max);
	alpha_cyc_table_max = 0;

	free(blocked);
	blocked = 0;
}

/**
 * computeUpperBound will look at the "blocked" set and find intervals of unblocked elements in 0...m-1
 * If it can determine that the independence numbers of those intervals are below the alpha value for 0...m-1,
 * it will return that value. However, it will return the alpha-value if it is not better.
 */
int computeUpperBound(int m, bool cyclic)
{
	int int_alpha = 0;

	int max_alpha = 0;
	int* alpha_table = 0;

	if ( cyclic )
	{
		alpha_table = alpha_cyc_table_max;
		max_alpha = alpha_cyc_table_max[m - 1];
	}
	else
	{
		alpha_table = alpha_int_table_max;
		max_alpha = alpha_int_table_max[m - 1];
	}

	int first = 0;
	int last = 0;
	while ( int_alpha < max_alpha && first < m )
	{
		while ( first < m && blocked[first] != 0 )
		{
			// increment first until unblocked!
			first++;
		}

		last = first;
		while ( last < m && blocked[last] == 0 )
		{
			last++;
		}

		// ok, compute the alpha-value
		int_alpha += alpha_table[(last - first) - 1]; // value for n is one less

		first = last;
	}

	if ( int_alpha < max_alpha )
	{
		return int_alpha;
	}
	return max_alpha;
}

int num_unblocked = 0;

void skip(int i)
{
	if ( blocked[i] != 0 )
	{
		return;
	}

	num_unblocked--;
	blocked[i] = i + 1;
}

void unskip(int i)
{
	if ( blocked[i] == i + 1 )
	{
		num_unblocked++;
		blocked[i] = 0;
	}
}

void block(int i, bool cyclic)
{
	if ( blocked[i] != 0 )
	{
		return;
	}

	blocked[i] = -(i + 1);
	num_unblocked--;

	for ( int j = 0; j < num_gens; j++ )
	{
		// block BACKWARDS
		int jpos = i - generators[j];
		
		while ( cyclic && jpos < 0) 
		{
			// continue until it is a value mod cyclicN
			jpos += cur_cyclic_N;
		}
		
		
		if ( jpos >= 0 && blocked[jpos] == 0 )
		{
			num_unblocked--;
			blocked[jpos] = i + 1;
		}
	}

	if ( cyclic )
	{
		for ( int j = 0; j < num_gens; j++ )
		{
			// block FORWARDS, MOD N
			int ipjmodn = (i + generators[j]) % cur_cyclic_N;
			if ( ipjmodn < global_N && blocked[ipjmodn] == 0 )
			{
				num_unblocked--;
				blocked[ipjmodn] = i + 1;
			}
		}
	}
}

void unblock(int i, bool cyclic)
{
	if ( blocked[i] != -(i + 1) )
	{
		return;
	}

	blocked[i] = 0;
	num_unblocked++;

	for ( int j = 0; j < num_gens; j++ )
	{
		int jpos = i - generators[j];
		if ( jpos >= 0 && blocked[jpos] == i + 1 )
		{
			num_unblocked++;
			blocked[jpos] = 0;
		}
	}

	if ( cyclic )
	{
		for ( int j = 0; j < num_gens; j++ )
		{
			// block FORWARDS, MOD N
			int ipjmodn = (i + generators[j]) % cur_cyclic_N;
			if ( ipjmodn < global_N && blocked[ipjmodn] == i + 1 )
			{
				num_unblocked++;
				blocked[ipjmodn] = 0;
			}
		}
	}
}

bool cut_out_early = false;
void cliquer(int nu, int size, bool cyclic)
{
	// This is an early-termination condition.
	if ( (time(NULL) - start_time) > total_time )
	{
		cut_out_early = true;
		return;
	}

	bool print_all = false;
	if ( print_all )
	{
		printf("cliquer(%d,%d,_) : ", nu, size);
		for ( int i = 0; i < global_N; i++ )
		{
			if ( blocked[i] == -(i + 1) )
			{
				printf("+");
			}
			else if ( blocked[i] == 0 )
			{
				printf("?");
			}
			else
			{
				printf("(%d)", blocked[i]);
			}
		}
		printf("\n");
	}

	if ( num_unblocked <= 0 )
	{
		if ( size >= global_max_size )
		{
			global_max_size = size;
			if ( global_max_size >= global_goal_size )
			{
				//				global_goal_size = global_max_size + 1; // don't increase, as we know all increments are +0/+1

				if ( !cyclic || cur_cyclic_N == global_N )
				{
					// output a best solution!
					printf("\t(size=%3d) : ", size);
					for ( int i = 0; i < global_N; i++ )
					{
						if ( blocked[i] == -(i + 1) )
						{
							printf("+");
						}
						else if ( blocked[i] == 0 )
						{
							printf("?");
						}
						else
						{
							printf("_");
						}
					}
					printf("\n");
				}
			}
		}
	}
	else
	{
		if ( size + num_unblocked < global_goal_size )
		{
			if ( print_all )
			{
				printf("%d + %d < %d = global_goal_size\n", size, num_unblocked, global_goal_size);
			}
			return;
		}

		// ok, it seems that we CAN continue... but let's check one more thing.

		// find next choice
		int max_i = -1;
		for ( int i = 0; i < global_N; i++ )
		{
			if ( blocked[i] == 0 )
			{
				max_i = i;
			}
		}

		if ( max_i < 0 )
		{
			printf("Found max_i=-1...\n");
			printf("cliquer(%d,%d,_) : ", nu, size);
			for ( int i = 0; i < global_N; i++ )
			{
				if ( blocked[i] == -(i + 1) )
				{
					printf("+");
				}
				else if ( blocked[i] == 0 )
				{
					printf("?");
				}
				else
				{
					printf("_");
				}
			}
			printf("\n\n");
			return;
		}

		if ( size + global_c[max_i] < global_goal_size )
		{
			if ( print_all )
			{
				printf("size + global_c[%d] = %d + %d <= %d = global_goal_size\n", max_i, size, global_c[max_i], global_goal_size);
				for ( int i = 0; i < global_N; i++ )
				{
					if ( blocked[i] == -(i + 1) )
					{
						printf("+");
					}
					else if ( blocked[i] == 0 )
					{
						printf("?");
					}
					else
					{
						printf("_");
					}
				}
				printf("\n");
			}
			return;
		}

		int best_alpha = computeUpperBound(max_i + 1, cyclic);

		if ( size + best_alpha < global_goal_size )
		{
			// then quit!
			if ( print_all )
			{
				if ( cyclic )
				{
					printf("cyclic ");
				}
				else
				{
					printf("       ");
				}
				printf("size + best_alpha = %d + %d <= %d = global_goal_size\n", size, best_alpha, global_goal_size);
				for ( int i = 0; i < global_N; i++ )
				{
					if ( blocked[i] == -(i + 1) )
					{
						printf("+");
					}
					else if ( blocked[i] == 0 )
					{
						printf("?");
					}
					else
					{
						printf("_");
					}
				}
				printf("\n");
			}
			return;
		}

		// ok, put it in the set and see what happens...
		block(max_i, cyclic);
		cliquer(num_unblocked, size + 1, cyclic);
		unblock(max_i, cyclic);

		if ( global_max_size >= global_goal_size )
		{
			// cut out early if we find the +1 increment!
			return;
		}

		// ok, leave it out of the set and see what happens
		skip(max_i);
		cliquer(num_unblocked, size, cyclic);
		unskip(max_i);

		if ( global_max_size >= global_goal_size )
		{
			// cut out early if we find the +1 increment!
			return;
		}
	}
}

int getMaxIndepSizeIntervalUsing(int n)
{
	// I can increase by at most one here!
	if ( n > 1 )
	{
		alpha_int_table_min[n - 1] = alpha_int_table_min[n - 2];
		alpha_int_table_max[n - 1] = alpha_int_table_max[n - 2] + 1;
	}

	global_max_size = alpha_int_table_min[n - 1];
	global_goal_size = alpha_int_table_min[n - 1] + 1; // want ONE MORE VALUE
	global_N = n;
	global_c = alpha_int_table_min;

	// Run the cliquer algorithm...

	cut_out_early = false;
	for ( int i = 0; i < n; i++ )
	{
		blocked[i] = 0;
	}
	num_unblocked = n;

	// force inclusion of the last possible element!
	block(n - 1, false);
	cliquer(num_unblocked, 1, false);
	unblock(n - 1, false);

	if ( cut_out_early )
	{
		return -1;
	}

	alpha_int_table_min[n - 1] = global_max_size;
	alpha_int_table_max[n - 1] = global_max_size;

	return global_max_size;
}

int getMaxIndepSizeCyclicUsing(int N, int n)
{
	if ( cur_cyclic_N != N )
	{
		// reset everything!
		for ( int i = 0; i < N; i++ )
		{
			alpha_cyc_table_max[i] = i + 1;
			alpha_cyc_table_min[i] = 1;
		}

		cur_cyclic_N = N;
	}

	if ( alpha_cyc_table_min[n - 1] == alpha_cyc_table_max[n - 1] )
	{
		if ( n == N )
		{
			alpha_cyc_values[n - 1] = alpha_cyc_table_max[n - 1];
		}
		return alpha_cyc_table_max[n - 1];
	}

	for ( int i = 0; i < n; i++ )
	{
		blocked[i] = 0;
	}
	num_unblocked = n;

	// I can increase by at most one here!
	if ( n > 1 )
	{
		alpha_cyc_table_min[n - 1] = alpha_cyc_table_min[n - 2];
		alpha_cyc_table_max[n - 1] = alpha_cyc_table_max[n - 2] + 1;
	}

	global_max_size = alpha_cyc_table_min[n - 1];
	global_goal_size = alpha_cyc_table_min[n - 1] + 1; // want THIS size...

	global_N = n;
	cur_cyclic_N = N;
	global_c = alpha_cyc_table_min;

	// Run the cliquer algorithm... CYCLIC MODE!
	block(n - 1, true);
	cliquer(num_unblocked, 1, true);
	unblock(n - 1, true);

	alpha_cyc_table_min[n - 1] = global_max_size;
	alpha_cyc_table_max[n - 1] = global_max_size;

	if ( n == N )
	{
		alpha_cyc_values[n - 1] = alpha_cyc_table_max[n - 1];
	}

	return global_max_size;
}

