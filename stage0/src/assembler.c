#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

/* debug */
//#include <stdio.h>

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
	/* ' ' = instr */
	/* ':' = label */
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
	uint64_t bit_offset;
	uint64_t rex_bit;
};

struct instruction {
	char name[5];
	int64_t rex_byte;
	uint64_t len;
	struct operand op0, op1, op2, op3, op4;
	uint8_t encoding[15];
};

struct reg {
	char name[5];
	uint64_t encoding;
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
#define REX_R 0x4
#define REX_B 0x1
#define OREX 0x48
#define REG_OPERAND(OFF,REX_BIT) {.type=OPRD_REG,.bit_offset=OFF,.rex_bit=REX_BIT}
#define IMM_OPERAND(OFF) {.type=OPRD_IMM,.bit_offset=OFF}
#define DECL_INSTR(NAME,REX_BYTE,LEN,OP0,OP1,OP2,OP3,OP4,...) {.name={NAME[0],NAME[1],NAME[2],NAME[3],NAME[4]},.rex_byte=REX_BYTE,.len=LEN,.op0=OP0,.op1=OP1,.op2=OP2,.op3=OP3,.op4=OP4,.encoding=__VA_ARGS__},
	DECL_INSTR("MOVrr", 0,  3,  REG_OPERAND(16,REX_R), REG_OPERAND(19,REX_B), NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0x89,0xC0})
	DECL_INSTR("MOVrw", 0,  7,  REG_OPERAND(16,REX_B), IMM_OPERAND(24),       NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0xC7,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("MOVrd", 0,  7,  REG_OPERAND(16,REX_B), IMM_OPERAND(40),       IMM_OPERAND(24), NO_OPERAND,     NO_OPERAND,      {OREX,0xC7,0xC0,0x00,0x00,0x00,0x00})
	DECL_INSTR("MOVrq", 0,  10, REG_OPERAND(8,REX_B),  IMM_OPERAND(64),       IMM_OPERAND(48), IMM_OPERAND(32),IMM_OPERAND(16), {OREX,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00})
	DECL_INSTR("ADDrr", 0,  3,  REG_OPERAND(16,REX_R), REG_OPERAND(19,REX_B), NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0x01,0xC0})
	DECL_INSTR("SUBrr", 0,  3,  REG_OPERAND(16,REX_R), REG_OPERAND(19,REX_B), NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0x29,0xC0})
	DECL_INSTR("XORrr", 0,  3,  REG_OPERAND(16,REX_R), REG_OPERAND(19,REX_B), NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0x31,0xC0})
	DECL_INSTR("CMPrr", 0,  3,  REG_OPERAND(16,REX_R), REG_OPERAND(19,REX_B), NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {OREX,0x39,0xC0})
	DECL_INSTR("JMP__", -1, 5,  IMM_OPERAND(8),        NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0xE9,0x00,0x00,0x00,0x00})
	DECL_INSTR("JE___", -1, 6,  IMM_OPERAND(16),       NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0x0F,0x84,0x00,0x00,0x00,0x00})
	DECL_INSTR("JNE__", -1, 6,  IMM_OPERAND(16),       NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0x0F,0x85,0x00,0x00,0x00,0x00})
	DECL_INSTR("SYSCL", -1, 2,  NO_OPERAND,            NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0x0F,0x05})
	DECL_INSTR("RET__", -1, 1,  NO_OPERAND,            NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0xC3})

	DECL_INSTR("DB___", -1, 1,  IMM_OPERAND(0),        NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0x00})
	DECL_INSTR("DW___", -1, 2,  IMM_OPERAND(0),        NO_OPERAND,            NO_OPERAND,      NO_OPERAND,     NO_OPERAND,      {0x00,0x00})
#undef NO_OPERAND
#undef REX_R
#undef REX_B
#undef REG_OPERAND
#undef IMM_OPERAND
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

		/* debug */
		//printf("read string: %.5s\n", tk.str);

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
		if(c == ' ') {
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

		write(2, "unknown tk type\n", 16);
		/* debug */
		//printf("got: %c\n", c);
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

			/* debug */
			//printf("got: %i\n", tk.type);

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

			*(uint16_t *)enc = (uint16_t)operand_encoding;

			++ *i;
			return 0;
		}
		if(tk.type == TK_REF) {
			operand_encoding = find_label_offset(tk);
			operand_encoding -= offset_past_instr;

			*(uint32_t *)enc = (uint32_t)operand_encoding;

			++ *i;
			return 0;
		}
		if(tk.type == TK_ADDR) {
			operand_encoding = find_label_offset(tk);
			operand_encoding += 0x400078;

			*(uint64_t *)enc = (uint64_t)operand_encoding;

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

		rex = 0x48; /* REX | REX.W */
		instr = instructions[tk.lut_idx];

		/* debug */
		//printf("idx: %lu; op0 type: %c; op1 type: %c\n", tk.lut_idx, "NIR"[instr.op0.type], "NIR"[instr.op1.type]);

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

		/* debug */
		//printf("i = %lu; enc = 0x%08lx\n", i, *(uint64_t *)&enc);
	}
}

int main() {
	tokens = (struct token *)mmap(NULL, 0x4000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(tokens == MAP_FAILED) { write(2, "tokens\n", 6); exit(1); }
	num_tokens = 0;
	labels = (struct label *)mmap(NULL, 0x2000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(labels == MAP_FAILED) { write(2, "labels\n", 6); exit(1); }
	num_labels = 0;

	tokenize();

	/* debug */
	//for(uint64_t i = 0; i < num_tokens; ++i) {
	//	printf("type: %c; str: %.5s; off: %i\n", "@dR :"[tokens[i].type], tokens[i].str, (int)tokens[i].offset);
	//}
	//puts("---");
	//for(uint64_t i = 0; i < num_labels; ++i) {
	//	printf("str: %.5s, next_idx: %i\n", labels[i].str, (int)labels[i].next_token_idx);
	//}

	emit();
}

