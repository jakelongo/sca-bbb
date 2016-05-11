#ifndef __neonExecute_INSTR_H__
#define __neonExecute_INSTR_H__

// All dynamically generated functions
void neonExecute_vandu64(uint8_t** membank);
void neonExecute_veoru64(uint8_t** membank);
void neonExecute_vmuli8 (uint8_t** membank);
void neonExecute_vaddi8 (uint8_t** membank);
void neonExecute_vsubi8 (uint8_t** membank);
void neonExecute_vmuli16(uint8_t** membank);
void neonExecute_vaddi16(uint8_t** membank);
void neonExecute_vsubi16(uint8_t** membank);
void neonExecute_vmuli32(uint8_t** membank);
void neonExecute_vaddi32(uint8_t** membank);
void neonExecute_vsubi32(uint8_t** membank);
void neonExecute_vaddi64(uint8_t** membank);
void neonExecute_vsubi64(uint8_t** membank);


#endif /* __neonExecute_INSTR_H__ */