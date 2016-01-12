#include "sched.h"

/* Global RR quantum parameter */
int rr_dyn_quantum=3;

/* Structure to store RR thread-specific fields */
struct rr_dyn_data {
     int remaining_ticks_slice;
     int quantum;
     int from_es;
};

static int task_new_rr_dyn(task_t* t){
    struct rr_dyn_data* cs_data=malloc(sizeof(struct rr_dyn_data));

    if (!cs_data)
        return 1;  /* Cannot reserve memory */


    /* initialize the quantum */
    cs_data->remaining_ticks_slice=rr_dyn_quantum;
    cs_data->quantum = rr_dyn_quantum;
    cs_data->from_es = 0;
    t->tcs_data=cs_data;
    return 0;
}

static void task_free_rr_dyn(task_t* t){
    if (t->tcs_data){
        free(t->tcs_data);
        t->tcs_data=NULL;
    }
}

static task_t* pick_next_task_rr_dyn(runqueue_t* rq,int cpu) {
    task_t* t=head_slist(&rq->tasks);
    
    if (t) {
        /* Current is not on the rq*/
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->cur_task=t;
    }
    
    return t;
}

static void enqueue_task_rr_dyn(task_t* t,int cpu, int runnable) {
    runqueue_t* rq=get_runqueue_cpu(cpu);
    struct rr_dyn_data* cs_data=(struct rr_dyn_data*) t->tcs_data;

    if (t->on_rq || is_idle_task(t))
        return;
    
    insert_slist(&rq->tasks,t); //Push task
    t->on_rq=TRUE;
    
    /*                      */
    
    if(cs_data->from_es == 1 && cs_data->quantum < 5){
      cs_data->quantum++;
    }
    
    cs_data->remaining_ticks_slice=cs_data->quantum; // Reset slice 
    
    /*                      */
            
    /* If the task was not runnable before on this RQ (just changed the status)*/
    if (!runnable){
        rq->nr_runnable++;
        t->last_cpu=cpu;
    }
}


static void task_tick_rr_dyn(runqueue_t* rq,int cpu){

    task_t* current=rq->cur_task;
    struct rr_dyn_data* cs_data=(struct rr_dyn_data*) current->tcs_data;

    if (is_idle_task(current))
        return;
    
    cs_data->remaining_ticks_slice--; /* Charge tick */
    
    if (current->runnable_ticks_left==1) {
	cs_data->from_es = 1;
        rq->nr_runnable--; // The task is either exiting or going to sleep right now    
        
    }else if (cs_data->remaining_ticks_slice<=0){
      
      /*                      */
	if(cs_data->quantum > 1){
	  cs_data->quantum--;
	}
	
	cs_data->from_es = 0;
      /*                      */
        rq->need_resched=TRUE; //Force a resched !!
    }
    
    
}

static task_t* steal_task_rr_dyn(runqueue_t* rq,int cpu){
    task_t* t=tail_slist(&rq->tasks);
    
    if (t) {
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->nr_runnable--;
    }
    return t;    
}
    

sched_class_t rr_dyn_sched={
    .task_new=task_new_rr_dyn,
    .task_free=task_free_rr_dyn,
    .pick_next_task=pick_next_task_rr_dyn,
    .enqueue_task=enqueue_task_rr_dyn,
    .task_tick=task_tick_rr_dyn,
    .steal_task=steal_task_rr_dyn
};