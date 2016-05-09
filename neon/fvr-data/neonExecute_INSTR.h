#ifndef __neonExecute_INSTR_H__
#define __neonExecute_INSTR_H__

// All dynamically generated functions

void neonExecute_vmulu32(uint8_t** memBank);
void neonExecute_vsubu32(uint8_t** memBank);
void neonExecute_vaddu32(uint8_t** memBank);
void neonExecute_veoru32(uint8_t** memBank);
void neonExecute_vcequ32(uint8_t** memBank);

#endif /* __neonExecute_INSTR_H__ */