#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

enum token_type {
	TK_REF = 0,
	TK_IMM = 1,
	TK_REG = 2,
	TK_INSTR = 3,
	TK_LABEL = 4,
	TK_ADDR = 5,
};

struct token {
	enum token_type type;
	char str[5];
	/* the 0-th char is always the token_type indicator, not stored in str */
	/* '@' = ref */
	/* 'x' = dword immediate */
	/* 'R' = reg */
	/* ' ' or '\t' = instr */
	/* ':' = label */
	/* ';' = ignored */
	/* chars 1-5 are parsed */
	/* the 6-th char is ignored */

	uint64_t lut_idx;
	uint64_t offset;
};

struct label {
	char str[5];
	uint64_t offset;
};

enum operand_type {
	OPRD_NONE = 0,
	OPRD_IMM = 1,
	OPRD_REG = 2,
};

struct operand {
	enum operand_type type;
	uint8_t bit_offset;
	uint8_t rex_bit;
	uint8_t imm_len;
};

struct instruction {
	char name[5];
	int8_t rex_byte;
	uint8_t len;
	struct operand op0, op1, op2, op3, op4;
	uint8_t encoding[15];
};

struct reg {
	char name[5];
	uint8_t encoding;
	uint8_t rex;
};

struct reg registers[] = {
#define DECL_REG(NAME,REX,ENC) {.name={NAME[0],NAME[1],NAME[2],NAME[3],NAME[4]},.rex=REX,.encoding=ENC},
	DECL_REG("rax__", 0, 0x0)
	DECL_REG("rcx__", 0, 0x1)
	DECL_REG("rdx__", 0, 0x2)
	DECL_REG("rbx__", 0, 0x3)
	DECL_REG("rsp__", 0, 0x4)
	DECL_REG("rbp__", 0, 0x5)
	DECL_REG("rsi__", 0, 0x6)
	DECL_REG("rdi__", 0, 0x7)
	DECL_REG("r8___", 1, 0x0)
	DECL_REG("r9___", 1, 0x1)
	DECL_REG("r10__", 1, 0x2)
	DECL_REG("r11__", 1, 0x3)
	DECL_REG("r12__", 1, 0x4)
	DECL_REG("r13__", 1, 0x5)
	DECL_REG("r14__", 1, 0x6)
	DECL_REG("r15__", 1, 0x7)
#undef DECL_REG
};

struct instruction instructions[] = {
#define NO_OPERAND {.type=OPRD_NONE}
#define REXR 0x4
#define REXB 0x1
#define REXW 0x48
#define REX0 0x40
#define REG_OPERAND(OFF,REX_BIT) {.type=OPRD_REG,.bit_offset=OFF,.rex_bit=REX_BIT}
#define IMM_OPERAND(OFF,LEN) {.type=OPRD_IMM,.bit_offset=OFF,.imm_len=LEN}
#define IMM_OPERAND1(OFF) IMM_OPERAND(OFF,1)
#define IMM_OPERAND2(OFF) IMM_OPERAND(OFF,2)
#define IMM_OPERAND4(OFF) IMM_OPERAND(OFF,4)
#define DECL_INSTR(NAME,REX_BYTE,LEN,OP0,OP1,OP2,OP3,OP4,...) {.name={NAME[0],NAME[1],NAME[2],NAME[3],NAME[4]},.rex_byte=REX_BYTE,.len=LEN,.op0=OP0,.op1=OP1,.op2=OP2,.op3=OP3,.op4=OP4,.encoding=__VA_ARGS__},
	DECL_INSTR("MOVrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x89,0xC0})
	DECL_INSTR("MOVrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC7,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("MOVrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0xC7,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("MOVrq", 0,  10, REG_OPERAND(8,REXB),  IMM_OPERAND2(64),      IMM_OPERAND2(48),     IMM_OPERAND2(32),  IMM_OPERAND2(16), {REXW,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00})

	DECL_INSTR("INC__", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xFF,0xC0})
	DECL_INSTR("DEC__", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xFF,0xC8})

