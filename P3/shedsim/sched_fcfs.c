#include "sched.h"


static int task_new_fcfs(task_t* t){
   
   //NO ES NECESARIO IMPLEMENTAR
   
    return 0;
}

static void task_free_fcfs(task_t* t){
	
	//NO ES NECESARIO IMPLEMENTAR
	
    if (t->tcs_data){
        free(t->tcs_data);
        t->tcs_data=NULL;
    }
}


static task_t* pick_next_task_fcfs(runqueue_t* rq,int cpu) {
    task_t* t=head_slist(&rq->tasks);
    
    if (t) {
        /* Current is not on the rq*/
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->cur_task=t;
    }
    
    return t;
}

static void enqueue_task_fcfs(task_t* t,int cpu, int runnable) {
    runqueue_t* rq=get_runqueue_cpu(cpu);
    insert_slist(&rq->tasks,t);
    t->on_rq=TRUE;
    
    if (!runnable){
        rq->nr_runnable++;
        t->last_cpu=cpu;
    }
}


static void task_tick_fcfs(runqueue_t* rq,int cpu){

    task_t* current=rq->cur_task;
        
    if (current->runnable_ticks_left==1) 
        rq->nr_runnable--; // The task is either exiting or going to sleep right now   
}

static task_t* steal_task_fcfs(runqueue_t* rq,int cpu){
    task_t* t=tail_slist(&rq->tasks);
    
    if (t) {
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->nr_runnable--;
    }
    return t; 
}
    

sched_class_t fcfs_sched={
    .task_new=task_new_fcfs,
    .task_free=task_free_fcfs,
    .pick_next_task=pick_next_task_fcfs,
    .enqueue_task=enqueue_task_fcfs,
    .task_tick=task_tick_fcfs,
    .steal_task=steal_task_fcfs
};
