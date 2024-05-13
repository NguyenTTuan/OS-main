/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  /* No action needed for direct mapped TLB on process context change
     since page table updates only affect the page directory entries. */

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/
  // Flush the TLB cache for the given page table

  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;

  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);
  if (val != 0) return val;

  return 0;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  /* By default using vmaid = 0 */
  int val;
  val = __free(proc, 0, reg_index);
  if (val != 0) return val;
  
  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  printf("TLB read - PID=%d, Source: %u, Destination: %u, Offset: %u\n", proc->pid, source, destination, offset);
  BYTE data = -1;
  int frmnum = -1;

  struct vm_rg_struct *currg = get_symrg_byid(proc->mm, source);

  struct vm_area_struct *cur_vma = get_vma_by_num(proc->mm, 0);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  addr_t address = currg->rg_start + offset;

  // printf("Address: %u\n", address);

  if(address >= currg->rg_end) {
    printf("Invalid destination\n");
    return -1;
  }

  int pgnum = PAGING_PGN(address);
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/

	int val = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frmnum);
  if(val < 0) return -1;

#ifdef IODUMP
  if (frmnum >= 0) {
    printf("TLB hit at read region=%d offset=%d\n", source, offset);
    int off = PAGING_OFFST(address);
    int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + off;
    
    val = MEMPHY_read(proc->mram, phyaddr, &data);
    if(val < 0) return -1; 
  }
  else {
    printf("TLB miss at read region=%d offset=%d\n", source, offset);
    val = __read(proc, 0, source, offset, &data);
    if(val < 0) return -1;
  }

  printf("read region=%d offset=%d value=%d\n", source, offset, data);
  destination = (uint32_t) data;

#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return 0;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  printf("TLB write - PID=%d, Destination: %u, Offset: %u\n", proc->pid, destination, offset);

  int val;
  int frmnum = -1;
  struct vm_rg_struct *currg = get_symrg_byid(proc->mm, destination);

  struct vm_area_struct *cur_vma = get_vma_by_num(proc->mm, 0);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  addr_t address = currg->rg_start + offset;

  // printf("Address: %u\n", address);

  if(address >= currg->rg_end) {
    printf("Invalid destination\n");
    return -1;
  }

  int pgnum = PAGING_PGN(address);

  /* TODO retrieve TLB CACHED frame num of accessing page(s)) */
  /* by using tlb_cache_read()/tlb_cache_write() */
  /* frmnum is return value of tlb_cache_read/write value */

  val = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frmnum);
  // printf("Frame num: %d\n", frmnum);
  if(val < 0) return -1;
  
#ifdef IODUMP
  if (frmnum >= 0) {
    printf("TLB hit at write region=%d offset=%d value=%d\n", destination, offset, data);
    int off = PAGING_OFFST(address);
    int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + off;

    MEMPHY_write(proc->mram, phyaddr, data);
    if(val < 0) return -1;
  }
	else {
    printf("TLB miss at write region=%d offset=%d value=%d\n", destination, offset, data);
    val = __write(proc, 0, destination, offset, data);
  }

  printf("write region=%d offset=%d value=%d\n", destination, offset, data);

#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return 0;
}

//#endif