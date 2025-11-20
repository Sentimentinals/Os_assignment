/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "os-mm.h"
#include "syscall.h"
#include "libmem.h"
#include "queue.h"
#include <stdlib.h>

#ifdef MM64
#include "mm64.h"
#else
#include "mm.h"
#endif

//typedef char BYTE;

int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
   int memop = regs->a1;
   BYTE value;
   
   /* TODO THIS DUMMY CREATE EMPTY PROC TO AVOID COMPILER NOTIFY 
    *      need to be eliminated
	*/
   struct pcb_t *caller = NULL;
   int i;

   /* Traverse running list to find the caller process */
   if (krnl->running_list != NULL && krnl->running_list->size > 0) {
       for (i = 0; i < krnl->running_list->size; i++) {
           if (krnl->running_list->proc[i] != NULL && 
               krnl->running_list->proc[i]->pid == pid) {
               caller = krnl->running_list->proc[i];
               break;
           }
       }
   }

   /* If not found in running list, try MLQ ready queues */
   #ifdef MLQ_SCHED
   if (caller == NULL && krnl->mlq_ready_queue != NULL) {
       int prio;
       for (prio = 0; prio < MAX_PRIO && caller == NULL; prio++) {
           for (i = 0; i < krnl->mlq_ready_queue[prio].size; i++) {
               if (krnl->mlq_ready_queue[prio].proc[i] != NULL &&
                   krnl->mlq_ready_queue[prio].proc[i]->pid == pid) {
                   caller = krnl->mlq_ready_queue[prio].proc[i];
                   break;
               }
           }
       }
   }
   #endif

   /* If still not found, try ready queue */
   if (caller == NULL && krnl->ready_queue != NULL && krnl->ready_queue->size > 0) {
       for (i = 0; i < krnl->ready_queue->size; i++) {
           if (krnl->ready_queue->proc[i] != NULL &&
               krnl->ready_queue->proc[i]->pid == pid) {
               caller = krnl->ready_queue->proc[i];
               break;
           }
       }
   }
   
   if (caller == NULL) {
       printf("[ERROR] Process with PID %d not found in any queue!\n", pid);
       return -1; /* Process not found */
   }
   
   /* Safety check: ensure caller->krnl is valid */
   if (caller->krnl == NULL) {
       printf("[ERROR] caller->krnl is NULL for pid=%d\n", pid);
       return -1;
   }
	
   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
			vmap_pgd_memset(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_INC_OP:
            inc_vma_limit(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_SWP_OP:
            __mm_swap_page(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_IO_READ:
            MEMPHY_read(caller->krnl->mram, regs->a2, &value);
            regs->a3 = value;
            break;
   case SYSMEM_IO_WRITE:
            MEMPHY_write(caller->krnl->mram, regs->a2, regs->a3);
            break;
   default:
            printf("Memop code: %d\n", memop);
            break;
   }
   
   return 0;
}


