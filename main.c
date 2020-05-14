#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>

/* The task function. */
void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Task 1 is running\n";
const char *pcTextForTask2 = "Task 2 is running\n";

int n = 5;
#define TST 1
#define SAFE_MODE 0

typedef struct Task
{
	int id;
	double arrival;
	double period; //Safe: random from 3xTc(i) to maximum_period_multipler x Tc(i) | No Guarantee: random from from 3xTc(i) to 10xTc(i)
	double computation; // random from 1 to maximum_computation_time
	int priority;
} Task;

static int myCompare(const void *a, const void *b)
{

	const Task *t1 = (Task *)a;
	const Task *t2 = (Task *)b;
	// setting up rules for comparison
	return t1->period < t2->period;
}

// Function to sortAndPrioritize the array
void sortAndPrioritize(Task arr[], int n)
{
	int i = 0, lastPriority = 1;
	qsort(arr, n, sizeof(Task), myCompare);


	for (i = 0; i < n - 1; i++)
	{
		if (arr[i].period != arr[i + 1].period)
			arr[i].priority = lastPriority++;
		else
			arr[i].priority = lastPriority;
	}
	arr[i].priority = lastPriority;
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
		printf("id:%d pri: %d, com: %lf, per: %lf \n", a_tasksArr[i].id, a_tasksArr[i].priority, a_tasksArr[i].computation, a_tasksArr[i].period);
	}
}

int currTasks = 0;
Task *arrayOfTasks;

void admitTask(Task newTask)
{
	int i = 0;
	Task *temp = malloc((currTasks + 1) * sizeof(Task));

	if (!temp)
		printf("allocNO\n");
	else
		printf("allocOK\n");

	for (i = 0; i < currTasks; i++)
	{
		temp[i] = arrayOfTasks[i];
	}

	free(arrayOfTasks);
	arrayOfTasks = temp;
	arrayOfTasks[currTasks++] = newTask;
	sortAndPrioritize(arrayOfTasks, currTasks);
}

void deleteTask(int index)
{
	int i = 0;
	Task *temp = malloc((currTasks - 1) * sizeof(Task));

	if (!temp)
		printf("allocNO\n");
	else
		printf("allocOK\n");

	for (i = 0; i < index; i++)
	{
		temp[i] = arrayOfTasks[i];
	}
	for (i = index + 1; i < currTasks; i++)
	{
		temp[i - 1] = arrayOfTasks[i];
	}

	free(arrayOfTasks);
	arrayOfTasks = temp;
	currTasks--;
	sortAndPrioritize(arrayOfTasks, currTasks);
}

int main(void)
{
	Task tempTask = {1, 3, 11, 2};
	admitTask(tempTask);

	tempTask.id = 2;
	tempTask.arrival = 9;
	tempTask.period = 20;
	tempTask.computation = 1;
	admitTask(tempTask);

	tempTask.id = 3;
	tempTask.arrival = 12;
	tempTask.period = 32;
	tempTask.computation = 5;
	admitTask(tempTask);

	tempTask.id = 4;
	tempTask.arrival = 6;
	tempTask.period = 27;
	tempTask.computation = 4;
	admitTask(tempTask);

	tempTask.id = 5;
	tempTask.arrival = 13;
	tempTask.period = 13;
	tempTask.computation = 1;
	admitTask(tempTask);

	tempTask.id = 5;
	tempTask.arrival = 13;
	tempTask.period = 13;
	tempTask.computation = 1;
	admitTask(tempTask);
	// printTasks(arrayOfTasks, currTasks);
	// deleteTask(2);
	printTasks(arrayOfTasks, currTasks);
	// sortAndPrioritize(arrayOfTasks, currTasks);
	/* Create one of the two tasks. */
	// xTaskCreate(vTaskFunction,			/* Pointer to the function that implements the task. */
	// 			"Task 1",				/* Text name for the task.  This is to facilitate debugging only. */
	// 			128,					/* Stack depth in words. */
	// 			(void *)&arrayOfTasks[0], /* Pass the text to be printed in as the task parameter. */
	// 			1,						/* This task will run at priority 1. */
	// 			NULL);					/* We are not using the task handle. */

	/* Create the other task in exactly the same way.  Note this time that we
	are creating the SAME task, but passing in a different parameter.  We are
	creating two instances of a single task implementation. */
	xTaskCreate(vTaskFunction, "Task 1", 100, (void *)&arrayOfTasks[0], 1, NULL);
	xTaskCreate(vTaskFunction, "Task 2", 100, (void *)&arrayOfTasks[1], 1, NULL);
	xTaskCreate(vTaskFunction, "Task 3", 100, (void *)&arrayOfTasks[2], 1, NULL);
	xTaskCreate(vTaskFunction, "Task 4", 100, (void *)&arrayOfTasks[3], 1, NULL);
	xTaskCreate(vTaskFunction, "Task 5", 100, (void *)&arrayOfTasks[4], 1, NULL);
	xTaskCreate(vTaskFunction, "Task 6", 100, (void *)&arrayOfTasks[5], 1, NULL);
	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vTaskFunction(void *pvParameters)
{
	Task t = *((Task *)(pvParameters));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		printf("%d\n", t.id);
	}
}
// /*-----------------------------------------------------------*/

// void vApplicationMallocFailedHook(void)
// {
// 	/* This function will only be called if an API call to create a task, queue
// 	or semaphore fails because there is too little heap RAM remaining - and
// 	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. */
// 	for (;;)
// 		;
// }
// /*-----------------------------------------------------------*/

// void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
// {
// 	/* This function will only be called if a task overflows its stack.  Note
// 	that stack overflow checking does slow down the context switch
// 	implementation and will only be performed if configCHECK_FOR_STACK_OVERFLOW
// 	is set to either 1 or 2 in FreeRTOSConfig.h. */
// 	for (;;)
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

// void vApplicationTickHook(void)
// {
// 	/* This example does not use the tick hook to perform any processing.   The
// 	tick hook will only be called if configUSE_TICK_HOOK is set to 1 in
// 	FreeRTOSConfig.h. */
// }
