/*------------------------------------------------------------------------------
 *      Linker script for running in internal SRAM on the ATSAM3X8E
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/* Memory Spaces Definitions */
MEMORY
{
  rom (rx)  : ORIGIN = 0x00080000, LENGTH = 512K
  ram (rwx) : ORIGIN = 0x20070000, LENGTH = 64K + 32K
}

/* The stack size used by the application. NOTE: you need to adjust according to your application. */
STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 0x400;

/* The heapsize used by the application. NOTE: you need to adjust according to your application. */
HEAP_SIZE = DEFINED(HEAP_SIZE) ? HEAP_SIZE : 0x200;

INCLUDE sam3xa_sram.ld
