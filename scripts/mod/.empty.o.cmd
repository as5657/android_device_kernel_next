cmd_scripts/mod/empty.o :=   gcc -Wp,-MD,scripts/mod/.empty.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/5/include -I/home/paul/Desktop/kernel/arch/x86/include -Iarch/x86/include/generated  -Iinclude -I/home/paul/Desktop/kernel/arch/x86/include/uapi -Iarch/x86/include/generated/uapi -I/home/paul/Desktop/kernel/include/uapi -Iinclude/generated/uapi -include /home/paul/Desktop/kernel/include/linux/kconfig.h -I/home/paul/Desktop/kernel/../vendor/hisi/modem/include/taf/ -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/modem/include/taf -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/ap/platform/hi3650 -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/audiodsp/include/ap/med -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/modem/include/taf -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/ap/platform/hi3650 -I/home/paul/Desktop/kernel/../kernel/drivers/vendor/hisi/audiodsp/include/ap/med -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wimplicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -DFEATURE_ON=1 -DFEATURE_OFF=0 -DBSP_CORE_APP -DBSP_COMPILE_ALLY -O2 -g -m64 -mno-mmx -mno-sse -mpreferred-stack-boundary=3 -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -maccumulate-outgoing-args -fstack-protector -DCONFIG_X86_X32_ABI -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -Wframe-larger-than=1024 -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-var-tracking-assignments -g -pg -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO    -DBSP_CORE_APP -DBSP_COMPILE_ALLY -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(empty)"  -D"KBUILD_MODNAME=KBUILD_STR(empty)" -c -o scripts/mod/.tmp_empty.o scripts/mod/empty.c

source_scripts/mod/empty.o := scripts/mod/empty.c

deps_scripts/mod/empty.o := \

scripts/mod/empty.o: $(deps_scripts/mod/empty.o)

$(deps_scripts/mod/empty.o):