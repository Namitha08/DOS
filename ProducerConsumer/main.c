/*  main.c  - main */

#include <xinu.h>
#include <stdbool.h>
#define QUEUESIZE 100
pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = QUEUESIZE;


/* Define your circular buffer structure and semaphore variables here */
int produced, consumed, mutex;
int queue[QUEUESIZE];
int head,tail;
int n = 0;

void mutex_acquire(sid32 mutex)
{
    wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
    signal(mutex);
}

/* Place the code for the buffer producer here */
process producer(sid32 consumed, sid32 produced,sid32 mutex)
{
	
    int i = 0;
    while(1){
        mutex_acquire(consumed);
        mutex_acquire(mutex);
        i++;
        queue[tail++] = i;     
        kprintf("added %d to queue\n", i);
        if(tail >= CONSUMED_MAX)
            tail = 0;
        mutex_release(mutex);
        mutex_release(produced);
    }
}

/* Place the code for the buffer consumer here */
process consumer(sid32 consumed, sid32 produced,sid32 mutex)
{
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */
	/* */

    int n;
    while(1)
    {
        mutex_acquire(produced);      
        mutex_acquire(mutex);
        n = queue[head++];
        kprintf("%d is consumed\n", n);
        if(head >= CONSUMED_MAX)
	        head = 0;
	      consumed_count += 1;
        mutex_release(mutex);        
        mutex_release(consumed);
    }
}


/* Timing utility function - please ignore */
process time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < CONSUMED_MAX * (i+1))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		kprintf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
	}
}

process	main(void)
{
	recvclr();

	/* Create the shared circular buffer and semaphores here */
	/* */
         
	  consumed = semcreate(N);
    produced = semcreate(0);
    mutex = semcreate(1);
    head = 0;
    tail = 0;
	  producer_id = create(producer, 4096, 50, "producer",3, consumed, produced, mutex);
	  consumer_id = create(consumer, 4096, 50, "consumer",3, consumed, produced, mutex);
	  timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id);
	resched_cntl(DEFER_STOP);

	return OK;
}