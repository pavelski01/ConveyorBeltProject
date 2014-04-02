#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define NOVAL -1
#define ASSERTNOERRPTR(x, msg) do { if ((x) == NULL) { perror(msg); exit(EXIT_FAILURE); } } while(NULL)
#define ASSERTNOERRVAL(x, msg) do { if ((x) == NOVAL) { perror(msg); exit(EXIT_FAILURE); } } while(NULL)
#define PIPE_READ 0
#define PIPE_WRITE 1

void calculate(int, char**);
void constant_statement(int, char**);
void linear_statement(int, char**);
void polynomial_statement(int, int, char**, int, int);
void prelude(int, char**);

int main(int argc, char** argv)
{
	prelude(argc, argv);
	calculate(argc, argv);
	exit(EXIT_SUCCESS);
}

void calculate(int argument_counter, char** argument_vector)
{
	if (argument_counter == 2) constant_statement(argument_counter, argument_vector);
	else if (argument_counter == 3) linear_statement(argument_counter, argument_vector);
	else polynomial_statement(argument_counter - 4, argument_counter, argument_vector, NOVAL, NOVAL);
}

void constant_statement(int argument_counter, char** argument_vector)
{
	FILE* file_pointer = fopen("./data.dat", "r"); 
	ASSERTNOERRPTR(file_pointer, "Can't open file!");
	long result = 0;
	int counter = 0;
	while (fscanf(file_pointer, "%ld", &result) == 1)
	{
		++counter;
		printf("\tx%-3d = %8ld, \tf( x%-3d) = %8ld \n", counter, result, 
			counter, (long) atoi(argument_vector[argument_counter - 1]));
	}
}

void linear_statement(int argument_counter, char** argument_vector)
{
	FILE* file_pointer = fopen("./data.dat", "r"); 
	ASSERTNOERRPTR(file_pointer, "Can't open file!");
	long result[2] = {0, 0};
	int counter = 0;
	while (fscanf(file_pointer, "%ld", &result[0]) == 1)
	{
		++counter;
		result[1] = result[0] * atoi(argument_vector[argument_counter - 2]) 
			+ atoi(argument_vector[argument_counter - 1]);
		printf("\tx%-3d = %8ld, \tf( x%-3d) = %8ld \n", counter, result[0], counter, result[1]);
	}
}

void polynomial_statement(int level, int argument_counter, char** argument_vector, 
	int descriptor_relay, int descriptor_receiver)
{
	long result[2] = {0, 0};
	int descriptor_awakening[2];
	int comparison_value = pipe(descriptor_awakening);
	ASSERTNOERRVAL(comparison_value, "Pipeline error!");
	int descriptor_latency[2];
	if ( (descriptor_relay > NOVAL) && (descriptor_receiver > NOVAL) )
	{
		descriptor_latency[PIPE_READ] = descriptor_receiver;
		descriptor_latency[PIPE_WRITE] = descriptor_relay;
	}
	else
	{
		descriptor_latency[PIPE_READ] = NOVAL;
		descriptor_latency[PIPE_WRITE] = NOVAL;
	}
	pid_t furcation = fork();
	ASSERTNOERRVAL(furcation, "Furcation error!");
	if (furcation == 0)
	{
		if (level > 0) polynomial_statement(level - 1, argument_counter, argument_vector, 
			descriptor_awakening[PIPE_WRITE], descriptor_awakening[PIPE_READ]);
		else
		{
			descriptor_latency[PIPE_READ] = descriptor_awakening[PIPE_READ];
			descriptor_latency[PIPE_WRITE] = descriptor_awakening[PIPE_WRITE];
			descriptor_awakening[PIPE_READ] = NOVAL;
			descriptor_awakening[PIPE_WRITE] = NOVAL;
			FILE* file_pointer = fopen("./data.dat", "r");
			ASSERTNOERRPTR(file_pointer, "Can't open file!");		
			close(descriptor_latency[PIPE_READ]);
			do
			{
				if (fscanf(file_pointer, "%ld", &result[0]) != 1) break;
				result[1] = result[0] * atoi(argument_vector[1]) + atoi(argument_vector[2]);
			}
			while (write(descriptor_latency[PIPE_WRITE], &result, sizeof(result)) > 0);
			fflush(stdout);
			close(descriptor_latency[PIPE_WRITE]);
		}
	}
	else
	{	
		close(descriptor_awakening[PIPE_WRITE]);
		close(descriptor_latency[PIPE_READ]);
		if (level == (argument_counter - 4))
		{
			int counter = 0;
			while (read(descriptor_awakening[PIPE_READ], &result, sizeof(result)) > 0) 
			{
				++counter;
				result[1] = result[0] * result[1] + atoi(argument_vector[level + 3]);
				printf("\tx%-3d = %8ld, \tf( x%-3d) = %8ld \n", counter, result[0], counter, result[1]);
			}
		}
		else
		{
			while (read(descriptor_awakening[PIPE_READ], &result, sizeof(result)) > 0) 
			{
				result[1] = result[0] * result[1] + atoi(argument_vector[level + 3]);
				write(descriptor_latency[PIPE_WRITE], &result, sizeof(result));
			}
		}
		close(descriptor_awakening[PIPE_READ]);
		fflush(stdout);
		close(descriptor_latency[PIPE_WRITE]);
		wait(0);
	}
}

void prelude(int argument_counter, char** argument_vector)
{
	printf("\tGiven statement: ");
	if (argument_counter == 1) printf("\tNo statement.");
	else if (argument_counter == 2) printf("\t%s", argument_vector[argument_counter - 1]);
	else if (argument_counter == 3)
	{
		if (atoi(argument_vector[argument_counter - 1]) < 0) 
			printf("%s*x - %d", argument_vector[argument_counter - 2], 
				abs(atoi(argument_vector[argument_counter - 1])));
		else printf("%s*x + %s", argument_vector[argument_counter - 2], argument_vector[argument_counter - 1]);
	}
	else
	{
		int iterator;
		for (iterator = 1; iterator < argument_counter; ++iterator)
		{
			if (iterator == 1) printf("%s*x^%d", argument_vector[iterator], (argument_counter - 2));
			else if (iterator == (argument_counter - 2))
			{
				if (atoi(argument_vector[iterator]) < 0) printf(" - %d*x", abs(atoi(argument_vector[iterator])));
				else printf(" + %s*x", argument_vector[iterator]);
			}
			else if (iterator == (argument_counter - 1))
			{
				if (atoi(argument_vector[iterator]) < 0) printf(" - %d", abs(atoi(argument_vector[iterator])));
				else printf(" + %s", argument_vector[iterator]);
			}
			else
			{
				if (atoi(argument_vector[iterator]) < 0) printf(" - %d*x^%d", abs(atoi(argument_vector[iterator])), 
					(argument_counter - iterator - 1));
				else printf(" + %s*x^%d", argument_vector[iterator], (argument_counter - iterator - 1));
			}
		}
	}
	printf("\n");
}
