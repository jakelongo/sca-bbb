.text
.arm
.fpu neon

.global _neonExecute_INSTR
.global neonExecute_INSTR

.type   _neonExecute_INSTR,%function
.type   neonExecute_INSTR,%function

.extern setTrigger(void)
.extern clrTrigger(void)

.align 8

@ neonExecute_INSTR(uint8_t** memBank, uint32_t opIndex); 
neonExecute_INSTR:
_neonExecute_INSTR:
  stmdb   sp!,{r4-r12,lr}

  /* Get some variables in place and return var */
  mov r4, r0
  mov r5, r0
  mov r6, r1
  mov r7, #0

  /* load the memory banks */
  vld1.u8 {d2}, [r4]!; /* load memBank[0] */
  vld1.u8 {d3}, [r4]!; /* load memBank[1] */
  vld1.u8 {d4}, [r4]!; /* load memBank[2] */
  vld1.u8 {d5}, [r4]!; /* load memBank[3] */
  vld1.u8 {d6}, [r4]!; /* load memBank[4] */
  vld1.u8 {d7}, [r4]!; /* load memBank[5] */
  vld1.u8 {d8}, [r4]!; /* load memBank[6] */
  vld1.u8 {d9}, [r4]!; /* load memBank[7] */
  vld1.u8 {d10}, [r4]!; /* load memBank[8] */
  vld1.u8 {d11}, [r4]!; /* load memBank[9] */
  vld1.u8 {d12}, [r4]!; /* load memBank[10] */
  vld1.u8 {d13}, [r4]!; /* load memBank[11] */

  bl setTrigger

  .rept 1000
  mov r1, r1
  .endr

  .rept 1
  INSTR     d0, d1, d2
  veor.u32  d0, d3, d4
  .endr

  bl clrTrigger

  .rept 3000
  mov r1, r1
  .endr

  vst1.u8 {d2}, [r5]!; /* load memBank[0] */
  vst1.u8 {d3}, [r5]!; /* load memBank[1] */
  vst1.u8 {d4}, [r5]!; /* load memBank[2] */
  vst1.u8 {d5}, [r5]!; /* load memBank[3] */
  vst1.u8 {d6}, [r5]!; /* load memBank[4] */
  vst1.u8 {d7}, [r5]!; /* load memBank[5] */
  vst1.u8 {d8}, [r5]!; /* load memBank[6] */
  vst1.u8 {d9}, [r5]!; /* load memBank[7] */
  vst1.u8 {d10}, [r5]!; /* load memBank[8] */
  vst1.u8 {d11}, [r5]!; /* load memBank[9] */
  vst1.u8 {d12}, [r5]!; /* load memBank[10] */
  vst1.u8 {d13}, [r5]!; /* load memBank[11] */

  mov r0, r7

  ldmia   sp!,{r4-r12,pc}

.size _neonExecute_INSTR,.-_neonExecute_INSTR
.size neonExecute_INSTR,.-neonExecute_INSTR
