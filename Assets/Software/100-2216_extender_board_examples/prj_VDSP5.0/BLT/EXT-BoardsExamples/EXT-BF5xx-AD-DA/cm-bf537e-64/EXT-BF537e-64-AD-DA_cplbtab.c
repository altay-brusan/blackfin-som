/* MANAGED-BY-SYSTEM-BUILDER                                    */
/* VisualDSP++ 5.0 Update 6                                     */
/* LDF Printer version: 5.6.0.4                                 */
/* ldfgen.exe version: 5.6.0.4                                  */
/* VDSG version: 5.6.0.4                                        */

/*
** CPLB table definitions generated on Feb 17, 2010 at 09:18:37.
**
** Copyright (C) 2000-2008 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically based upon the options selected
** in the LDF Wizard. Changes to the LDF configuration should be made by
** changing the appropriate options rather than editing this file.
**
** Configuration:-
**     crt_doj:                                EXT-BF537e-64-AD-DA_basiccrt.doj
**     processor:                              ADSP-BF537
**     product_name:                           VisualDSP++ 5.0 Update 6
**     si_revision:                            0.3
**     default_silicon_revision_from_archdef:  0.3
**     cplb_init_cplb_ctrl:                    63
**     cplb_init_dcache_ctrl:                  dcache_wt
**     cplb_init_cplb_src_file:                S:\blackfin\VDK\prj_VDSP5.0\BLT\EXT-BoardsExamples\EXT-BF5xx-AD-DA\cm-bf537e-64\EXT-BF537e-64-AD-DA_cplbtab.c
**     cplb_init_cplb_obj_file:                EXT-BF537e-64-AD-DA_cplbtab.doj
**     using_cplusplus:                        true
**     mem_init:                               false
**     use_vdk:                                false
**     use_eh:                                 true
**     use_argv:                               false
**     running_from_internal_memory:           true
**     user_heap_src_file:                     S:\blackfin\VDK\prj_VDSP5.0\BLT\EXT-BoardsExamples\EXT-BF5xx-AD-DA\cm-bf537e-64\EXT-BF537e-64-AD-DA_heaptab.c
**     libraries_use_stdlib:                   true
**     libraries_use_fileio_libs:              false
**     libraries_use_ieeefp_emulation_libs:    false
**     libraries_use_eh_enabled_libs:          false
**     system_heap:                            L3
**     system_heap_min_size:                   40M
**     system_stack:                           L1
**     system_stack_min_size:                  2k
**     use_sdram:                              true
**     use_sdram_size:                         64MB
**     use_sdram_partitioned:                  none
**
*/


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_2_2)
#pragma diag(suppress:misra_rule_8_10)
#pragma diag(suppress:misra_rule_10_1_a)
#endif /* _MISRA_RULES */



#define CACHE_MEM_MODE CPLB_DDOCACHE_WT



#include <sys/platform.h>
#include <cplbtab.h>

#pragma section("cplb_data")

cplb_entry dcplbs_table[] = {


/*$VDSG<customizable-data-cplb-table>                           */
/* This code is preserved if the CPLB tables are re-generated.  */


   // L1 Data A & B, (set write-through bit to avoid 1st write exceptions)
   {0xFF800000, (PAGE_SIZE_4MB | CPLB_DNOCACHE | CPLB_LOCK | CPLB_WT)}, 

   // Async Memory Bank 0 (Prim A)
   {0x20000000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)},
   
   // Async Memory Bank 1 (Prim B)
   {0x20100000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)}, 
    
   // Async Memory Bank 2 (Second)
   {0x20200000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)}, 
   
   // Async Memory Bank 3
   {0x20300000, (PAGE_SIZE_1MB | CPLB_DNOCACHE)},

      // 512 MB (Maximum) SDRAM memory space 
   {0x00000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   {0x00C00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE | CPLB_DIRTY | CPLB_LOCK)}, 
   
   // CPLBs covering 48MB
   {0x01000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x01c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x02c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03000000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03400000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03800000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
   {0x03c00000, (PAGE_SIZE_4MB | CACHE_MEM_MODE)},
 

   // end of section - termination
   {0xffffffff, 0}, 
/*$VDSG<customizable-data-cplb-table>                           */


}; /* dcplbs_table */

#pragma section("cplb_data")

cplb_entry icplbs_table[] = {


/*$VDSG<customizable-instr-cplb-table>                          */
/* This code is preserved if the CPLB tables are re-generated.  */


   // L1 Code
   {0xFFA00000, (PAGE_SIZE_1MB | CPLB_I_PAGE_MGMT)}, 

   // Async Memory Bank 2 (Secnd)
   // Async Memory Bank 1 (Prim B)
   // Async Memory Bank 0 (Prim A)
   {0x20200000, (PAGE_SIZE_1MB | CPLB_INOCACHE)}, 
   {0x20100000, (PAGE_SIZE_1MB | CPLB_IDOCACHE)}, 
   {0x20000000, (PAGE_SIZE_1MB | CPLB_IDOCACHE)}, 

   // 512 MB (Maximum) SDRAM memory space 
   
   // CPLBs covering 64MB
   {0x00000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x00c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x01c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x02c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03000000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03400000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03800000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},
   {0x03c00000, (PAGE_SIZE_4MB | CPLB_IDOCACHE)},

   // Async Memory Bank 3 
   {0x20300000, (PAGE_SIZE_1MB | CPLB_INOCACHE)}, 

   // end of section - termination
   {0xffffffff, 0}, 
/*$VDSG<customizable-instr-cplb-table>                          */


}; /* icplbs_table */



#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


