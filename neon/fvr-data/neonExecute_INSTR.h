#ifndef __neonExecute_INSTR_H__
#define __neonExecute_INSTR_H__

// All dynamically generated functions

uint32_t neonExecute_vmulu32(uint32_t** memBank);
uint32_t neonExecute_vsubu32(uint32_t** memBank);
uint32_t neonExecute_vaddu32(uint32_t** memBank);
uint32_t neonExecute_veoru32(uint32_t** memBank);
uint32_t neonExecute_vcequ32(uint32_t** memBank);

#endif /* __neonExecute_INSTR_H__ */