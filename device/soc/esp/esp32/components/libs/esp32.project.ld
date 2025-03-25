/* Automatically generated file; DO NOT EDIT */
/* Espressif IoT Development Framework Linker Script */
/* Generated from: /home/syn/code/esp32/sdk/components/esp32/ld/esp32.project.ld.in */

/*  Default entry point:  */
ENTRY(call_start_cpu0);

SECTIONS
{
  /* RTC fast memory holds RTC wake stub code,
     including from any source file named rtc_wake_stub*.c
  */
  .rtc.text :
  {
    . = ALIGN(4);

    *(.rtc.literal .rtc.text .rtc.text.*)

    *rtc_wake_stub*.*(.literal .text .literal.* .text.*)
    _rtc_text_end = ABSOLUTE(.);
  } > rtc_iram_seg

  /*
    This section is required to skip rtc.text area because rtc_iram_seg and
    rtc_data_seg are reflect the same address space on different buses.
  */
  .rtc.dummy :
  {
    _rtc_dummy_start = ABSOLUTE(.);
    _rtc_fast_start = ABSOLUTE(.);
    . = SIZEOF(.rtc.text);
    _rtc_dummy_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* This section located in RTC FAST Memory area.
     It holds data marked with RTC_FAST_ATTR attribute.
     See the file "esp_attr.h" for more information.
  */
  .rtc.force_fast :
  {
    . = ALIGN(4);
    _rtc_force_fast_start = ABSOLUTE(.);

    _coredump_rtc_fast_start = ABSOLUTE(.);
    *(.rtc.fast.coredump .rtc.fast.coredump.*)
    _coredump_rtc_fast_end = ABSOLUTE(.);

    *(.rtc.force_fast .rtc.force_fast.*)
    . = ALIGN(4) ;

    _rtc_force_fast_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* RTC data section holds RTC wake stub
     data/rodata, including from any source file
     named rtc_wake_stub*.c and the data marked with
     RTC_DATA_ATTR, RTC_RODATA_ATTR attributes.
     The memory location of the data is dependent on
     CONFIG_ESP32_RTCDATA_IN_FAST_MEM option.
  */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);

    /* coredump mapping */
    _coredump_rtc_start = ABSOLUTE(.);
    *(.rtc.coredump .rtc.coredump.*)
    _coredump_rtc_end = ABSOLUTE(.);

    /* should be placed after coredump mapping */
    *(.rtc.data .rtc.data.*)
    *(.rtc.rodata .rtc.rodata.*)

    *rtc_wake_stub*.*(.data .rodata .data.* .rodata.* .bss .bss.*)
    _rtc_data_end = ABSOLUTE(.);

  } > rtc_data_location

  /* RTC bss, from any source file named rtc_wake_stub*.c */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);
    *rtc_wake_stub*.*(.bss .bss.*)
    *rtc_wake_stub*.*(COMMON)

    *(.rtc.bss)

    _rtc_bss_end = ABSOLUTE(.);
  } > rtc_data_location

  /* This section holds data that should not be initialized at power up
     and will be retained during deep sleep.
     User data marked with RTC_NOINIT_ATTR will be placed
     into this section. See the file "esp_attr.h" for more information.
	 The memory location of the data is dependent on
     CONFIG_ESP32_RTCDATA_IN_FAST_MEM option.
  */
  .rtc_noinit (NOLOAD):
  {
    . = ALIGN(4);
    _rtc_noinit_start = ABSOLUTE(.);
    *(.rtc_noinit .rtc_noinit.*)
    . = ALIGN(4) ;
    _rtc_noinit_end = ABSOLUTE(.);
  } > rtc_data_location

  /* This section located in RTC SLOW Memory area.
     It holds data marked with RTC_SLOW_ATTR attribute.
     See the file "esp_attr.h" for more information.
  */
  .rtc.force_slow :
  {
    . = ALIGN(4);
    _rtc_force_slow_start = ABSOLUTE(.);
    *(.rtc.force_slow .rtc.force_slow.*)
    . = ALIGN(4) ;
    _rtc_force_slow_end = ABSOLUTE(.);
  } > rtc_slow_seg

  /* Get size of rtc slow data based on rtc_data_location alias */
  _rtc_slow_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_slow_end - _rtc_data_start)
                        : (_rtc_force_slow_end - _rtc_force_slow_start);

  _rtc_fast_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_fast_end - _rtc_fast_start)
                        : (_rtc_noinit_end - _rtc_fast_start);

  ASSERT((_rtc_slow_length <= LENGTH(rtc_slow_seg)),
          "RTC_SLOW segment data does not fit.")

  ASSERT((_rtc_fast_length <= LENGTH(rtc_data_seg)),
          "RTC_FAST segment data does not fit.")

  /* Send .iram0 code to iram */
  .iram0.vectors :
  {
    _iram_start = ABSOLUTE(.);
    /* Vectors go to IRAM */
    _vector_table = ABSOLUTE(.);
    /* Vectors according to builds/RF-2015.2-win32/esp108_v1_2_s5_512int_2/config.html */
    . = 0x0;
    KEEP(*(.WindowVectors.text));
    . = 0x180;
    KEEP(*(.Level2InterruptVector.text));
    . = 0x1c0;
    KEEP(*(.Level3InterruptVector.text));
    . = 0x200;
    KEEP(*(.Level4InterruptVector.text));
    . = 0x240;
    KEEP(*(.Level5InterruptVector.text));
    . = 0x280;
    KEEP(*(.DebugExceptionVector.text));
    . = 0x2c0;
    KEEP(*(.NMIExceptionVector.text));
    . = 0x300;
    KEEP(*(.KernelExceptionVector.text));
    . = 0x340;
    KEEP(*(.UserExceptionVector.text));
    . = 0x3C0;
    KEEP(*(.DoubleExceptionVector.text));
    . = 0x400;
    _invalid_pc_placeholder = ABSOLUTE(.);
    *(.*Vector.literal)

    *(.UserEnter.literal);
    *(.UserEnter.text);
    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)

    _init_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.text :
  {
    /* Code marked as runnning out of IRAM */
    _iram_text_start = ABSOLUTE(.);

    *(.iram1 .iram1.*)
    *app_trace.a:*app_trace.*(.literal .literal.* .text .text.*)
    *app_trace.a:*app_trace_util.*(.literal .literal.* .text .text.*)
    *esp_event.a:*default_event_loop.*(.literal.esp_event_isr_post .text.esp_event_isr_post)
    *esp_event.a:*esp_event.*(.literal.esp_event_isr_post_to .text.esp_event_isr_post_to)
    *esp_hw_support.a:*cpu_util.*(.literal .literal.* .text .text.*)
    *esp_hw_support.a:*rtc_clk.*(.literal .literal.* .text .text.*)
    *esp_hw_support.a:*rtc_init.*(.literal.rtc_vddsdio_set_config .text.rtc_vddsdio_set_config)
    *esp_hw_support.a:*rtc_pm.*(.literal .literal.* .text .text.*)
    *esp_hw_support.a:*rtc_sleep.*(.literal .literal.* .text .text.*)
    *esp_hw_support.a:*rtc_time.*(.literal .literal.* .text .text.*)
    *esp_hw_support.a:*rtc_wdt.*(.literal .literal.* .text .text.*)
    *esp_ringbuf.a:(.literal .literal.* .text .text.*)
    *esp_system.a:*panic.*(.literal .literal.* .text .text.*)
    *esp_system.a:*panic_arch.*(.literal .literal.* .text .text.*)
    *esp_system.a:*panic_handler.*(.literal .literal.* .text .text.*)
    *esp_system.a:*reset_reason.*(.literal .literal.* .text .text.*)
    *esp_system.a:*system_api.*(.literal.esp_system_abort .text.esp_system_abort)
    *esp_osal.a:(.literal .literal.* .text .text.*)
    *libgcc.a:_divsf3.*(.literal .literal.* .text .text.*)
    *libgcc.a:lib2funcs.*(.literal .literal.* .text .text.*)
    *libgcov.a:(.literal .literal.* .text .text.*)
    *hal.a:*cpu_hal.*(.literal .literal.* .text .text.*)
    *hal.a:*i2c_hal_iram.*(.literal .literal.* .text .text.*)
    *hal.a:*ledc_hal_iram.*(.literal .literal.* .text .text.*)
    *hal.a:*soc_hal.*(.literal .literal.* .text .text.*)
    *hal.a:*spi_flash_hal_iram.*(.literal .literal.* .text .text.*)
    *hal.a:*spi_hal_iram.*(.literal .literal.* .text .text.*)
    *hal.a:*spi_slave_hal_iram.*(.literal .literal.* .text .text.*)
    *hal.a:*wdt_hal_iram.*(.literal .literal.* .text .text.*)
    *heap.a:*heap_tlsf.*(.literal .literal.* .text .text.*)
    *heap.a:*multi_heap.*(.literal .literal.* .text .text.*)
    *log.a:(.literal .literal.* .text .text.*)
    *net80211.a:(.wifi0iram .wifi0iram.*)
    *net80211.a:(.wifirxiram .wifirxiram.*)
    *net80211.a:(.wifislprxiram .wifislprxiram.*)
    *newlib.a:*abort.*(.literal .literal.* .text .text.*)
    *newlib.a:*heap.*(.literal .literal.* .text .text.*)
    *pp.a:(.wifi0iram .wifi0iram.*)
    *pp.a:(.wifiorslpiram .wifiorslpiram.*)
    *pp.a:(.wifirxiram .wifirxiram.*)
    *pp.a:(.wifislprxiram .wifislprxiram.*)
    *rtc.a:(.literal .literal.* .text .text.*)
    *soc.a:*lldesc.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*memspi_host_driver.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_chip_gd.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_chip_generic.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_chip_issi.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_chip_mxic.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_chip_winbond.*(.literal .literal.* .text .text.*)
    *spi_flash.a:*spi_flash_rom_patch.*(.literal .literal.* .text .text.*)
    *xt_hal.a:(.literal .literal.* .text .text.*)
    *xtensa.a:*eri.*(.literal .literal.* .text .text.*)
    *xtensa.a:*xtensa_intr_asm.*(.literal .literal.* .text .text.*)

  } > iram0_0_seg

  .dram0.data :
  {
    _data_start = ABSOLUTE(.);
    _bt_data_start = ABSOLUTE(.);
    *bt.a:(.data .data.*)
    . = ALIGN (4);
    _bt_data_end = ABSOLUTE(.);
    _btdm_data_start = ABSOLUTE(.);
    *btdm_app.a:(.data .data.*)
    . = ALIGN (4);
    _btdm_data_end = ABSOLUTE(.);
    _nimble_data_start = ABSOLUTE(.);
    *nimble.a:(.data .data.*)
    . = ALIGN (4);
    _nimble_data_end = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)

    /* coredump mapping */
    _coredump_dram_start = ABSOLUTE(.);
    *(.dram1.coredump .dram1.coredump.*)
    _coredump_dram_end = ABSOLUTE(.);

    /* should be placed after coredump mapping */
    _esp_system_init_fn_array_start = ABSOLUTE(.);
    KEEP (*(SORT(.esp_system_init_fn) SORT(.esp_system_init_fn.*)))
    _esp_system_init_fn_array_end = ABSOLUTE(.);

    *(.data .data.*)
    *(.dram1 .dram1.*)
    *app_trace.a:*app_trace.*(.rodata .rodata.*)
    *app_trace.a:*app_trace_util.*(.rodata .rodata.*)
    *esp_hw_support.a:*rtc_clk.*(.rodata .rodata.*)
    *esp_system.a:*panic.*(.rodata .rodata.*)
    *esp_system.a:*panic_arch.*(.rodata .rodata.*)
    *esp_system.a:*panic_handler.*(.rodata .rodata.*)
    *esp_system.a:*reset_reason.*(.rodata .rodata.*)
    *libgcc.a:_divsf3.*(.rodata .rodata.*)
    *libgcov.a:(.rodata .rodata.*)
    *hal.a:*cpu_hal.*(.rodata .rodata.*)
    *hal.a:*i2c_hal_iram.*(.rodata .rodata.*)
    *hal.a:*ledc_hal_iram.*(.rodata .rodata.*)
    *hal.a:*soc_hal.*(.rodata .rodata.*)
    *hal.a:*spi_flash_hal_iram.*(.rodata .rodata.*)
    *hal.a:*spi_hal_iram.*(.rodata .rodata.*)
    *hal.a:*spi_slave_hal_iram.*(.rodata .rodata.*)
    *hal.a:*wdt_hal_iram.*(.rodata .rodata.*)
    *heap.a:*heap_tlsf.*(.rodata .rodata.*)
    *heap.a:*multi_heap.*(.rodata .rodata.*)
    *newlib.a:*abort.*(.rodata .rodata.*)
    *newlib.a:*heap.*(.rodata .rodata.*)
    *phy.a:(.rodata .rodata.*)
    *soc.a:*lldesc.*(.rodata .rodata.*)
    *spi_flash.a:*memspi_host_driver.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_chip_gd.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_chip_generic.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_chip_issi.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_chip_mxic.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_chip_winbond.*(.rodata .rodata.*)
    *spi_flash.a:*spi_flash_rom_patch.*(.rodata .rodata.*)
    *kernel.a:(.rodata .rodata.*)
    *arch.a:(.rodata .rodata.*)
    . = ALIGN(4);
    _edata = ABSOLUTE(.);
    _data_end = ABSOLUTE(.);
    . = ALIGN(4);
  } > dram0_0_seg

  /*This section holds data that should not be initialized at power up.
    The section located in Internal SRAM memory region. The macro _NOINIT
    can be used as attribute to place data into this section.
    See the esp_attr.h file for more information.
  */
  .noinit (NOLOAD):
  {
    . = ALIGN(4);
    _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    . = ALIGN(4) ;
    _noinit_end = ABSOLUTE(.);
  } > dram0_0_seg

   /* external memory bss, from any global variable with EXT_RAM_ATTR attribute*/
  .ext_ram.bss (NOLOAD) :
  {
    _ext_ram_bss_start = ABSOLUTE(.);


    . = ALIGN(4);
    _ext_ram_bss_end = ABSOLUTE(.);
  } > extern_ram_seg

  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    _bt_bss_start = ABSOLUTE(.);
    *bt.a:(.bss .bss.* COMMON)
    . = ALIGN (4);
    _bt_bss_end = ABSOLUTE(.);
    _btdm_bss_start = ABSOLUTE(.);
    *btdm_app.a:(.bss .bss.* COMMON)
    . = ALIGN (4);
    _btdm_bss_end = ABSOLUTE(.);
    _nimble_bss_start = ABSOLUTE(.);
    *nimble.a:(.bss .bss.* COMMON)
    . = ALIGN (4);
    _nimble_bss_end = ABSOLUTE(.);

    *(.bss .bss.*)
    *(.ext_ram.bss .ext_ram.bss.*)
    *(.dynbss .dynsbss .gnu.linkonce.b .gnu.linkonce.b.* .gnu.linkonce.sb .gnu.linkonce.sb.* .gnu.linkonce.sb2 .gnu.linkonce.sb2.* .sbss .sbss.* .sbss2 .sbss2.* .scommon .share.mem)
    *(COMMON)
    . = ALIGN (8);
    __heap_start = ABSOLUTE(.);
	. += 0x8;/* 0x20000; */
    __heap_end = ABSOLUTE(.);
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
  } > dram0_0_seg

  ASSERT(((_bss_end - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")

  .flash.appdesc : ALIGN(0x10)
  {
    _rodata_start = ABSOLUTE(.);

    *(.rodata_desc .rodata_desc.*)               /* Should be the first.  App version info.        DO NOT PUT ANYTHING BEFORE IT! */
    *(.rodata_custom_desc .rodata_custom_desc.*) /* Should be the second. Custom app version info. DO NOT PUT ANYTHING BEFORE IT! */

    /* Create an empty gap within this section. Thanks to this, the end of this
     * section will match .flah.rodata's begin address. Thus, both sections
     * will be merged when creating the final bin image. */
    . = ALIGN(ALIGNOF(.flash.rodata));
  } >default_rodata_seg

  .flash.rodata : ALIGN(0x10)
  {
    _flash_rodata_start = ABSOLUTE(.);

    *(EXCLUDE_FILE(*libgcov.a *phy.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_hw_support.a:*rtc_clk.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *libgcc.a:*_divsf3.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.*) .rodata EXCLUDE_FILE(*libgcov.a *phy.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_hw_support.a:*rtc_clk.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *libgcc.a:*_divsf3.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.*) .rodata.*)
    *(.rodata_wlog_error .rodata_wlog_error.*)
    *(.rodata_wlog_info .rodata_wlog_info.*)
    *(.rodata_wlog_warning .rodata_wlog_warning.*)


    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    . = (. + 3) & ~ 3;
    __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    . = (. + 7) & ~ 3;
    /*  C++ constructor and destructor tables

        Make a point of not including anything from crtbegin.o or crtend.o, as IDF doesn't use toolchain crt
      */
    __init_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors SORT(.ctors.*)))
    __init_array_end = ABSOLUTE(.);

    KEEP (*crtbegin.*(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.*) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    /* Addresses of memory regions reserved via
       SOC_RESERVE_MEMORY_REGION() */
    soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    . = ALIGN(4);
    _thread_local_start = ABSOLUTE(.);
    *(.tdata)
    *(.tdata.*)
    *(.tbss)
    *(.tbss.*)
    _thread_local_end = ABSOLUTE(.);
    . = ALIGN(4);
  } >default_rodata_seg

  _flash_rodata_align = ALIGNOF(.flash.rodata);

  .flash.rodata_noload (NOLOAD) :
  {
    . = ALIGN (4);
    *(.rodata_wlog_debug .rodata_wlog_debug.*)
    *(.rodata_wlog_verbose .rodata_wlog_verbose.*)
    _flash_rodata_end = ABSOLUTE(.);
  } > default_rodata_seg

  .flash.text :
  {
    _stext = .;
    _text_start = ABSOLUTE(.);

    *(.phyiram .phyiram.*)
    *(EXCLUDE_FILE(*esp_ringbuf.a *esp_osal.a *libgcov.a *rtc.a *xt_hal.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_event.a:*default_event_loop.* *esp_event.a:*esp_event.* *esp_hw_support.a:*cpu_util.* *esp_hw_support.a:*rtc_clk.* *esp_hw_support.a:*rtc_init.* *esp_hw_support.a:*rtc_pm.* *esp_hw_support.a:*rtc_sleep.* *esp_hw_support.a:*rtc_time.* *esp_hw_support.a:*rtc_wdt.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *esp_system.a:*system_api.* *libgcc.a:*_divsf3.* *libgcc.a:*lib2funcs.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *log.a:*log.* *log.a:*log_freertos.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.* *xtensa.a:*eri.* *xtensa.a:*xtensa_intr_asm.*) .literal EXCLUDE_FILE(*esp_ringbuf.a *esp_osal.a *libgcov.a *rtc.a *xt_hal.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_event.a:*default_event_loop.* *esp_event.a:*esp_event.* *esp_hw_support.a:*cpu_util.* *esp_hw_support.a:*rtc_clk.* *esp_hw_support.a:*rtc_init.* *esp_hw_support.a:*rtc_pm.* *esp_hw_support.a:*rtc_sleep.* *esp_hw_support.a:*rtc_time.* *esp_hw_support.a:*rtc_wdt.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *esp_system.a:*system_api.* *libgcc.a:*_divsf3.* *libgcc.a:*lib2funcs.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *log.a:*log.* *log.a:*log_freertos.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.* *xtensa.a:*eri.* *xtensa.a:*xtensa_intr_asm.*) .literal.* EXCLUDE_FILE(*esp_ringbuf.a *esp_osal.a *libgcov.a *rtc.a *xt_hal.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_event.a:*default_event_loop.* *esp_event.a:*esp_event.* *esp_hw_support.a:*cpu_util.* *esp_hw_support.a:*rtc_clk.* *esp_hw_support.a:*rtc_init.* *esp_hw_support.a:*rtc_pm.* *esp_hw_support.a:*rtc_sleep.* *esp_hw_support.a:*rtc_time.* *esp_hw_support.a:*rtc_wdt.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *esp_system.a:*system_api.* *libgcc.a:*_divsf3.* *libgcc.a:*lib2funcs.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *log.a:*log.* *log.a:*log_freertos.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.* *xtensa.a:*eri.* *xtensa.a:*xtensa_intr_asm.*) .text EXCLUDE_FILE(*esp_ringbuf.a *esp_osal.a *libgcov.a *rtc.a *xt_hal.a *app_trace.a:*app_trace.* *app_trace.a:*app_trace_util.* *esp_event.a:*default_event_loop.* *esp_event.a:*esp_event.* *esp_hw_support.a:*cpu_util.* *esp_hw_support.a:*rtc_clk.* *esp_hw_support.a:*rtc_init.* *esp_hw_support.a:*rtc_pm.* *esp_hw_support.a:*rtc_sleep.* *esp_hw_support.a:*rtc_time.* *esp_hw_support.a:*rtc_wdt.* *esp_system.a:*panic.* *esp_system.a:*panic_arch.* *esp_system.a:*panic_handler.* *esp_system.a:*reset_reason.* *esp_system.a:*system_api.* *libgcc.a:*_divsf3.* *libgcc.a:*lib2funcs.* *hal.a:*cpu_hal.* *hal.a:*i2c_hal_iram.* *hal.a:*ledc_hal_iram.* *hal.a:*soc_hal.* *hal.a:*spi_flash_hal_iram.* *hal.a:*spi_hal_iram.* *hal.a:*spi_slave_hal_iram.* *hal.a:*wdt_hal_iram.* *heap.a:*heap_tlsf.* *heap.a:*multi_heap.* *log.a:*log.* *log.a:*log_freertos.* *newlib.a:*abort.* *newlib.a:*heap.* *soc.a:*lldesc.* *spi_flash.a:*memspi_host_driver.* *spi_flash.a:*spi_flash_chip_gd.* *spi_flash.a:*spi_flash_chip_generic.* *spi_flash.a:*spi_flash_chip_issi.* *spi_flash.a:*spi_flash_chip_mxic.* *spi_flash.a:*spi_flash_chip_winbond.* *spi_flash.a:*spi_flash_rom_patch.* *xtensa.a:*eri.* *xtensa.a:*xtensa_intr_asm.*) .text.*)
     *(EXCLUDE_FILE(*net80211.a *pp.a) .wifi0iram EXCLUDE_FILE(*net80211.a *pp.a) .wifi0iram.*)
    *(EXCLUDE_FILE(*pp.a) .wifiorslpiram EXCLUDE_FILE(*pp.a) .wifiorslpiram.*)
    *(EXCLUDE_FILE(*net80211.a *pp.a) .wifirxiram EXCLUDE_FILE(*net80211.a *pp.a) .wifirxiram.*)
    *(.wifislpiram .wifislpiram.*)
    *(EXCLUDE_FILE(*net80211.a *pp.a) .wifislprxiram EXCLUDE_FILE(*net80211.a *pp.a) .wifislprxiram.*)
    *esp_event.a:*default_event_loop.*(.literal.esp_event_handler_instance_register .literal.esp_event_handler_instance_unregister .literal.esp_event_handler_register .literal.esp_event_handler_unregister .literal.esp_event_loop_create_default .literal.esp_event_loop_delete_default .literal.esp_event_post .literal.esp_event_send_to_default_loop .text .text.esp_event_handler_instance_register .text.esp_event_handler_instance_unregister .text.esp_event_handler_register .text.esp_event_handler_unregister .text.esp_event_loop_create_default .text.esp_event_loop_delete_default .text.esp_event_post .text.esp_event_send_to_default_loop)
    *esp_event.a:*esp_event.*(.literal.base_node_add_handler .literal.esp_event_handler_instance_register_with .literal.esp_event_handler_instance_unregister_with .literal.esp_event_handler_register_with .literal.esp_event_handler_register_with_internal .literal.esp_event_handler_unregister_with .literal.esp_event_handler_unregister_with_internal .literal.esp_event_loop_create .literal.esp_event_loop_delete .literal.esp_event_loop_run .literal.esp_event_loop_run_task .literal.esp_event_post_to .literal.handler_instances_add .literal.handler_instances_remove .literal.handler_instances_remove_all$isra$1 .literal.loop_node_add_handler .text .text.base_node_add_handler .text.esp_event_dump .text.esp_event_handler_instance_register_with .text.esp_event_handler_instance_unregister_with .text.esp_event_handler_register_with .text.esp_event_handler_register_with_internal .text.esp_event_handler_unregister_with .text.esp_event_handler_unregister_with_internal .text.esp_event_loop_create .text.esp_event_loop_delete .text.esp_event_loop_run .text.esp_event_loop_run_task .text.esp_event_post_to .text.handler_instances_add .text.handler_instances_remove .text.handler_instances_remove_all$isra$1 .text.loop_node_add_handler)
    *esp_hw_support.a:*rtc_init.*(.literal.rtc_init .literal.rtc_vddsdio_get_config .text .text.rtc_init .text.rtc_vddsdio_get_config)
    *esp_system.a:*system_api.*(.literal.esp_get_free_heap_size .literal.esp_get_free_internal_heap_size .literal.esp_get_idf_version .literal.esp_get_minimum_free_heap_size .literal.esp_register_shutdown_handler .literal.esp_unregister_shutdown_handler .text .text.esp_get_free_heap_size .text.esp_get_free_internal_heap_size .text.esp_get_idf_version .text.esp_get_minimum_free_heap_size .text.esp_register_shutdown_handler .text.esp_unregister_shutdown_handler)
    *esp_osal.a:*port.*(.literal.esp_startup_start_app .text.esp_startup_start_app)
    *esp_osal.a:*port_common.*(.literal.main_task .text.main_task)
    *log.a:*log.*(.literal.esp_log_level_set .literal.esp_log_set_vprintf .literal.esp_log_writev .literal.heap_bubble_down .text .text.esp_log_level_set .text.esp_log_set_vprintf .text.esp_log_writev .text.heap_bubble_down)
    *log.a:*log_freertos.*(.literal.esp_log_system_timestamp .text .text.esp_log_system_timestamp)

    *(.stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)

/* -------------------------------------------------------- */
    . = ALIGN(4);
    __zinitcall_bsp_start = .;
    KEEP (*(.zinitcall.bsp0.init))
    KEEP (*(.zinitcall.bsp1.init))
    KEEP (*(.zinitcall.bsp2.init))
    KEEP (*(.zinitcall.bsp3.init))
    KEEP (*(.zinitcall.bsp4.init))
    __zinitcall_bsp_end = .;
    __zinitcall_device_start = .;
    KEEP (*(.zinitcall.device0.init))
    KEEP (*(.zinitcall.device1.init))
    KEEP (*(.zinitcall.device2.init))
    KEEP (*(.zinitcall.device3.init))
    KEEP (*(.zinitcall.device4.init))
    __zinitcall_device_end = .;
    __zinitcall_core_start = .;
    KEEP (*(.zinitcall.core0.init))
    KEEP (*(.zinitcall.core1.init))
    KEEP (*(.zinitcall.core2.init))
    KEEP (*(.zinitcall.core3.init))
    KEEP (*(.zinitcall.core4.init))
    __zinitcall_core_end = .;
    __zinitcall_sys_service_start = .;
    KEEP (*(.zinitcall.sys.service0.init))
    KEEP (*(.zinitcall.sys.service1.init))
    KEEP (*(.zinitcall.sys.service2.init))
    KEEP (*(.zinitcall.sys.service3.init))
    KEEP (*(.zinitcall.sys.service4.init))
    __zinitcall_sys_service_end = .;
    __zinitcall_sys_feature_start = .;
    KEEP (*(.zinitcall.sys.feature0.init))
    KEEP (*(.zinitcall.sys.feature1.init))
    KEEP (*(.zinitcall.sys.feature2.init))
    KEEP (*(.zinitcall.sys.feature3.init))
    KEEP (*(.zinitcall.sys.feature4.init))
    __zinitcall_sys_feature_end = .;
    __zinitcall_run_start = .;
    KEEP (*(.zinitcall.run0.init))
    KEEP (*(.zinitcall.run1.init))
    KEEP (*(.zinitcall.run2.init))
    KEEP (*(.zinitcall.run3.init))
    KEEP (*(.zinitcall.run4.init))
    __zinitcall_run_end = .;
    __zinitcall_app_service_start = .;
    KEEP (*(.zinitcall.app.service0.init))
    KEEP (*(.zinitcall.app.service1.init))
    KEEP (*(.zinitcall.app.service2.init))
    KEEP (*(.zinitcall.app.service3.init))
    KEEP (*(.zinitcall.app.service4.init))
    __zinitcall_app_service_end = .;
    __zinitcall_app_feature_start = .;
    KEEP (*(.zinitcall.app.feature0.init))
    KEEP (*(.zinitcall.app.feature1.init))
    KEEP (*(.zinitcall.app.feature2.init))
    KEEP (*(.zinitcall.app.feature3.init))
    KEEP (*(.zinitcall.app.feature4.init))
    __zinitcall_app_feature_end = .;
    __zinitcall_test_start = .;
    KEEP (*(.zinitcall.test0.init))
    KEEP (*(.zinitcall.test1.init))
    KEEP (*(.zinitcall.test2.init))
    KEEP (*(.zinitcall.test3.init))
    KEEP (*(.zinitcall.test4.init))
    __zinitcall_test_end = .;
    __zinitcall_exit_start = .;
    KEEP (*(.zinitcall.exit0.init))
    KEEP (*(.zinitcall.exit1.init))
    KEEP (*(.zinitcall.exit2.init))
    KEEP (*(.zinitcall.exit3.init))
    KEEP (*(.zinitcall.exit4.init))
    __zinitcall_exit_end = .;
    _hdf_drivers_start = .;
    KEEP (*(.hdf.driver))
    _hdf_drivers_end = .;

/* -------------------------------------------------------- */


    /** CPU will try to prefetch up to 16 bytes of
      * of instructions. This means that any configuration (e.g. MMU, PMS) must allow
      * safe access to up to 16 bytes after the last real instruction, add
      * dummy bytes to ensure this
      */
    . += 16;

    _text_end = ABSOLUTE(.);
    _etext = .;

    /* Similar to _iram_start, this symbol goes here so it is
       resolved by addr2line in preference to the first symbol in
       the flash.text segment.
    */
    _flash_cache_start = ABSOLUTE(0);
  } >default_code_seg

  /* Marks the end of IRAM code segment */
  .iram0.text_end (NOLOAD) :
  {
    . = ALIGN (4);
    _iram_text_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.data :
  {
    . = ALIGN(4);
    _iram_data_start = ABSOLUTE(.);

    /* coredump mapping */
    _coredump_iram_start = ABSOLUTE(.);
    *(.iram.data.coredump .iram.data.coredump.*)
    _coredump_iram_end = ABSOLUTE(.);

    /* should be placed after coredump mapping */
    *(.iram.data .iram.data.*)

    _iram_data_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.bss (NOLOAD) :
  {
    . = ALIGN(4);
    _iram_bss_start = ABSOLUTE(.);

    *(.iram.bss .iram.bss.*)

    _iram_bss_end = ABSOLUTE(.);
    . = ALIGN(4);
    _iram_end = ABSOLUTE(.);
   } > iram0_0_seg

  /* Marks the end of data, bss and possibly rodata  */
  .dram0.heap_start (NOLOAD) :
  {
    . = ALIGN (8);
    _heap_start = ABSOLUTE(.);
  } > dram0_0_seg
}

ASSERT(((_iram_end - ORIGIN(iram0_0_seg)) <= LENGTH(iram0_0_seg)),
          "IRAM0 segment data does not fit.")

ASSERT(((_heap_start - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")
