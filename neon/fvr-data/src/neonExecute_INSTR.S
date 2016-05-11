.text
.arm
.fpu neon

#define MAKE_FN_NAME(x,y) neonExecute_## x ## y
#define FUNCTION_NAME(x,y) MAKE_FN_NAME(x,y)

#define UMAKE_FN_NAME(x,y) _neonExecute_## x ## y
#define UFUNCTION_NAME(x,y) UMAKE_FN_NAME(x,y)

.global UFUNCTION_NAME(INSTR,WORD)
.global FUNCTION_NAME(INSTR,WORD)

.type   UFUNCTION_NAME(INSTR,WORD),%function
.type   FUNCTION_NAME(INSTR,WORD),%function

.extern trigger_set
.extern trigger_clr

.align 8

@ FUNCTION_NAME(INSTR,WORD)(uint8_t** memBank);
FUNCTION_NAME(INSTR,WORD):
UFUNCTION_NAME(INSTR,WORD):
  stmdb   sp!,{r4-r12,lr}

  /* Get some variables in place and return var */
  mov r4, r0
  mov r5, r0
  mov r6, r1
  mov r7, #0

  /* load the memory banks */
  vld1.WORD {d0}, [r4]!; /* load memBank[0] */
  vld1.WORD {d1}, [r4]!; /* load memBank[0] */
  vld1.WORD {d2}, [r4]!; /* load memBank[0] */
  vld1.WORD {d3}, [r4]!; /* load memBank[1] */
  vld1.WORD {d4}, [r4]!; /* load memBank[2] */
  vld1.WORD {d5}, [r4]!; /* load memBank[3] */
  vld1.WORD {d6}, [r4]!; /* load memBank[4] */
  vld1.WORD {d7}, [r4]!; /* load memBank[5] */
  vld1.WORD {d8}, [r4]!; /* load memBank[6] */
  vld1.WORD {d9}, [r4]!; /* load memBank[7] */
  vld1.WORD {d10}, [r4]!; /* load memBank[8] */
  vld1.WORD {d11}, [r4]!; /* load memBank[9] */
  vld1.WORD {d12}, [r4]!; /* load memBank[10] */
  vld1.WORD {d13}, [r4]!; /* load memBank[11] */

  bl trigger_set

  .rept 1000
  mov r1, r1
  .endr

  .rept 1
  veor.u32   d2, d5, d6
  INSTR.WORD d2, d3, d4
  .endr

  bl trigger_clr

  .rept 3000
  mov r1, r1
  .endr

  vst1.WORD {d0}, [r5]!; /* load memBank[0] */
  vst1.WORD {d1}, [r5]!; /* load memBank[0] */
  vst1.WORD {d2}, [r5]!; /* load memBank[0] */
  vst1.WORD {d3}, [r5]!; /* load memBank[1] */
  vst1.WORD {d4}, [r5]!; /* load memBank[2] */
  vst1.WORD {d5}, [r5]!; /* load memBank[3] */
  vst1.WORD {d6}, [r5]!; /* load memBank[4] */
  vst1.WORD {d7}, [r5]!; /* load memBank[5] */
  vst1.WORD {d8}, [r5]!; /* load memBank[6] */
  vst1.WORD {d9}, [r5]!; /* load memBank[7] */
  vst1.WORD {d10}, [r5]!; /* load memBank[8] */
  vst1.WORD {d11}, [r5]!; /* load memBank[9] */
  vst1.WORD {d12}, [r5]!; /* load memBank[10] */
  vst1.WORD {d13}, [r5]!; /* load memBank[11] */

  mov r0, r7

  ldmia   sp!,{r4-r12,pc}

.size UFUNCTION_NAME(INSTR,WORD),.-UFUNCTION_NAME(INSTR,WORD)
.size FUNCTION_NAME(INSTR,WORD),.-FUNCTION_NAME(INSTR,WORD)
