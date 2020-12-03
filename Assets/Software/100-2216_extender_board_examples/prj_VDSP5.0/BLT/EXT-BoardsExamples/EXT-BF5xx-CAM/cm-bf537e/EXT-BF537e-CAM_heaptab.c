/* MANAGED-BY-SYSTEM-BUILDER                                    */
/* VisualDSP++ 5.0 Update 8                                     */
/* LDF Printer version: 5.8.0.3                                 */
/* ldfgen.exe version: 5.8.0.3                                  */
/* VDSG version: 5.8.0.3                                        */

/*
** User heap source file generated on Jul 06, 2010 at 14:55:21.
**
** Copyright (C) 2000-2010 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically based upon the options selected
** in the LDF Wizard. Changes to the LDF configuration should be made by
** changing the appropriate options rather than editing this file.
**
** Configuration:-
**     crt_doj:                                EXT-BF537e-CAM_basiccrt.doj
**     processor:                              ADSP-BF537
**     product_name:                           VisualDSP++ 5.0 Update 8
**     si_revision:                            0.3
**     default_silicon_revision_from_archdef:  0.3
**     using_cplusplus:                        true
**     mem_init:                               false
**     use_vdk:                                false
**     use_eh:                                 true
**     use_argv:                               true
**     running_from_internal_memory:           true
**     user_heap_src_file:                     S:\blackfin\VDK\prj_VDSP5.0\BLT\EXT-BoardsExamples\EXT-BF5xx-CAM\cm-bf537e\EXT-BF537e-CAM_heaptab.c
**     libraries_use_stdlib:                   true
**     libraries_use_fileio_libs:              false
**     libraries_use_ieeefp_emulation_libs:    false
**     libraries_use_eh_enabled_libs:          false
**     libraries_use_utility_rom:              true
**     system_heap:                            L3
**     system_heap_size:                       10M
**     system_stack:                           L1
**     system_stack_min_size:                  2k
**     use_sdram:                              true
**     use_sdram_size:                         32MB
**     use_sdram_partitioned:                  none
**
*/


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_1_1)
#pragma diag(suppress:misra_rule_2_2)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_8_10)
#pragma diag(suppress:misra_rule_10_1_a)
#pragma diag(suppress:misra_rule_11_3)
#pragma diag(suppress:misra_rule_12_7)
#else
#pragma diag(suppress:1124)
#endif /* _MISRA_RULES */



extern "asm" int ldf_heap_space;
extern "asm" int ldf_heap_length;


struct heap_table_t
{
  void          *base;
  unsigned long  length;
  long int       userid;
};

#pragma file_attr("libData=HeapTable")
#pragma section("constdata")
struct heap_table_t heap_table[2] =
{


  { &ldf_heap_space, (unsigned long) &ldf_heap_length, 0 },


  { 0, 0, 0 }
};



#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


