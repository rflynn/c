/*
Ryan Flynn
p-arsee-rror@g-mail (remove dashes)
Wed Apr 18 05:00:01 EDT 2008
*/

typedef int tym;
void a(tym *tym) { tym *x; } /* no problems here */

typedef int sym;
void b(sym *sym) { sym *y; } /* 'y' undeclared(?!) */

int main(void) { return 0; }

/*
Seems to be something wrong with types named 'sym'?

Error Message:
-------------------------------------------------------------
pizza@debian:~/c$ gcc -W -Wall -save-temps wtf.c
wtf.c: In function 'a':
wtf.c:187: warning: unused variable 'x'
wtf.c: At top level:
wtf.c:187: warning: unused parameter 'sym'
wtf.c: In function 'b':
wtf.c:190: error: 'y' undeclared (first use in this function)
wtf.c:190: error: (Each undeclared identifier is reported only once
wtf.c:190: error: for each function it appears in.)


GCC Version:
-------------------------------------------------------------
pizza@debian:~/c$ gcc --version | head -n 1
gcc (GCC) 4.1.2 20061115 (prerelease) (Debian 4.1.1-21)

Contents of wtf.i:
-------------------------------------------------------------
# 1 "wtf.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "wtf.c"
# 12 "wtf.c"
typedef int tym;
void a(tym *sym) { tym *x; }

typedef int sym;
void b(sym *sym) { sym *y; }

main() { return 0; }

More GCC info:

pizza@debian:~/proj/lng$ gcc -dumpversion
4.1.2
pizza@debian:~/proj/lng$ gcc -dumpmachine
i486-linux-gnu
pizza@debian:~/proj/lng$ gcc -dumpspecs
*asm:
%{v:-V} %{Qy:} %{!Qn:-Qy} %{n} %{T} %{Ym,*} %{Yd,*}  %{Wa,*:%*} %{m32:--32} %{m64:--64}

*asm_debug:
%{gstabs*:--gstabs}%{!gstabs*:%{g*:--gdwarf2}}

*asm_final:


*asm_options:
%a %Y %{c:%W{o*}%{!o*:-o %w%b%O}}%{!c:-o %d%w%u%O}

*invoke_as:
%{!S:-o %|.s |
 as %(asm_options) %|.s %A }

 *cpp:
 %{posix:-D_POSIX_SOURCE} %{pthread:-D_REENTRANT}

 *cpp_options:
 %(cpp_unique_options) %1 %{m*} %{std*&ansi&trigraphs} %{W*&pedantic*} %{w} %{f*} %{g*:%{!g0:%{!fno-working-directory:-fworking-directory}}} %{O*} %{undef} %{save-temps:-fpch-preprocess}

 *cpp_debug_options:
 %{d*}

 *cpp_unique_options:
 %{C|CC:%{!E:%eGCC does not support -C or -CC without -E}} %{!Q:-quiet} %{nostdinc*} %{C} %{CC} %{v} %{I*&F*} %{P} %I %{MD:-MD %{!o:%b.d}%{o*:%.d%*}} %{MMD:-MMD %{!o:%b.d}%{o*:%.d%*}} %{M} %{MM} %{MF*} %{MG} %{MP} %{MQ*} %{MT*} %{!E:%{!M:%{!MM:%{MD|MMD:%{o*:-MQ %*}}}}} %{remap} %{g3:-dD} %{H} %C %{D*&U*&A*} %{i*} %Z %i %{fmudflap:-D_MUDFLAP -include mf-runtime.h} %{fmudflapth:-D_MUDFLAP -D_MUDFLAPTH -include mf-runtime.h} %{E|M|MM:%W{o*}}

 *trad_capable_cpp:
 cc1 -E %{traditional|ftraditional|traditional-cpp:-traditional-cpp}

*cc1:
%(cc1_cpu) %{profile:-p}

*cc1_options:
%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} %1 %{!Q:-quiet} -dumpbase %B %{d*} %{m*} %{a*} %{c|S:%{o*:-auxbase-strip %*}%{!o*:-auxbase %b}}%{!c:%{!S:-auxbase %b}} %{g*} %{O*} %{W*&pedantic*} %{w} %{std*&ansi&trigraphs} %{v:-version} %{pg:-p} %{p} %{f*} %{undef} %{Qn:-fno-ident} %{--help:--help} %{--target-help:--target-help} %{!fsyntax-only:%{S:%W{o*}%{!o*:-o %b.s}}} %{fsyntax-only:-o %j} %{-param*} %{fmudflap|fmudflapth:-fno-builtin -fno-merge-constants} %{coverage:-fprofile-arcs -ftest-coverage}

*cc1plus:


*link_gcc_c_sequence:
%{static:--start-group} %G %L %{static:--end-group}%{!static:%G}

*link_ssp:
%{fstack-protector|fstack-protector-all:-lssp_nonshared -lssp}

*endfile:
%{ffast-math|funsafe-math-optimizations:crtfastmath.o%s}    %{shared|pie:crtendS.o%s;:crtend.o%s} crtn.o%s

*link:
%{!static:--eh-frame-hdr} %{m64:-m elf_x86_64} %{!m64:-m elf_i386}   %{shared:-shared}   %{!shared:     %{!static:       %{rdynamic:-export-dynamic}       %{!m64:%{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}}       %{m64:%{!dynamic-linker:-dynamic-linker /lib64/ld-linux-x86-64.so.2}}}     %{static:-static}}

*lib:
%{pthread:-lpthread}    %{shared:-lc}    %{!shared:%{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc}}

*mfwrap:
 %{static: %{fmudflap|fmudflapth:  --wrap=malloc --wrap=free --wrap=calloc --wrap=realloc --wrap=mmap --wrap=munmap --wrap=alloca} %{fmudflapth: --wrap=pthread_create}} %{fmudflap|fmudflapth: --wrap=main}

 *mflib:
 %{fmudflap|fmudflapth: -export-dynamic}

 *libgcc:
 %{static|static-libgcc:-lgcc -lgcc_eh}%{!static:%{!static-libgcc:%{!shared-libgcc:-lgcc --as-needed -lgcc_s --no-as-needed}%{shared-libgcc:-lgcc_s%{!shared: -lgcc}}}}

 *startfile:
 %{!shared: %{pg|p|profile:gcrt1.o%s;pie:Scrt1.o%s;:crt1.o%s}}    crti.o%s %{static:crtbeginT.o%s;shared|pie:crtbeginS.o%s;:crtbegin.o%s}

 *switches_need_spaces:


 *cross_compile:


on:
4.1.2

*multilib:
. !m64 !m32;64:../lib64 m64 !m32;32:../lib !m64 m32;

*multilib_defaults:
m32

*multilib_extra:


*multilib_matches:
m64 m64;m32 m32;

*multilib_exclusions:


*multilib_options:
m64/m32

*linker:
collect2

*link_libgcc:
%D

*md_exec_prefix:


*md_startfile_prefix:


*md_startfile_prefix_1:


*startfile_prefix_spec:


*sysroot_spec:
--sysroot=%R

*sysroot_suffix_spec:


*sysroot_hdrs_suffix_spec:


*cc1_cpu:
%{!mtune*: %{m386:mtune=i386 %n`-m386' is deprecated. Use `-march=i386' or `-mtune=i386' instead.
} %{m486:-mtune=i486 %n`-m486' is deprecated. Use `-march=i486' or `-mtune=i486' instead.
} %{mpentium:-mtune=pentium %n`-mpentium' is deprecated. Use `-march=pentium' or `-mtune=pentium' instead.
} %{mpentiumpro:-mtune=pentiumpro %n`-mpentiumpro' is deprecated. Use `-march=pentiumpro' or `-mtune=pentiumpro' instead.
} %{mcpu=*:-mtune=%* %n`-mcpu=' is deprecated. Use `-mtune=' or '-march=' instead.
}} %<mcpu=* %{mintel-syntax:-masm=intel %n`-mintel-syntax' is deprecated. Use `-masm=intel' instead.
} %{mno-intel-syntax:-masm=att %n`-mno-intel-syntax' is deprecated. Use `-masm=att' instead.
}

*link_emulation:
elf_i386

*dynamic_linker:
/lib/ld-linux.so.2

*link_command:
%{!fsyntax-only:%{!c:%{!M:%{!MM:%{!E:%{!S:    %(linker) %l %{pie:-pie} %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} %{r}    %{s} %{t} %{u*} %{x} %{z} %{Z} %{!A:%{!nostdlib:%{!nostartfiles:%S}}}    %{static:} %{L*} %(mfwrap) %(link_libgcc) %o %(mflib)    %{fprofile-arcs|fprofile-generate|coverage:-lgcov}    %{!nostdlib:%{!nodefaultlibs:%(link_ssp) %(link_gcc_c_sequence)}}    %{!A:%{!nostdlib:%{!nostartfiles:%E}}} %{T*} }}}}}}

*/
