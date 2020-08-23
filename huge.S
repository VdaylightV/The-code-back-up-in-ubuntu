.globl foo
foo:
  #128 MiB of nop
  .fill 1024 * 1024 *128, 1, 0x90
  ret
