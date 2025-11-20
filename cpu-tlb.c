/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */

 #include "mm.h"
 #include <stdlib.h>
 #include <stdio.h>
 
 /*
   This function is unnecessary
 */
 int
 tlb_change_all_page_tables_of (struct pcb_t *proc, struct memphy_struct *mp)
 {
   /* TODO: update all page table directory info
    *      in flush or wipe TLB (if needed)
    */
   tlb_flush_tlb_of(proc, mp);
 
   return 0;
 }
 
 int
 tlb_flush_tlb_of (struct pcb_t *proc, struct memphy_struct *mp)
 {
   /* TODO: flush tlb cached*/
   // each process has its tlb_entry directly mapped to a specific address
   for (int i = 0; i * 8 + (proc->pid - 1) < mp->maxsz; i++)
     {
       // this loops through every entries supposed to be
       // belonged to that process and set the entry to 0
       tlb_cache_write (mp, proc->pid, i, 0);
     }
 
   return 0;
 }
 
 /*tlballoc - CPU TLB-based allocate a region memory
  *@proc:  Process executing the instruction
  *@size: allocated size
  *@reg_index: memory region ID (used to identify variable in symbole table)
  */
 int
 tlballoc (struct pcb_t *proc, uint32_t size, uint32_t reg_index)
 {
   int* addr = (int*)malloc(sizeof(int));
   int val;
 
   /* By default using vmaid = 0 */
   val = __alloc (proc, 0, reg_index, size, addr);
 
   /* TODO: update TLB CACHED frame num of the new allocated page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
 
   int pgn = PAGING_PGN (*addr);
   int* frmnum = (int*)malloc(sizeof(int));
   if (pg_getpage (proc->mm, pgn, frmnum, proc) != 0)
     return -1; /* invalid page access */
   tlb_cache_write (proc->tlb, proc->pid, pgn, *frmnum);
 
   return val;
 }
 
 /*pgfree - CPU TLB-based free a region memory
  *@proc: Process executing the instruction
  *@size: allocated size
  *@reg_index: memory region ID (used to identify variable in symbole table)
  */
 int
 tlbfree_data (struct pcb_t *proc, uint32_t reg_index)
 {
   __free (proc, 0, reg_index);
 
   /* TODO: update TLB CACHED frame num of freed page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
   tlb_flush_tlb_of (proc, proc->tlb);
 
   return 0;
 }
 
 /*tlbread - CPU TLB-based read a region memory
  *@proc: Process executing the instruction
  *@source: index of source register
  *@offset: source address = [source] + [offset]
  *@destination: destination storage
  */
 int
 tlbread (struct pcb_t *proc, uint32_t source, uint32_t offset,
          uint32_t destination)
 {
   BYTE *data = (BYTE *)malloc(sizeof (BYTE));
   int frmnum = -1;
 
   /* TODO: retrieve TLB CACHED frame num of accessing page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
   /* frmnum is return value of tlb_cache_read/write value*/
 
   // retrieve pgnum from address
   struct vm_rg_struct *currg = get_symrg_byid (proc->mm, source);
   if(!currg){
     printf("Invalid address: region not found at region=%d offset=%d. READ operation aborted.\n", source, offset);
     return -1;
   }
   if (currg->rg_start + offset >= currg->rg_end) {
     // ! Invalid access address (out of bound)
     printf("Invalid address: out of bound at region=%d offset=%d. READ operation aborted.\n", source, offset);
     return -1;
   }
   int addr = currg->rg_start + offset; // get logical address
   int pgn = PAGING_PGN (addr);
 
   // Read cache using tlb_cache_read()
   int hit_flag = tlb_cache_read (proc->tlb, proc->pid, pgn, &frmnum);
     // printf("frame number: %d\n", frmnum);
 
 
   // this section indicates a HIT, retrieve data
   if (hit_flag >= 0) {
     // physical address
     int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
 
     // retrieve data stored in physical memory
     MEMPHY_read (proc->mram, phyaddr, data);
   }
 
 #ifdef IODUMP
   if (hit_flag >= 0)
     printf ("TLB hit at read region=%d offset=%d value=%d\n", source, offset, *data);
   else
     printf ("TLB miss at read region=%d offset=%d value=%d\n", source, offset, *data);
 #ifdef PAGETBL_DUMP
   print_pgtbl (proc, 0, -1); // print max TBL
 #endif
   MEMPHY_dump (proc->mram);
 #endif
 
   // this section indicates a HIT, retrieve data
  *@offset: destination address = [destination] + [offset]
  */
 int
 tlbwrite (struct pcb_t *proc, BYTE data, uint32_t destination, uint32_t offset)
 {
   int val;
   int *frmnum = (int *)malloc(sizeof(int));
   *frmnum = -1;
 
   /* TODO: retrieve TLB CACHED frame num of accessing page(s))*/
   /* by using tlb_cache_read()/tlb_cache_write()
   frmnum is return value of tlb_cache_read/write value*/
   // retrieve pgnum from address
   struct vm_rg_struct *currg = get_symrg_byid (proc->mm, destination);
   if(!currg){
     printf("Invalid address: region not found at region=%d offset=%d. WRITE operation aborted.\n", destination, offset);
     return -1;
   }
   if (currg->rg_start + offset >= currg->rg_end) {
     // ! Invalid access address (out of bound)
     printf("Invalid address: out of bound at region=%d offset=%d. READ operation aborted.\n", destination, offset);
     return -1;
   }
   int addr = currg->rg_start + offset; // get logical address
   int pgn = PAGING_PGN (addr);
 
 #ifdef IODUMP
   // if (*frmnum >= 0)
   //   printf ("TLB hit at write region=%d offset=%d value=%d\n", destination,
   //           offset, data);
   // else
     printf ("TLB write region=%d offset=%d value=%d\n", destination,
             offset, data);
 #ifdef PAGETBL_DUMP
```c
/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */

 #include "mm.h"
 #include <stdlib.h>
 #include <stdio.h>
 
 /*
   This function is unnecessary
 */
 int
 tlb_change_all_page_tables_of (struct pcb_t *proc, struct memphy_struct *mp)
 {
   /* TODO: update all page table directory info
    *      in flush or wipe TLB (if needed)
    */
   tlb_flush_tlb_of(proc, mp);
 
   return 0;
 }
 
 int
 tlb_flush_tlb_of (struct pcb_t *proc, struct memphy_struct *mp)
 {
   /* TODO: flush tlb cached*/
   // each process has its tlb_entry directly mapped to a specific address
   for (int i = 0; i * 8 + (proc->pid - 1) < mp->maxsz; i++)
     {
       // this loops through every entries supposed to be
       // belonged to that process and set the entry to 0
       tlb_cache_write (mp, proc->pid, i, 0);
     }
 
   return 0;
 }
 
 /*tlballoc - CPU TLB-based allocate a region memory
  *@proc:  Process executing the instruction
  *@size: allocated size
  *@reg_index: memory region ID (used to identify variable in symbole table)
  */
 int
 tlballoc (struct pcb_t *proc, uint32_t size, uint32_t reg_index)
 {
   int* addr = (int*)malloc(sizeof(int));
   int val;
 
   /* By default using vmaid = 0 */
   val = __alloc (proc, 0, reg_index, size, addr);
 
   /* TODO: update TLB CACHED frame num of the new allocated page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
 
   int pgn = PAGING_PGN (*addr);
   int* frmnum = (int*)malloc(sizeof(int));
   if (pg_getpage (proc->mm, pgn, frmnum, proc) != 0)
     return -1; /* invalid page access */
   tlb_cache_write (proc->tlb, proc->pid, pgn, *frmnum);
 
   return val;
 }
 
 /*pgfree - CPU TLB-based free a region memory
  *@proc: Process executing the instruction
  *@size: allocated size
  *@reg_index: memory region ID (used to identify variable in symbole table)
  */
 int
 tlbfree_data (struct pcb_t *proc, uint32_t reg_index)
 {
   __free (proc, 0, reg_index);
 
   /* TODO: update TLB CACHED frame num of freed page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
   tlb_flush_tlb_of (proc, proc->tlb);
 
   return 0;
 }
 
 /*tlbread - CPU TLB-based read a region memory
  *@proc: Process executing the instruction
  *@source: index of source register
  *@offset: source address = [source] + [offset]
  *@destination: destination storage
  */
 int
 tlbread (struct pcb_t *proc, uint32_t source, uint32_t offset,
          uint32_t destination)
 {
   BYTE *data = (BYTE *)malloc(sizeof (BYTE));
   int frmnum = -1;
 
   /* TODO: retrieve TLB CACHED frame num of accessing page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
   /* frmnum is return value of tlb_cache_read/write value*/
 
   // retrieve pgnum from address
   struct vm_rg_struct *currg = get_symrg_byid (proc->mm, source);
   if(!currg){
     printf("Invalid address: region not found at region=%d offset=%d. READ operation aborted.\n", source, offset);
     return -1;
   }
   if (currg->rg_start + offset >= currg->rg_end) {
     // ! Invalid access address (out of bound)
     printf("Invalid address: out of bound at region=%d offset=%d. READ operation aborted.\n", source, offset);
     return -1;
   }
   int addr = currg->rg_start + offset; // get logical address
   int pgn = PAGING_PGN (addr);
 
   // Read cache using tlb_cache_read()
   int hit_flag = tlb_cache_read (proc->tlb, proc->pid, pgn, &frmnum);
     // printf("frame number: %d\n", frmnum);
 
 
   // this section indicates a HIT, retrieve data
   if (hit_flag >= 0) {
     // physical address
     int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
 
     // retrieve data stored in physical memory
     MEMPHY_read (proc->mram, phyaddr, data);
   }
 
 #ifdef IODUMP
   if (hit_flag >= 0)
     printf ("TLB hit at read region=%d offset=%d value=%d\n", source, offset, *data);
   else
     printf ("TLB miss at read region=%d offset=%d value=%d\n", source, offset, *data);
 #ifdef PAGETBL_DUMP
   print_pgtbl (proc, 0, -1); // print max TBL
 #endif
   MEMPHY_dump (proc->mram);
 #endif
 
   // this section indicates a HIT, retrieve data
   if (hit_flag < 0) {
     // TLB miss, retrieve frame number from page table
     if (pg_getpage (proc->mm, pgn, &frmnum, proc) != 0)
       return -1; /* invalid page access */
 
     // physical address
     int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
 
     // retrieve data stored in physical memory
     MEMPHY_read (proc->mram, phyaddr, data);
 
     // update TLB cache
     tlb_cache_write (proc->tlb, proc->pid, pgn, frmnum);
   }
 
   // assign data to destination
   // TODO: check if destination is a register or memory address
   // For now, assume destination is a register index
   // This part might need adjustment based on how 'destination' is used
   // For example, if destination is a register, you might have a function like
   // set_reg_value(proc, destination, *data);
   // If destination is a memory address, it would be a write operation.
   // Given the context of tlbread, it's likely reading into a register.
   // Assuming a simple assignment for now.
   proc->regs[destination] = (uint32_t)*data;
 
   free(data);
   return 0;
 }
 
 /*tlbwrite - CPU TLB-based write a region memory
  *@proc: Process executing the instruction
  *@data: data to write
  *@destination: index of destination register
  *@offset: destination address = [destination] + [offset]
  */
 int
 tlbwrite (struct pcb_t *proc, BYTE data, uint32_t destination, uint32_t offset)
 {
   int val;
   int *frmnum = (int *)malloc(sizeof(int));
   *frmnum = -1;
 
   /* TODO: retrieve TLB CACHED frame num of accessing page(s))*/
   /* by using tlb_cache_read()/tlb_cache_write()
   frmnum is return value of tlb_cache_read/write value*/
   // retrieve pgnum from address
   struct vm_rg_struct *currg = get_symrg_byid (proc->mm, destination);
   if(!currg){
     printf("Invalid address: region not found at region=%d offset=%d. WRITE operation aborted.\n", destination, offset);
     return -1;
   }
   if (currg->rg_start + offset >= currg->rg_end) {
     // ! Invalid access address (out of bound)
     printf("Invalid address: out of bound at region=%d offset=%d. READ operation aborted.\n", destination, offset);
     return -1;
   }
   int addr = currg->rg_start + offset; // get logical address
   int pgn = PAGING_PGN (addr);
 
 #ifdef IODUMP
   // if (*frmnum >= 0)
   //   printf ("TLB hit at write region=%d offset=%d value=%d\n", destination,
   //           offset, data);
   // else
     printf ("TLB write region=%d offset=%d value=%d\n", destination,
             offset, data);
 #ifdef PAGETBL_DUMP
   print_pgtbl (proc, 0, -1); // print max TBL
 #endif
   MEMPHY_dump (proc->mram);
 #endif
 
   if (pg_getpage (proc->mm, pgn, frmnum, proc) != 0)
     return -1; /* invalid page access */
 
   int phyaddr = (*frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
 
   val = MEMPHY_write (proc->mram, phyaddr, data);
 
   /* TODO: update TLB CACHED with frame num of recent accessing page(s)*/
   /* by using tlb_cache_read()/tlb_cache_write()*/
   tlb_cache_write (proc->tlb, proc->pid, pgn, *frmnum);
   printf("%s:%d\n",__func__,__LINE__);
 
   return val;
 }
 
 // #endif
```