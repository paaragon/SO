#include "sched.h"

/* Global multiQRR quantum parameter */
int mix_quantum=3;


/* Structure to store RR thread-specific fields */
// ATENCION: puede requerir nuevos campos. Modificad si es necesario
struct mix_data {
     int remaining_ticks_slice;
     int quantum;
     int from_es;
     int new;
};

// ATENCION: estructura nueva. Son las 2 listas de tareas que se usaran. La de indice 0 es mas prioritaria
// Por defecto, todas las tareas comienzan en la lista 0. Si agotan su quanto de tiempo antes de bloquearse
// pasan a la lista 1. Cuando vuelven del boqueo, van siempre a la lista 0
struct multiQueues {
	slist_t tasks[2];
};


//ATENCION: funcion nueva que NO hay que modificar
static int sched_init_mix(void) {
   int cpu;

  for (cpu=0;cpu<nr_cpus;cpu++){
     //init_runqueue(&runqueues[cpu],cpu);
     struct multiQueues* rq_data = malloc(sizeof(struct multiQueues));
     init_slist(&(rq_data->tasks[0]), offsetof(task_t,rq_links));
     init_slist(&(rq_data->tasks[1]), offsetof(task_t,rq_links));
     runqueue_t* rq = get_runqueue_cpu(cpu);     
     rq->rq_cs_data = rq_data;	    
  }
  return 0;
}

//ATENCION: funcion nueva que NO hay que modificar
void sched_destroy_mix() {
int cpu;

  for (cpu=0;cpu<nr_cpus;cpu++){
	runqueue_t* rq =  get_runqueue_cpu(cpu);
	if (rq->rq_cs_data ) {
		free(rq->rq_cs_data);
		rq->rq_cs_data = NULL;
	}
  }
}

static int task_new_mix(task_t* t){
    struct mix_data* cs_data=malloc(sizeof(struct mix_data));

    if (!cs_data)
        return 1;  /* Cannot reserve memory */


    /* initialize the quantum */
    cs_data->remaining_ticks_slice=mix_quantum;

    // ATENCION: si se incluya algun campo mas en la informacion privada de la tarea, hay que inicializarlo aqui
    cs_data->from_es = 1;
    cs_data->quantum = mix_quantum;
    cs_data->new = 1;

    t->tcs_data=cs_data;
    return 0;
}

static void task_free_mix(task_t* t){
    if (t->tcs_data){
        free(t->tcs_data);
        t->tcs_data=NULL;
    }
}

static task_t* pick_next_task_mix(runqueue_t* rq,int cpu) {
    // ATENCION: ejemplo de uso de las listas
    // 1. accedo al campo privado
    struct multiQueues* q_data = (struct multiQueues*) rq->rq_cs_data;

    // 2. cojo una tarea de la lista 0
    task_t* t=head_slist(&(q_data->tasks[0]));
    
    if (t) {

        // ATENCION: ejemplo de llamada a remove con la lista 0
	remove_slist(&q_data->tasks[0],t);
        t->on_rq=FALSE;
        rq->cur_task=t;
	
    } else {
      
      t=head_slist(&(q_data->tasks[1]));
    
	if (t) {

	    // ATENCION: ejemplo de llamada a remove con la lista 0
	    remove_slist(&q_data->tasks[1],t);
	    t->on_rq=FALSE;
	    rq->cur_task=t;
	    
	}
    }
  
    
    return t;
}

static void enqueue_task_mix(task_t* t,int cpu, int runnable) {
    runqueue_t* rq=get_runqueue_cpu(cpu);
    struct mix_data* cs_data=(struct mix_data*) t->tcs_data;
    struct multiQueues* q_data = (struct multiQueues*) rq->rq_cs_data;
   
    if (t->on_rq || is_idle_task(t))
        return;
    
    if(cs_data->from_es == 0){
      insert_slist(&q_data->tasks[1],t); //Push task
      if(cs_data->quantum > 1)
	cs_data->quantum--;
    }else{
      insert_slist(&q_data->tasks[0],t); //Push task
      if(cs_data->quantum < 5 && cs_data->new == 0)
	cs_data->quantum++;
      cs_data->new = 0;
    }
    t->on_rq=TRUE;
    cs_data->remaining_ticks_slice=cs_data->quantum; // Reset slice 
   
     
    /* If the task was not runnable before on this RQ (just changed the status)*/
    if (!runnable){
        rq->nr_runnable++;
        t->last_cpu=cpu;
    }
}


static void task_tick_mix(runqueue_t* rq,int cpu){

    task_t* current=rq->cur_task;
    struct mix_data* cs_data=(struct mix_data*) current->tcs_data;

    if (is_idle_task(current))
        return;
    
    cs_data->remaining_ticks_slice--; /* Charge tick */

    if (cs_data->remaining_ticks_slice<=0) {
	cs_data->from_es = 0;
        rq->need_resched=TRUE; //Force a resched !!

    }
    if (current->runnable_ticks_left==1) {
	cs_data->from_es = 1;
        rq->nr_runnable--; // The task is either exiting or going to sleep right now 	
   }   
}

static task_t* steal_task_mix(runqueue_t* rq,int cpu){
    struct multiQueues* q_data = (struct multiQueues*) rq->rq_cs_data;
  
    //ATENCION: para la modificaion, tratar de migrar siempre tareas de la lista 1. Si no hay, coger de la 0
    task_t* t=tail_slist(&q_data->tasks[1]);
    if (t) {
        remove_slist(&q_data->tasks[1],t);
        t->on_rq=FALSE;
        rq->nr_runnable--;
    }else{
	t=tail_slist(&q_data->tasks[0]);
	if (t) {
	    remove_slist(&q_data->tasks[0],t);
	    t->on_rq=FALSE;
	    rq->nr_runnable--;
	}
    }
    return t;  
}
    

sched_class_t mix_sched={ 
    .sched_init = sched_init_mix,  
    .sched_destroy = sched_destroy_mix,   
    .task_new=task_new_mix,
    .task_free=task_free_mix,
    .pick_next_task=pick_next_task_mix,
    .enqueue_task=enqueue_task_mix,
    .task_tick=task_tick_mix,
    .steal_task=steal_task_mix
};