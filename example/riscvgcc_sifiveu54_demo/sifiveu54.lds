OUTPUT_ARCH("riscv")

MEMORY
{
    IRAM (rwx)  : ORIGIN = 0x80000000, LENGTH = 0x80000000
}

PHDRS
{
    rom PT_LOAD;
    ram_init PT_LOAD;
    tls PT_TLS;
    ram PT_LOAD;
    itim_init PT_LOAD;
    text PT_LOAD;
    lim_init PT_LOAD;
}

ENTRY(_enter)

SECTIONS
{
    /* Each hart is allocated its own stack of size __stack_size. This value
     * can be overriden at build-time by adding the following to CFLAGS:
     *
     *     -Xlinker --defsym=__stack_size=0xf00
     *
     * where 0xf00 can be replaced with a multiple of 16 of your choice.
     *
     * __stack_size is PROVIDE-ed as a symbol so that initialization code
     * initializes the stack pointers for each hart at the right offset from
     * the _sp symbol.
     */
    __stack_size = DEFINED(__stack_size) ? __stack_size : 0x000F0000;
    PROVIDE(__stack_size = __stack_size);

    /* The size of the heap can be overriden at build-time by adding the
     * following to CFLAGS:
     *
     *     -Xlinker --defsym=__heap_size=0xf00
     *
     * where 0xf00 can be replaced with the value of your choice.
     *
     * Altertatively, the heap can be grown to fill the entire remaining region
     * of RAM by adding the following to CFLAGS:
     *
     *     -Xlinker --defsym=__heap_max=1
     *
     * Note that depending on the memory layout, the bitness (32/64bit) of the
     * target, and the code model in use, this might cause a relocation error.
     */
    __heap_size = DEFINED(__heap_size) ? __heap_size : 0x800;

    /* The boot hart sets which hart runs the pre-main initialization routines,
     * including copying .data into RAM, zeroing the BSS region, running
     * constructors, etc. After initialization, the boot hart is also the only
     * hart which runs application code unless the application overrides the
     * secondary_main() function to start execution on secondary harts.
     */
    PROVIDE(__boot_hart = 0);

    /* The chicken bit is used by pre-main initialization to enable/disable
     * certain core features */
    PROVIDE(__chicken_bit = 1);

    /* The memory_ecc_scrub bit is used by _entry code to enable/disable
     * memories scrubbing to zero  */
    PROVIDE(__eccscrub_bit = 0);

    /* ROM SECTION
     *
     * The following sections contain data which lives in read-only memory, if
     * such memory is present in the design, for the entire duration of program
     * execution.
     */

    .init : {
        /* The _enter symbol is placed in the .text.init.enter section
         * and must be placed at the beginning of the program */
        KEEP (*(.text.init.enter))
        KEEP (*(.text.init.*))
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(.text.start))
    } >IRAM :rom

    .fini : {
        KEEP (*(SORT_NONE(.fini)))
    } >IRAM :rom

    .preinit_array : ALIGN(8) {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >IRAM :rom

    .init_array : ALIGN(8) {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
        PROVIDE_HIDDEN (__constructors_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.hal.init_array.*)));
        KEEP (*(.hal.init_array));
        PROVIDE_HIDDEN (__constructors_end = .);
    } >IRAM :rom

    .fini_array : ALIGN(8) {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
        PROVIDE_HIDDEN (__destructors_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.hal.fini_array.*)));
        KEEP (*(.hal.fini_array));
        PROVIDE_HIDDEN (__destructors_end = .);
        KEEP (*(SORT(.rte_shell*)))
    } >IRAM :rom

	.privileged_functions : ALIGN (32) {
        __privileged_functions_start__ = .;
        KEEP(*(privileged_functions))
        . = ALIGN(32);
		__privileged_functions_end__ = .;
	} >IRAM

    .ctors : {
        . = ALIGN(32);
        __unprivileged_section_start__ = .;
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
        KEEP (*(.hal.ctors .hal.ctors.*))
    } >IRAM :rom

    .dtors : {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        KEEP (*(.hal.dtors .hal.dtors.*))
    } >IRAM : rom

    .rodata : {
        *(.rdata)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(8);
        *(.srodata.cst16)
        *(.srodata.cst8)
        *(.srodata.cst4)
        *(.srodata.cst2)
        *(.srodata .srodata.*)
    } >IRAM :rom

    /* ITIM SECTION
     *
     * The following sections contain data which is copied from read-only
     * memory into an instruction tightly-integrated memory (ITIM), if one
     * is present in the design, during pre-main program initialization.
     *
     * Generally, the data copied into the ITIM should be performance-critical
     * functions which benefit from low instruction-fetch latency.
     */
    .itim : ALIGN(8) {
        *(.itim .itim.*)
    } >IRAM AT>IRAM :itim_init

    PROVIDE( __segment_itim_source_start = LOADADDR(.itim) );
    PROVIDE( __segment_itim_target_start = ADDR(.itim) );
    PROVIDE( __segment_itim_target_end = ADDR(.itim) + SIZEOF(.itim) );

    /* LIM SECTION
     *
     * The following sections contain data which is copied from read-only
     * memory into a loosely integrated memory (LIM), which is shared with L2
     * cache, during pre-main program initialization.
     *
     * Generally, the data copied into the LIM should be performance-critical
     * functions which benefit from low instruction-fetch latency.
     */
    .lim : ALIGN(8) {
        *(.lim .lim.*)
    } >IRAM AT>IRAM :lim_init

    PROVIDE( __segment_lim_source_start = LOADADDR(.lim) );
    PROVIDE( __segment_lim_target_start = ADDR(.lim) );
    PROVIDE( __segment_lim_target_end = ADDR(.lim) + SIZEOF(.lim) );

    /* TEXT SECTION
     *
     * The following section contains the code of the program, excluding
     * everything that's been allocated into the ITIM/LIM already
     */
    .text : {
        *(.text.unlikely .text.unlikely.*)
        *(.text.startup .text.startup.*)
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
        *(freertos_system_calls)
        . = ALIGN(32);
        __unprivileged_section_end__ = .;
    } >IRAM :text

    /* RAM SECTION
     *
     * The following sections contain data which is copied from read-only
     * memory into a read-write-capable memory such as data tightly-integrated
     * memory (DTIM) or another main memory, as well as the BSS, stack, and
     * heap.
     *
     * You might notice that .data, .tdata, .tbss, .tbss_space, and .bss all
     * have an apparently unnecessary ALIGN at their top. This is because
     * the implementation of _start in Freedom Metal libgloss depends on the
     * ADDR and LOADADDR being 8-byte aligned.
     */

    .data : ALIGN(8) {
        . = ALIGN(32);
        __unprivileged_data_section_start__ = .;
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata2.*)
        *(.gnu.linkonce.s.*)
    } >IRAM AT>IRAM :ram_init

    .tdata : ALIGN(8) {
        PROVIDE( __tls_base = . );
	*(.tdata .tdata.* .gnu.linkonce.td.*)
    } >IRAM AT>IRAM :tls :ram_init

    PROVIDE( __tdata_source = LOADADDR(.tdata) );
    PROVIDE( __tdata_size = SIZEOF(.tdata) );

    PROVIDE( __segment_data_source_start = LOADADDR(.data) );
    PROVIDE( __segment_data_target_start = ADDR(.data) );
    PROVIDE( __segment_data_target_end = ADDR(.tdata) + SIZEOF(.tdata) );

    .tbss : ALIGN(8) {
	*(.tbss .tbss.* .gnu.linkonce.tb.*)
	*(.tcommon .tcommon.*)
	PROVIDE( __tls_end = . );
    } >IRAM AT>IRAM :tls :ram
    PROVIDE( __tbss_size = SIZEOF(.tbss) );
    PROVIDE( __tls_size = __tls_end - __tls_base );

    .tbss_space : ALIGN(8) {
	. = . + __tbss_size;
    } >IRAM :ram

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(32);
        __unprivileged_data_section_end__ = .;
    } >IRAM :ram

    PROVIDE( __segment_bss_source_start = LOADADDR(.tbss) );
    PROVIDE( __segment_bss_target_start = ADDR(.tbss) );
    PROVIDE( __segment_bss_target_end = ADDR(.bss) + SIZEOF(.bss) );

    .privileged_data (NOLOAD) : ALIGN(32) {
        __privileged_data_start__ = .;
		*(privileged_data)
		/* Non kernel data is kept out of the first _Privileged_Data_Region_Size
		bytes of SRAM. */
        . = ALIGN(32);
		__privileged_data_end__ = .;
	} >IRAM

    .stack (NOLOAD) : ALIGN(16) {
        PROVIDE(__segment_stack_begin = .);
        . += __stack_size; /* Hart 0 */
        PROVIDE( _sp = . );
        /* Hart 1 */
        . += __stack_size;
        /* Hart 2 */
        . += __stack_size;
        /* Hart 3 */
        . += __stack_size;
        PROVIDE(__segment_stack_end = .);
    } >IRAM :ram

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __end = . );
        PROVIDE( __heap_start = . );
        PROVIDE( __segment_heap_target_start = . );
        /* If __heap_max is defined, grow the heap to use the rest of RAM,
         * otherwise set the heap size to __heap_size */
        . = DEFINED(__heap_max) ? MIN( LENGTH(ram) - ( . - ORIGIN(ram)) , 0x10000000) : __heap_size;
        PROVIDE( __segment_heap_target_end = . );
        PROVIDE( _heap_end = . );
        PROVIDE( __heap_end = . );
    } >IRAM :ram

    /* C++ exception handling information is
     * not useful with our current runtime environment,
     * and it consumes flash space. Discard it until
     * we have something that can use it
     */
    /DISCARD/ : {
	*(.eh_frame .eh_frame.*)
    }
}