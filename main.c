#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Task
{
	int id;
	double arrival;
	double period;		//Safe: random from 3xTc(i) to maximum_period_multipler x Tc(i) | No Guarantee: random from from 3xTc(i) to 10xTc(i)
	double computation; // random from 1 to maximum_computation_time
	int priority;
	xTaskHandle handle;
} Task;

void genFunction(void *a_pvParameters);
void taskScheduler(void *a_pvParameters);
void sortAndPrioritize(Task a_tasksArr[], int a_size);
void printTasks(Task a_tasksArr[], int a_size);
void admitTask(Task a_newTask);
void deleteTask(int a_index);
double computeUtilization(Task a_tasksArr[], int a_size);

int g_currTasks = 0;
Task *g_arrayOfTasks;
int g_maxTime = 0;
int g_time = 0;
xTaskHandle g_schedulerHandle;

#define N 5
#define SAFE_MODE 0

int main(void)
{
	int i = 0;
	Task tempTask;
	srand(0);
	for (i = 0; i < N; i++)
	{
		tempTask.id = i + 1;
		tempTask.arrival = (double)(rand() % (20 * (i + 1))) + 1;
		tempTask.period = (double)(rand() % 100) + 1;
		tempTask.computation = (double)(rand() % (int)tempTask.period) + 1;
		tempTask.handle = NULL;
		admitTask(tempTask);
	}
	printTasks(g_arrayOfTasks, g_currTasks);
	printf("max %d\n", g_maxTime);
	xTaskCreate(taskScheduler, "scheduler", 100, NULL, N + 1, &g_schedulerHandle);
	vTaskStartScheduler();

	while (1)
		;
}

void taskScheduler(void *a_pvParameters)
{
	int i = 0;
	while (1)
	{
		if (g_time <= (g_maxTime + 1))
		{
			printf("\t%d\n", g_time);
			for (i = 0; i < g_currTasks; i++)
			{
				if (g_time >= (g_arrayOfTasks[i].arrival + g_arrayOfTasks[i].period))
				{
					vTaskDelete(g_arrayOfTasks[i].handle);
					deleteTask(i);
					i--;
				}
				else if ((g_arrayOfTasks[i].arrival <= g_time) && (!g_arrayOfTasks[i].handle))
				{
					printf("\t\ttsk %d\n", g_arrayOfTasks[i].id);
					xTaskCreate(genFunction, NULL, 100, (void *)&g_arrayOfTasks[i], g_arrayOfTasks[i].priority, &(g_arrayOfTasks[i].handle));
				}
			}
			vTaskSuspend(NULL);
		}
	}
}

void genFunction(void *a_pvParameters)
{
	Task t = *((Task *)(a_pvParameters));

	/* As per most tasks, this task is implemented in an infinite loop. */
	while (1)
	{
		vTaskSuspendAll();
		printf("t%d\n", t.id);
		xTaskResumeAll();
	}
}

static int myCompare(const void *a, const void *b)
{

	const Task *t1 = (Task *)a;
	const Task *t2 = (Task *)b;
	// setting up rules for comparison
	return t1->period < t2->period;
	// return t1->arrival < t2->arrival;
}

void sortAndPrioritize(Task a_tasksArr[], int a_size)
{
	int i = 0, lastPriority = 1;
	qsort(a_tasksArr, a_size, sizeof(Task), myCompare);

	for (i = 0; i < a_size - 1; i++)
	{
		if (a_tasksArr[i].period != a_tasksArr[i + 1].period)
			a_tasksArr[i].priority = lastPriority++;
		else
			a_tasksArr[i].priority = lastPriority;
	}
	a_tasksArr[i].priority = lastPriority;
}

double computeUtilization(Task a_tasksArr[], int a_size)
{
	int i = 0;
	double cpuUtilization = 0.0;
	for (i = 0; i < a_size; i++)
	{
		printf("c: %lf, p: %lf \n", a_tasksArr[i].computation, a_tasksArr[i].period);
		cpuUtilization += (a_tasksArr[i].computation / a_tasksArr[i].period);
	}
	printf("CPU Utilization: %lf \n", cpuUtilization);
	return cpuUtilization;
}

void printTasks(Task a_tasksArr[], int a_size)
{
	int i = 0;
	printf("Print called\n");
	for (i = 0; i < a_size; i++)
	{
		printf("id:%d, ari:%lf  pri: %d, com: %lf, per: %lf \n", a_tasksArr[i].id, a_tasksArr[i].arrival, a_tasksArr[i].priority, a_tasksArr[i].computation, a_tasksArr[i].period);
	}
}

void admitTask(Task a_newTask)
{
	int i = 0;
	Task *temp = malloc((g_currTasks + 1) * sizeof(Task));

	vTaskSuspendAll();
	printf("add %d, ", a_newTask.id);
	xTaskResumeAll();

	if (!temp)
		printf("allocNO\n");
	else
		printf("allocOK\n");

	for (i = 0; i < g_currTasks; i++)
	{
		temp[i] = g_arrayOfTasks[i];
	}

	free(g_arrayOfTasks);
	g_arrayOfTasks = temp;
	g_arrayOfTasks[g_currTasks++] = a_newTask;
	if (a_newTask.arrival + a_newTask.period > g_maxTime)
		g_maxTime = a_newTask.arrival + a_newTask.period;
	sortAndPrioritize(g_arrayOfTasks, g_currTasks);
}

void deleteTask(int a_index)
{
	int i = 0;
	Task *temp = malloc((g_currTasks - 1) * sizeof(Task));

	printf("\t\tdel %d, ", g_arrayOfTasks[a_index].id);

	if (!temp)
		printf("dallocNO\n");
	else
		printf("dallocOK\n");

	for (i = 0; i < a_index; i++)
	{
		temp[i] = g_arrayOfTasks[i];
	}
	for (i = a_index + 1; i < g_currTasks; i++)
	{
		temp[i - 1] = g_arrayOfTasks[i];
	}

	free(g_arrayOfTasks);
	g_arrayOfTasks = temp;
	g_currTasks--;
	sortAndPrioritize(g_arrayOfTasks, g_currTasks);
}

// /*-----------------------------------------------------------*/

// void vApplicationMallocFailedHook(void)
// {
// 	/* This function will only be called if an API call to create a task, queue
// 	or semaphore fails because there is too little heap RAM remaining - and
// 	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. */
// 	while(1)
// 		;
// }
// /*-----------------------------------------------------------*/

// void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
// {
// 	/* This function will only be called if a task overflows its stack.  Note
// 	that stack overflow checking does slow down the context switch
// 	implementation and will only be performed if configCHECK_FOR_STACK_OVERFLOW
// 	is set to either 1 or 2 in FreeRTOSConfig.h. */
// 	while(1)
// 		;
// }
// /*-----------------------------------------------------------*/

// void vApplicationIdleHook(void)
// {
// 	/* This example does not use the idle hook to perform any processing.  The
// 	idle hook will only be called if configUSE_IDLE_HOOK is set to 1 in
// 	FreeRTOSConfig.h. */
// }
// /*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
	/* This example does not use the tick hook to perform any processing.   The
 	tick hook will only be called if configUSE_TICK_HOOK is set to 1 in
 	FreeRTOSConfig.h. */
	if (g_time <= g_maxTime + 1)
	{
		g_time++;
	}
	vTaskResume(g_schedulerHandle);
}
