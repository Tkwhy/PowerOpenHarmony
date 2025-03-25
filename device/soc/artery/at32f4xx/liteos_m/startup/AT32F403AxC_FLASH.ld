/*
*****************************************************************************
**
**  File        : AT32F403AxG_FLASH.ld
**
**  Abstract    : Linker script for AT32F403AxG Device with
**                1000KByte FLASH, 96KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : Artery Tek AT32
**
**  Environment : Arm gcc toolchain
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20018000;    /* end of RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x400;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 992K /* all flash is 1000K, 0-8K is bootloader, 9-1000K is user code */
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 96K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    _stext = .;
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    _hdf_drivers_start = .;
    KEEP(*(.hdf.driver))
    _hdf_drivers_end = .; /* define a section for hdf driver */

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

 .zinitcall_array :
 {
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_core_start = .);
  KEEP (*(SORT(.zinitcall.core*)))
  KEEP (*(.zinitcall.core*))
  PROVIDE_HIDDEN (__zinitcall_core_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_device_start = .);
  KEEP (*(SORT(.zinitcall.device*)))
  KEEP (*(.zinitcall.device*))
  PROVIDE_HIDDEN (__zinitcall_device_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_bsp_start = .);
  KEEP (*(SORT(.zinitcall.bsp*)))
  KEEP (*(.zinitcall.bsp*))
  PROVIDE_HIDDEN (__zinitcall_bsp_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_sys_service_start = .);
  KEEP (*(SORT(.zinitcall.sys.service*)))
  KEEP (*(.zinitcall.sys.service*))
  PROVIDE_HIDDEN (__zinitcall_sys_service_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_app_service_start = .);
  KEEP (*(SORT(.zinitcall.app.service*)))
  KEEP (*(.zinitcall.app.service*))
  PROVIDE_HIDDEN (__zinitcall_app_service_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_sys_feature_start = .);
  KEEP (*(SORT(.zinitcall.sys.feature*)))
  KEEP (*(.zinitcall.sys.feature*))
  PROVIDE_HIDDEN (__zinitcall_sys_feature_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_app_feature_start = .);
  KEEP (*(SORT(.zinitcall.app.feature*)))
  KEEP (*(.zinitcall.app.feature*))
  PROVIDE_HIDDEN (__zinitcall_app_feature_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_run_start = .);
  KEEP (*(SORT(.zinitcall.run*)))
  KEEP (*(.zinitcall.run*))
  PROVIDE_HIDDEN (__zinitcall_run_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_test_start = .);
  KEEP (*(SORT(.zinitcall.test*)))
  KEEP (*(.zinitcall.test*))
  PROVIDE_HIDDEN (__zinitcall_test_end = .);
  . = ALIGN(0x4) ;
  PROVIDE_HIDDEN (__zinitcall_exit_start = .);
  KEEP (*(SORT(.zinitcall.exit*)))
  KEEP (*(.zinitcall.exit*))
  PROVIDE_HIDDEN (__zinitcall_exit_end = .);
 } > FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata.g_hdfConfigEntrySymbol)
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  . = ALIGN(0x1000);
  _flash_end = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  _sstack = .;
  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

   . = ALIGN(0x40);
  __stack_end = _estack;
  __stack_limit = _estack - _Min_Heap_Size - _Min_Stack_Size - 0x1400;

  __heap_start = _end + _Min_Heap_Size;
  __heap_end = __stack_limit - _Min_Heap_Size;
  __heap_lenth = __heap_end - __heap_start;
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