	DECL_INSTR("ADDrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x01,0xC0})
	DECL_INSTR("ADDrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("ADDrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("ORrr_", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x09,0xC0})
	DECL_INSTR("ORrw_", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xC8,0x00,0x00,0x00,0x00})
	DECL_INSTR("ORrd_", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xC8,0x00,0x00,0x00,0x00})
	DECL_INSTR("ADCrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x11,0xC0})
	DECL_INSTR("ADCrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xD0,0x00,0x00,0x00,0x00})
	DECL_INSTR("ADCrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xD0,0x00,0x00,0x00,0x00})
	DECL_INSTR("SBBrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x19,0xC0})
	DECL_INSTR("SBBrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xD8,0x00,0x00,0x00,0x00})
	DECL_INSTR("SBBrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xD8,0x00,0x00,0x00,0x00})
	DECL_INSTR("ANDrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x21,0xC0})
	DECL_INSTR("ANDrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xE0,0x00,0x00,0x00,0x00})
	DECL_INSTR("ANDrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xE0,0x00,0x00,0x00,0x00})
	DECL_INSTR("SUBrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x29,0xC0})
	DECL_INSTR("SUBrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xE8,0x00,0x00,0x00,0x00})
	DECL_INSTR("SUBrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xE8,0x00,0x00,0x00,0x00})
	DECL_INSTR("XORrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x31,0xC0})
	DECL_INSTR("XORrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xF0,0x00,0x00,0x00,0x00})
	DECL_INSTR("XORrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xF0,0x00,0x00,0x00,0x00})
	DECL_INSTR("CMPrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x39,0xC0})
	DECL_INSTR("CMPrw", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND4(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xF8,0x00,0x00,0x00,0x00})
	DECL_INSTR("CMPrd", 0,  7,  REG_OPERAND(16,REXB), IMM_OPERAND2(40),      IMM_OPERAND2(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x81,0xF8,0x00,0x00,0x00,0x00})

	DECL_INSTR("MUL__", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xF7,0xE0})
	DECL_INSTR("IMUL_", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xF7,0xE8})
	DECL_INSTR("DIV__", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xF7,0xF0})
	DECL_INSTR("IDIV_", 0,  3,  REG_OPERAND(16,REXB), NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xF7,0xF8})

	DECL_INSTR("IMLrr", 0,  4,  REG_OPERAND(24,REXB), REG_OPERAND(27,REXB),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x0F,0xAF,0xC0})

	DECL_INSTR("ROLrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xC0,0x00})
	DECL_INSTR("RORrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xC8,0x00})
	DECL_INSTR("RCLrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xD0,0x00})
	DECL_INSTR("RCRrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xD8,0x00})
	DECL_INSTR("SHLrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xE0,0x00})
	DECL_INSTR("SHRrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xE8,0x00})
	DECL_INSTR("SARrb", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0xC1,0xF8,0x00})

	DECL_INSTR("TSTrr", 0,  3,  REG_OPERAND(16,REXB), REG_OPERAND(19,REXR),  NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x85,0xC0})

	DECL_INSTR("JO___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x80,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNO__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x81,0x00,0x00,0x00,0x00})
	DECL_INSTR("JB___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x82,0x00,0x00,0x00,0x00})
	DECL_INSTR("JC___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x82,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNAE_", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x82,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNB__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x83,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNC__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x83,0x00,0x00,0x00,0x00})
	DECL_INSTR("JAE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x83,0x00,0x00,0x00,0x00})
	DECL_INSTR("JZ___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x84,0x00,0x00,0x00,0x00})
	DECL_INSTR("JE___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x84,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNZ__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x85,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x85,0x00,0x00,0x00,0x00})
	DECL_INSTR("JBE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x86,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNA__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x86,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNBE_", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x87,0x00,0x00,0x00,0x00})
	DECL_INSTR("JA___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x87,0x00,0x00,0x00,0x00})
	DECL_INSTR("JS___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x88,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNS__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x89,0x00,0x00,0x00,0x00})
	DECL_INSTR("JP___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8A,0x00,0x00,0x00,0x00})
	DECL_INSTR("JPE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8A,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNP__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8B,0x00,0x00,0x00,0x00})
	DECL_INSTR("JPO__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8B,0x00,0x00,0x00,0x00})
	DECL_INSTR("JL___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8C,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNGE_", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8C,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNL__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8D,0x00,0x00,0x00,0x00})
	DECL_INSTR("JGE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8D,0x00,0x00,0x00,0x00})
	DECL_INSTR("JLE__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8E,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNG__", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8E,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNLE_", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8F,0x00,0x00,0x00,0x00})
	DECL_INSTR("JG___", -1, 6,  IMM_OPERAND4(16),     NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x8F,0x00,0x00,0x00,0x00})
	DECL_INSTR("SYSCL", -1, 2,  NO_OPERAND,           NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x0F,0x05})

	DECL_INSTR("JMP__", -1, 5,  IMM_OPERAND4(8),      NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0xE9,0x00,0x00,0x00,0x00})
	DECL_INSTR("CALL_", -1, 5,  IMM_OPERAND4(8),      NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0xE8,0x00,0x00,0x00,0x00})
	DECL_INSTR("RET__", -1, 1,  NO_OPERAND,           NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0xC3})

	DECL_INSTR("PUSH_", 0,  2,  REG_OPERAND(8,REXB),  NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x50})
	DECL_INSTR("POP__", 0,  2,  REG_OPERAND(8,REXB),  NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x58})

	DECL_INSTR("LSVq_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x8B,0x44,0x24,0x00})
	DECL_INSTR("LSVd_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REX0,0x8B,0x44,0x24,0x00})
	DECL_INSTR("LSVw_", 1,  6,  REG_OPERAND(27,REXR), IMM_OPERAND1(40),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x66,REX0,0x8B,0x44,0x24,0x00})
	DECL_INSTR("LSVb_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REX0,0x8A,0x44,0x24,0x00})

	DECL_INSTR("SSVq_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REXW,0x89,0x44,0x24,0x00})
	DECL_INSTR("SSVd_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REX0,0x89,0x44,0x24,0x00})
	DECL_INSTR("SSVw_", 1,  6,  REG_OPERAND(27,REXR), IMM_OPERAND1(40),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x66,REX0,0x89,0x44,0x24,0x00})
	DECL_INSTR("SSVb_", 0,  5,  REG_OPERAND(19,REXR), IMM_OPERAND1(32),      NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {REX0,0x88,0x44,0x24,0x00})

	DECL_INSTR("LRPq_", 0,  4,  REG_OPERAND(19,REXR), REG_OPERAND(16,REXB),  IMM_OPERAND1(24),     NO_OPERAND,        NO_OPERAND,       {REXW,0x8B,0x40,0x00})
	DECL_INSTR("LRPd_", 0,  4,  REG_OPERAND(19,REXR), REG_OPERAND(16,REXB),  IMM_OPERAND1(24),     NO_OPERAND,        NO_OPERAND,       {REX0,0x8B,0x40,0x00})
	DECL_INSTR("LRPw_", 1,  5,  REG_OPERAND(27,REXR), REG_OPERAND(24,REXB),  IMM_OPERAND1(32),     NO_OPERAND,        NO_OPERAND,       {0x66,REX0,0x8B,0x40,0x00})
	DECL_INSTR("LRPb_", 0,  4,  REG_OPERAND(19,REXR), REG_OPERAND(16,REXB),  IMM_OPERAND1(24),     NO_OPERAND,        NO_OPERAND,       {REX0,0x8A,0x40,0x00})

	DECL_INSTR("SRPq_", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      REG_OPERAND(19,REXR), NO_OPERAND,        NO_OPERAND,       {REXW,0x89,0x40,0x00})
	DECL_INSTR("SRPd_", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      REG_OPERAND(19,REXR), NO_OPERAND,        NO_OPERAND,       {REX0,0x89,0x40,0x00})
	DECL_INSTR("SRPw_", 1,  5,  REG_OPERAND(24,REXB), IMM_OPERAND1(32),      REG_OPERAND(27,REXR), NO_OPERAND,        NO_OPERAND,       {0x66,REX0,0x89,0x40,0x00})
	DECL_INSTR("SRPb_", 0,  4,  REG_OPERAND(16,REXB), IMM_OPERAND1(24),      REG_OPERAND(19,REXR), NO_OPERAND,        NO_OPERAND,       {REX0,0x88,0x40,0x00})

	DECL_INSTR("DB___", -1, 1,  IMM_OPERAND1(0),      NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x00})
	DECL_INSTR("DW___", -1, 2,  IMM_OPERAND2(0),      NO_OPERAND,            NO_OPERAND,           NO_OPERAND,        NO_OPERAND,       {0x00,0x00})
#undef NO_OPERAND
#undef REXR
#undef REXB
#undef REXW
#undef REX0
#undef REG_OPERAND
#undef IMM_OPERAND
#undef IMM_OPERAND1
#undef IMM_OPERAND2
#undef IMM_OPERAND4
#undef DECL_INSTR
};

struct token *tokens;
uint64_t num_tokens;
struct label *labels;
uint64_t num_labels;

void
mem_cpy(void *dst, void *src, uint64_t size)
{
	while(size-- > 0) {
		((uint8_t *)dst)[size] = ((uint8_t *)src)[size];
	}
}

uint64_t
find_reg(struct token t)
{
	uint64_t i;
	struct reg r;

	for(i = 0; i < sizeof(registers)/sizeof(*registers); ++i) {
		r = registers[i];
		if(r.name[0] == t.str[0] &&
		   r.name[1] == t.str[1] &&
		   r.name[2] == t.str[2] &&
		   r.name[3] == t.str[3] &&
		   r.name[4] == t.str[4]) {
			return i;
		}
	}

	write(2, "no such register\n", 17);
	exit(1);
}

uint64_t
find_instr(struct token t)
{
	uint64_t i;
	struct instruction instr;

	for(i = 0; i < sizeof(instructions)/sizeof(*instructions); ++i) {
		instr = instructions[i];
		if(instr.name[0] == t.str[0] &&
		   instr.name[1] == t.str[1] &&
		   instr.name[2] == t.str[2] &&
		   instr.name[3] == t.str[3] &&
		   instr.name[4] == t.str[4]) {
			return i;
		}
	}

	write(2, "no such instruction\n", 20);
	#ifdef DEBUG
	fprintf(stderr, "str: %s\n", t.str);
	#endif
	exit(1);
}

void
tokenize()
{
	char c;
	struct token tk;
	ssize_t ret;
	struct label l;
	uint64_t prev_offset;

	prev_offset = 0;

	while(1) {
		ret = read(STDIN_FILENO, &c, 1);
		if(ret == 0) break;
		ret = read(STDIN_FILENO, &tk.str, 5);
		if(ret < 5) break;

		#ifdef DEBUG
		fprintf(stderr, "read string: %.5s\n", tk.str);
		#endif

		tk.offset = prev_offset;
		
		if(c == '@') {
			tk.type = TK_REF;
			goto tk_type_found;
		}
		if(c == 'x') {
			tk.type = TK_IMM;
			goto tk_type_found;
		}
		if(c == 'R') {
			tk.type = TK_REG;
			tk.lut_idx = find_reg(tk);

			goto tk_type_found;
		}
		if(c == ' ' || c == '\t') {
			tk.type = TK_INSTR;
			tk.lut_idx = find_instr(tk);
			prev_offset += instructions[tk.lut_idx].len;

			goto tk_type_found;
		}
		if(c == ':') {
			tk.type = TK_LABEL;
			mem_cpy(l.str, tk.str, 5);
			l.offset = prev_offset;
			labels[num_labels] = l;
			++num_labels;
			read(STDIN_FILENO, &c, 1);
			continue;
		}
		if(c == '&') {
			tk.type = TK_ADDR;
			goto tk_type_found;
		}
		if(c == ';') {
			read(STDIN_FILENO, &c, 1);
			continue;
		}

		write(2, "unknown tk type\n", 16);
		#ifdef DEBUG
		fprintf(stderr, "got: %c\n", c);
		#endif
		exit(1);

	tk_type_found:
		ret = read(STDIN_FILENO, &c, 1);
		if(ret == 0) break;

		tokens[num_tokens] = tk;
		++num_tokens;
	}
}

uint64_t
parse_hex_dig(char c)
{
	if(c <= '9') return c - '0';
	if(c <= 'F') return c - 'A' + 10;
	return c - 'a' + 10;
}

uint64_t
find_label_offset(struct token t)
{
	uint64_t i;
	struct label l;

	for(i = 0; i < num_labels; ++i) {
		l = labels[i];
		if(l.str[0] == t.str[0] &&
		   l.str[1] == t.str[1] &&
		   l.str[2] == t.str[2] &&
		   l.str[3] == t.str[3] &&
		   l.str[4] == t.str[4]) {
			return l.offset;
		}
	}

	write(2, "no such label\n", 14);
	#ifdef DEBUG
	fprintf(stderr, "str: %.5s\n", t.str);
	#endif
	exit(1);
}

uint64_t
parse_operand(uint64_t *i, uint8_t *enc, struct operand op, uint64_t offset_past_instr)
{
	struct token tk;
	struct reg r;
	uint64_t byte_off;
	uint64_t bit_off;
	uint64_t operand_encoding;

	byte_off = op.bit_offset / 8;
	bit_off = op.bit_offset % 8;
	enc += byte_off;

	if(op.type == OPRD_REG) {
		tk = tokens[*i];
		++ *i;
		if(tk.type != TK_REG) {
			write(2, "expected a register\n", 20);

			#ifdef DEBUG
			fprintf(stderr, "got: %i\n", tk.type);
			#endif

			exit(1);
		}

		r = registers[tk.lut_idx];
		operand_encoding = r.encoding << bit_off;
		*enc |= operand_encoding;

		if(r.rex) {
			return op.rex_bit;
		}
		return 0;
	}
	if(op.type == OPRD_IMM) {
		tk = tokens[*i];

		if(tk.type == TK_IMM) {
			operand_encoding = parse_hex_dig(tk.str[0]);
			operand_encoding <<= 4;
			operand_encoding |= parse_hex_dig(tk.str[1]);
			operand_encoding <<= 4;
			operand_encoding |= parse_hex_dig(tk.str[2]);
			operand_encoding <<= 4;
			operand_encoding |= parse_hex_dig(tk.str[3]);

			mem_cpy(enc, &operand_encoding, op.imm_len);

			++ *i;
			return 0;
		}
		if(tk.type == TK_REF) {
			operand_encoding = find_label_offset(tk);
			operand_encoding -= offset_past_instr;

			mem_cpy(enc, &operand_encoding, op.imm_len);

			++ *i;
			return 0;
		}
		if(tk.type == TK_ADDR) {
			operand_encoding = find_label_offset(tk);
			operand_encoding += 0x400078;

			mem_cpy(enc, &operand_encoding, op.imm_len);

			++ *i;
			return 0;
		}

		write(2, "expected an immediate or ref\n", 29);
		exit(1);
	}

	return 0;
}

void
emit()
{
	struct token tk;
	struct instruction instr;
	uint64_t i;
	uint8_t enc[19];
	uint64_t ret;
	uint64_t len;
	uint8_t rex;

	for(i = 0; i < num_tokens; ) {
		tk = tokens[i];
		++i;

		if(tk.type != TK_INSTR) {
			write(2, "wrong token type\n", 17);
			exit(1);
		}

		instr = instructions[tk.lut_idx];
		if(instr.rex_byte != -1) {
			rex = instr.encoding[instr.rex_byte];
		}

		#ifdef DEBUG
		fprintf(stderr, "idx: %lu; op0 type: %c; op1 type: %c\n", tk.lut_idx, "NIR"[instr.op0.type], "NIR"[instr.op1.type]);
		#endif

		len = instr.len;
		mem_cpy(enc, instr.encoding, len);
		rex |= parse_operand(&i, enc, instr.op0, tk.offset + len);
		rex |= parse_operand(&i, enc, instr.op1, tk.offset + len);
		rex |= parse_operand(&i, enc, instr.op2, tk.offset + len);
		rex |= parse_operand(&i, enc, instr.op3, tk.offset + len);
		rex |= parse_operand(&i, enc, instr.op4, tk.offset + len);

		if(instr.rex_byte != -1) {
			enc[instr.rex_byte] = rex;
		}

		ret = write(1, enc, len);
		if(ret < len) {
			write(2, "could not write\n", 16);
			exit(1);
		}

		#ifdef DEBUG
		fprintf(stderr, "i = %lu; enc = 0x%08lx\n", i, *(uint64_t *)&enc);
		#endif
	}
}

int main() {
	tokens = (struct token *)mmap(NULL, 0x400000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(tokens == MAP_FAILED) { write(2, "tokens\n", 6); exit(1); }
	num_tokens = 0;
	labels = (struct label *)mmap(NULL, 0x200000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(labels == MAP_FAILED) { write(2, "labels\n", 6); exit(1); }
	num_labels = 0;

	tokenize();

	#ifdef DEBUG
	for(uint64_t i = 0; i < num_tokens; ++i) {
		fprintf(stderr, "type: %c; str: %.5s; off: %i\n", "@dR :"[tokens[i].type], tokens[i].str, (int)tokens[i].offset);
	}
	fprintf(stderr, "---\n");
	for(uint64_t i = 0; i < num_labels; ++i) {
		fprintf(stderr, "str: %.5s, offset: %i\n", labels[i].str, (int)labels[i].offset);
	}
	#endif

	emit();
}

