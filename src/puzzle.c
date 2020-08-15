#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>


/**
 * Global Constant
 */

/**
 * The id of the four available actions for moving the blank (empty slot). e.x.
 * Left: moves the blank to the left, etc.
 */

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

/** The number of tiles in the puzzle */
#define MAX_TILES 16

/* The width or height of puzzle */
#define N 4

/* meaning a variable is unused */
#define UNUSED -1
/**
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[MAX_TILES];
	int g;
	int f;
} node;

/**
 * Global Variables
 */

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;

// this is a sum of manhattan distance, used to prune the search space
// this is the result of heuristic function, estimating the minimum solution moves
// When we apply a heuristic function, sum_manhattan is updated
int sum_manhattan;

// used to store the minimum cost that exceed current threshold
// becomes the next threshold for the next iteration
int new_threshold;

// this is the threshold for current iteration
// value will be replaced by new threshold if estimated optimal moves is exceeded
int threshold;

// Initial node of the problem
node initial_node;

// Statistics about the number of generated and expendad nodes
unsigned long generated;
unsigned long expanded;

/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s )
{
	int i;

	for( i = 0; i < MAX_TILES; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}

void printf_comma( long unsigned int n )
{
    if (n < 1000)
	{
        printf ("%lu", n);
        return;
    }
    printf_comma (n/1000);
    printf (",%03lu", n%1000);
}

/* return the sum of manhattan distances from state to goal */
/* this is only used once to calculate the base heuristic */
void calculate_manhattan( int* state )
{
	// for looping
	int i;

	// Coordinates of current tile
	int x, y;

	// Original coordinates where the tile should be
	int ox, oy;

	for(i = 0; i < MAX_TILES; i++)
	{

		// if tile is a blank, ignore it and continue next loop
	 	if(state[i] == 0) { continue; }

		// calculate the position of tile
		x = i % N;
		y = i / N;
		// calculate original position of tile
		ox = state[i] % N;
		oy = state[i] / N;

		// sum up all manhattan distance
		sum_manhattan += (abs(x - ox) + abs(y - oy));
	}
}

// calculate change in heuristic when moves are applied, update sum_manhattan
// this function is used throughout the search
void change_manhattan( int* state, int op )
{
	// The tile position in the puzzle
    int	tile_num =  blank_pos + (op == LEFT ? -1 : (op == RIGHT ? 1 : (op == UP ? -4 : 4)));

	// Coordinates of the current tile
	int x = tile_num % N;
	int y = tile_num / N;

	// Orignial coordinates of the tile
	int ox = state[tile_num] % N;
	int oy = state[tile_num] / N;

	// Minus the manhattan distance of tile with old position
	sum_manhattan -= (abs(x - ox) + abs(y - oy));

	// Calculate the manhattan distance of tile with new position
	tile_num = blank_pos;
	x = tile_num % N;
	y = tile_num / N;

	// Add the manhattan distance of tile with new position
	sum_manhattan += (abs(x - ox) + abs(y - oy));
}

// return the minimum number between new_threshold recorded and evaluation function
int min( int node_f )
{
	if(node_f > new_threshold){ return new_threshold; }
	else{ return node_f; }
}

/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
       	return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == LEFT ? -1 : (op == RIGHT ? 1 : (op == UP ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;

	//update blank pos
	blank_pos = t;
}

// Reverse the move applied to node, for the next move loop to be applied
void reverse_move( node* n, int op )
{
	// move to be applied
	int move;

	// check which move to be applied according to previous move
	if(op == LEFT){ move = RIGHT; }
	if(op == RIGHT){ move = LEFT; }
	if(op == UP){ move = DOWN; }
	if(op == DOWN){ move = UP; }

	// update sum_manhattan because move is reversed
	change_manhattan(n->state, move);

	// update node by swapping tiles
	apply(n, move);

	// minus cost by 1 because move is reversed
	n->g = n->g - 1;

	// update evaluation function with previous manhattan distance and previous cost
	n->f = n->g + sum_manhattan;
}

// IDA*(n, B, B') [from pseudocode]
/* there's no B(threshold) and B'(new threshold) because they are global variables */
/* there is an int argument (last_move) to record the previous move applied
    and is used for preventing inverse move being applied for current ida search */
struct node* ida( struct node* node, int last_move )
{
	// for looping moves
	int move_num;

	// to track whether we have apply any move in the current ida* function
	int previous_move = UNUSED;

	/* storing move applied in the current ida* function for passing it to
	    next ida* function */
	int move_applied;

	struct node* r;

	// if tiles are all in correct position, return node
	if(sum_manhattan == 0){ return node; }

	// for a in A(n.s) [from pseudocode]
	for(move_num = 0; move_num < 4; move_num++)
	{
		// if there's a move applied previously, reverse the node to previous version
		if(previous_move != -1){ reverse_move(node, previous_move); previous_move = UNUSED; }

		// prevent inverse movement for being applied
		if(last_move == LEFT){ if (move_num == RIGHT) continue; }
		if(last_move == RIGHT){ if (move_num == LEFT) continue; }
		if(last_move == UP){ if (move_num == DOWN) continue; }
		if(last_move == DOWN){ if (move_num == UP) continue; }

		if(applicable( move_num ))
		{
			// store the movement applied to reverse the move when looping
			previous_move = move_num;

			generated++;

			// update sum of manhattan with new move
			change_manhattan( node->state, move_num );

			// do n'.s <-- f(a, n.s) [from pseudocode]
			apply( node, move_num );

			// n'.g <-- n.g + 1 [from pseudocode]
			// n'.f <-- n'.g + h(n'.s) [from pseudocode]
			node->f = ++node->g + sum_manhattan;

			// if n'.f > B [from pseudocode]
			if(node->f > threshold)
			{
				// then B' <-- min(n'.f, B') [from pseudocode]
				new_threshold = min( node->f );
			// else [from pseudocode]
			}else
			{
				// if h(n'.s) = 0 [from pseudocode]
				if(sum_manhattan == 0)
				{
					// then return n' [from pseudocode]
					return node;
				}

				r = NULL;
				move_applied = move_num;
				expanded++;

				// r <-- IDA*(n', B, B') [from pseudocode]
				r = ida( node, move_applied );

				// if r != NIL [from pseudocode]
				if(r != NULL)
				{
					// then return r [from pseudocode]
					return r;
				}
			}
		}
	}

	/* reverse the node to previous version if there's any move applied before return NULL */
	if(previous_move != -1){ reverse_move( node, previous_move ); previous_move = UNUSED; }
	// return NIL [from pseudocode]
	return NULL;
}

// IDA - Control- Loop() [from pseudocode]
int ida_control_loop()
{
	struct node* r = NULL;

	generated = 0;
	expanded = 0;

	// for looping each tile
	int i;

	// calculate the base heuristic, only used once
	calculate_manhattan( initial_node.state );

	// B <-- h(s0) [from pseudocode]
	initial_node.f = threshold = sum_manhattan;

	printf("Initial Estimate = %d\n", initial_node.f);
	printf("Threshold = %d ", initial_node.f);

	// repeat [from pseudocode]
	while( r == NULL )
	{
		// B' <-- INIFINITY [from pseudocode]
		new_threshold = INT_MAX;

		// n.s <-- s0 [from pseudocode]
		struct node temp;

		// reset the node
		for(i = 0; i < MAX_TILES; i++)
		{
			if(initial_node.state[i] == 0)
			{
				blank_pos = i;
			}
			temp.state[i] = initial_node.state[i];
		}

		// n.g <-- 0 [from pseudocode]
		temp.g = 0;
		temp.f = threshold;

		// r <-- IDA*(n, B, B') [from pseudocode]
		r = ida( &temp, UNUSED );

		// if r = NIL [from pseudocode]
		if(r == NULL)
		{
			// then B <-- B' [from pseudocode]
			threshold = new_threshold;
			printf("%d ", threshold);
		}
	}

	// until r != NIL [from pseudocode]
	// return r.g [from pseudocode]
	if(r) { return r->g;}
	else { return UNUSED;}
}

static inline float compute_current_time()
{
	struct rusage r_usage;

	getrusage( RUSAGE_SELF, &r_usage );
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}

int main( int argc, char **argv )
{
	int i, solution_length;

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
			{
				initial_node.state[i] = atoi( tile );
				blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
				tile = strtok( NULL, " " );
			}
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}

	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;
	printf("\n");
	printf("Initial State:\n");
	print_state( initial_node.state );

	/* solve */
	float t0 = compute_current_time();

	solution_length = ida_control_loop();

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);
	printf("\nExpanded = ");
	printf_comma(expanded);
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");

	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSoulution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);
	fclose(initFile);

	return( 0 );
}
