#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
using namespace std;

ifstream file;

int IP = 0;
int bitsRead = 0;

// Helper Functions
string registerLookup(int reg, int W);
string EAClookup(int MOD, int R_M, int W, int segOverride = 0);
string opExtendLookup(int opExtend);
string rotateExtendLookup(int op);
string testExtendLookup(int op);
string intToHexStr(int i, int widthInBits);
int readNumBits(int num);
int get8bit2sComp(int rawBinary);
string getRegFromOp(int opcode, int W);
void printInstruction(int byte, int segOverride = 0);

// Operand Encoding Functions
string regMemOperands(int W, int D, int segOverride);
string regMemOnly(int W, int segOverride);
string immedData(int W);
string immedToRegister(int opcode, int W);
string shortLabel();
string multiImmedOperands(int opcode);
string multiRotateOperands(int opcode);
string multiTestOperands(int opcode);
string multiIncOperands(int opcode);

enum opModes {
	NO_OP_ENC,
	REG_MEM_8_0,	
	REG_MEM_16_0,
	REG_MEM_8_1,
	REG_MEM_16_1,
	REG_MEM_8_ONLY,	
	REG_MEM_16_ONLY,
	AL_IMMED8,
	AX_IMMED8,
	AX_IMMED16,
	IMMED8_TO_REG,
	IMMED16_TO_REG,
	SHORT_LABEL,
	MULTI_IMMED8,
	SEGREG_0,	
	SEGREG_1,	
	EXCHANGE,	
	XCHG,
	NEAR_LABEL,
	FAR_LABEL,
	FAR_PROC,
	NEAR_PROC,
	ADDR_8_0,
	ADDR_16_0,
	ADDR_8_1,
	ADDR_16_1,
	DEST_SRC_STR_8,
	DEST_SRC_STR_16,
	DEST_STR_8,
	DEST_STR_16,
	SRC_STR_8,
	SRC_STR_16,
	IMMED8_TO_MEM8,
	IMMED16_TO_MEM16,
	SOURCE_TABLE,
	OPCODE_SOURCE,
	MULTI_INC,
	AA_SKIP,
	MULTI_TEST,
	MULTI_ROR,
	IMMED8,
	IMMED16,
	SS,
	DS,
	CS,
	ES
};

struct Instruction {
	string data;
	opModes operMode;
};

// all 8086 instructions
const Instruction instrs[256] {
	{"ADD\t", REG_MEM_8_0},		
	{"ADD\t", REG_MEM_16_0},
	{"ADD\t", REG_MEM_8_1},
	{"ADD\t", REG_MEM_16_1},
	{"ADD\t", AL_IMMED8},
	{"ADD\t", AX_IMMED16},
	{"PUSH\tES", NO_OP_ENC},
	{"POP\tES", NO_OP_ENC},
	{"OR\t", REG_MEM_8_0},
	{"OR\t", REG_MEM_16_0},
	{"OR\t", REG_MEM_8_1},
	{"OR\t", REG_MEM_16_1},
	{"OR\t", AL_IMMED8},
	{"OR\t", AX_IMMED16},
	{"PUSH\tCS", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"ADC\t", REG_MEM_8_0},
	{"ADC\t", REG_MEM_16_0},
	{"ADC\t", REG_MEM_8_1},
	{"ADC\t", REG_MEM_16_1},
	{"ADC\t", AL_IMMED8},
	{"ADC\t", AX_IMMED16},
	{"PUSH\tSS", NO_OP_ENC},
	{"POP\tSS", NO_OP_ENC},
	{"SBB\t", REG_MEM_8_0},
	{"SBB\t", REG_MEM_16_0},
	{"SBB\t", REG_MEM_8_1},
	{"SBB\t", REG_MEM_16_1},
	{"SBB\t", AL_IMMED8},
	{"SBB\t", AX_IMMED16},
	{"PUSH\tDS", NO_OP_ENC},
	{"POP\tDS", NO_OP_ENC},
	{"AND\t", REG_MEM_8_0},
	{"AND\t", REG_MEM_16_0},
	{"AND\t", REG_MEM_8_1},
	{"AND\t", REG_MEM_16_1},
	{"AND\t", AL_IMMED8},
	{"AND\t", AX_IMMED16},
	{"ES:", ES},
	{"DAA", NO_OP_ENC},
	{"SUB\t", REG_MEM_8_0},
	{"SUB\t", REG_MEM_16_0},
	{"SUB\t", REG_MEM_8_1},
	{"SUB\t", REG_MEM_16_1},
	{"SUB\t", AL_IMMED8},
	{"SUB\t", AX_IMMED16},
	{"CS:", CS},
	{"DAS", NO_OP_ENC},
	{"XOR\t", REG_MEM_8_0},
	{"XOR\t", REG_MEM_16_0},
	{"XOR\t", REG_MEM_8_1},
	{"XOR\t", REG_MEM_16_1},
	{"XOR\t", AL_IMMED8},
	{"XOR\t", AX_IMMED16},
	{"SS:", SS},
	{"AAA", NO_OP_ENC},
	{"CMP\t", REG_MEM_8_0},
	{"CMP\t", REG_MEM_16_0},
	{"CMP\t", REG_MEM_8_1},
	{"CMP\t", REG_MEM_16_1},
	{"CMP\t", AL_IMMED8},
	{"CMP\t", AX_IMMED16},
	{"DS:", DS},
	{"AAS", NO_OP_ENC},
	{"INC\tAX", NO_OP_ENC},
	{"INC\tCX", NO_OP_ENC},
	{"INC\tDX", NO_OP_ENC},
	{"INC\tBX", NO_OP_ENC},
	{"INC\tSP", NO_OP_ENC},
	{"INC\tBP", NO_OP_ENC},
	{"INC\tSI", NO_OP_ENC},
	{"INC\tDI", NO_OP_ENC},
	{"DEC\tAX", NO_OP_ENC},
	{"DEC\tCX", NO_OP_ENC},
	{"DEC\tDX", NO_OP_ENC},
	{"DEC\tBX", NO_OP_ENC},
	{"DEC\tSP", NO_OP_ENC},
	{"DEC\tBP", NO_OP_ENC},
	{"DEC\tSI", NO_OP_ENC},
	{"DEC\tDI", NO_OP_ENC},
	{"PUSH\tAX", NO_OP_ENC},
	{"PUSH\tCX", NO_OP_ENC},
	{"PUSH\tDX", NO_OP_ENC},
	{"PUSH\tBX", NO_OP_ENC},
	{"PUSH\tSP", NO_OP_ENC},
	{"PUSH\tBP", NO_OP_ENC},
	{"PUSH\tSI", NO_OP_ENC},
	{"PUSH\tDI", NO_OP_ENC},
	{"POP\tAX", NO_OP_ENC},
	{"POP\tCX", NO_OP_ENC},
	{"POP\tDX", NO_OP_ENC},
	{"POP\tBX", NO_OP_ENC},
	{"POP\tSP", NO_OP_ENC},
	{"POP\tBP", NO_OP_ENC},
	{"POP\tSI", NO_OP_ENC},
	{"POP\tDI", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"JO\t", SHORT_LABEL},
	{"JNO\t", SHORT_LABEL},
	{"JB\t", SHORT_LABEL},
	{"JNB\t", SHORT_LABEL},
	{"JE\t", SHORT_LABEL},
	{"JNE\t", SHORT_LABEL},
	{"JBE\t", SHORT_LABEL},
	{"JNBE\t", SHORT_LABEL},
	{"JS\t", SHORT_LABEL},
	{"JNS\t", SHORT_LABEL},
	{"JP\t", SHORT_LABEL},
	{"JNP\t", SHORT_LABEL},
	{"JL\t", SHORT_LABEL},
	{"JNL\t", SHORT_LABEL},
	{"JLE\t", SHORT_LABEL},
	{"JNLE\t", SHORT_LABEL},
	{"MULTI\t", MULTI_IMMED8},	
	{"MULTI\t", MULTI_IMMED8},	
	{"MULTI\t", MULTI_IMMED8},	
	{"MULTI\t", MULTI_IMMED8},	
	{"TEST\t", REG_MEM_8_0},
	{"TEST\t", REG_MEM_16_0},
	{"XCHG\t", REG_MEM_8_0},
	{"XCHG\t", REG_MEM_16_0},
	{"MOV\t", REG_MEM_8_0},
	{"MOV\t", REG_MEM_16_0},	
	{"MOV\t", REG_MEM_8_1},
	{"MOV\t", REG_MEM_16_1},
	{"MOV\t", SEGREG_0},			// NEED TO IMPLEMENT
	{"LEA\t", REG_MEM_16_1},	
	{"MOV\t", SEGREG_1},			// NEED TO IMPLEMENT
	{"POP\t", REG_MEM_16_1},	
	{"NOP\t", EXCHANGE},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"XCHG\t", XCHG},
	{"CBW\t", NO_OP_ENC},
	{"CWD\t", NO_OP_ENC},
	{"CALL\t", FAR_PROC},			// NEED TO IMPLEMENT
	{"WAIT\t", NO_OP_ENC},
	{"PUSHF\t", NO_OP_ENC},
	{"POPF\t", NO_OP_ENC},
	{"SAHF\t", NO_OP_ENC},
	{"LAHF\t", NO_OP_ENC},
	{"MOV\t", ADDR_8_0},			// NEED TO IMPLEMENT
	{"MOV\t", ADDR_16_0},			// NEED TO IMPLEMENT
	{"MOV\t", ADDR_8_1},			// NEED TO IMPLEMENT
	{"MOV\t", ADDR_16_1},			// NEED TO IMPLEMENT
	{"MOVS\t", DEST_SRC_STR_8},		// NEED TO IMPLEMENT
	{"MOVS\t", DEST_SRC_STR_16},	// NEED TO IMPLEMENT
	{"CMPS\t", DEST_SRC_STR_8},		// NEED TO IMPLEMENT
	{"CMPS\t", DEST_SRC_STR_16},	// NEED TO IMPLEMENT
	{"TEST\t", AL_IMMED8},
	{"TEST\t", AX_IMMED16},
	{"STOS\t", DEST_STR_8},			// NEED TO IMPLEMENT
	{"STOS\t", DEST_STR_16},		// NEED TO IMPLEMENT
	{"LODS\t", SRC_STR_8},			// NEED TO IMPLEMENT
	{"LODS\t", SRC_STR_16},			// NEED TO IMPLEMENT
	{"SCAS\t", DEST_STR_8},			// NEED TO IMPLEMENT
	{"SCAS\t", DEST_STR_16},		// NEED TO IMPLEMENT
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},		
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED8_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},
	{"MOV\t", IMMED16_TO_REG},		
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"RET\t", IMMED16},
	{"RET\t", NO_OP_ENC},		
	{"LES\t", REG_MEM_16_1},	
	{"LDS\t", REG_MEM_16_1},	
	{"MOV\t", IMMED8_TO_MEM8},		// NEED TO IMPLEMENT
	{"MOV\t", IMMED16_TO_MEM16},	// NEED TO IMPLEMENT
	{"**NOT USED**", NO_OP_ENC},	
	{"**NOT USED**", NO_OP_ENC},	
	{"RET\t", IMMED16},
	{"RET\t", NO_OP_ENC},
	{"INT\t3", NO_OP_ENC},
	{"INT\t", IMMED8},
	{"INTO", NO_OP_ENC},
	{"IRET", NO_OP_ENC},
	{"MULTI", MULTI_ROR},
	{"MULTI", MULTI_ROR},
	{"MULTI", MULTI_ROR},
	{"MULTI", MULTI_ROR},
	{"AAM", AA_SKIP},				
	{"AAD", NO_OP_ENC},	
	{"**NOT USED**", NO_OP_ENC},
	{"XLAT", SOURCE_TABLE},			// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT
	{"LOOPNE\t", SHORT_LABEL},		
	{"LOOPE\t", SHORT_LABEL},	
	{"LOOP\t", SHORT_LABEL},
	{"JCXZ\t", SHORT_LABEL},	
	{"IN\t", AL_IMMED8},
	{"IN\t", AX_IMMED8},
	{"OUT\t", AL_IMMED8},
	{"OUT\t", AX_IMMED8},
	{"CALL\t", NEAR_PROC},			// NEED TO IMPLEMENT
	{"JMP\t", NEAR_LABEL},			// NEED TO IMPLEMENT
	{"JMP\t", FAR_LABEL},			// NEED TO IMPLEMENT
	{"JMP\t", SHORT_LABEL},
	{"IN\tAL,DX", NO_OP_ENC},
	{"IN\tAX,DX", NO_OP_ENC},
	{"OUT\tAL,DX", NO_OP_ENC},
	{"OUT\tAX,DX", NO_OP_ENC},
	{"LOCK", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"REPNE", NO_OP_ENC},
	{"REP", NO_OP_ENC},
	{"HLT", NO_OP_ENC},
	{"CMC", NO_OP_ENC},
	{"MULTI", MULTI_TEST},
	{"MULTI", MULTI_TEST},
	{"CLC", NO_OP_ENC},
	{"STC", NO_OP_ENC},
	{"CLI", NO_OP_ENC},
	{"STI", NO_OP_ENC},
	{"CLD", NO_OP_ENC},
	{"STD", NO_OP_ENC},
	{"MULTI\t", MULTI_INC},
	{"MULTI\t", MULTI_INC},
};

int main()
{
	file.open("ExampleArithmeticBinary.txt", ios::in | ios::out);

	if (!file) {
		std::cout << "Error: readFile: file \"" << "ExampleArithmeticBinary.txt" << "\" could not be opened" << endl;
		return -1;
	}
	
	// reads entire file
	int byte = 0;
	while ((byte = readNumBits(8)) >= 0) {
		printInstruction(byte);
	}

	cout << "\nPress Enter to close..." << endl;
	cin.get();
	return 0;
}

// takes in a byte and prints out the corresponding instruction according to its operand encoding
void printInstruction(int byte, int segOverride)
{
	Instruction instr = instrs[byte];

	switch (instr.operMode) {
	case SS:
	case DS:
	case CS:
	case ES:
		printInstruction(readNumBits(8), instr.operMode);
		break;
	case REG_MEM_8_0:
		cout << instr.data << regMemOperands(0, 0, segOverride) << endl;
		break;
	case REG_MEM_16_0:
		cout << instr.data << regMemOperands(1, 0, segOverride) << endl;
		break;
	case REG_MEM_8_1:
		cout << instr.data << regMemOperands(0, 1, segOverride) << endl;
		break;
	case REG_MEM_16_1:
		cout << instr.data << regMemOperands(1, 1, segOverride) << endl;
		break;
	case IMMED8_TO_REG:
		cout << instr.data << immedToRegister(byte, 0) << endl;
		break;
	case IMMED16_TO_REG:
		cout << instr.data << immedToRegister(byte, 1) << endl;
		break;
	case SHORT_LABEL:
		cout << instr.data << shortLabel() << endl;
		break;
	case MULTI_IMMED8:
		cout << multiImmedOperands(byte) << endl;
		break;
	case AL_IMMED8:
		cout << instr.data << "AL," << immedData(0) << endl;
		break;
	case AX_IMMED8:
		cout << instr.data << "AX," << immedData(0) << endl;
		break;
	case AX_IMMED16:
		cout << instr.data << "AX," << immedData(1) << endl;
		break;
	case XCHG:
		cout << instr.data << "AX," << getRegFromOp(byte, 1) << endl;
		break;
	case IMMED8:
		cout << instr.data << immedData(0) << endl;
		break;
	case IMMED16:
		cout << instr.data << immedData(1) << endl;
		break;
	case MULTI_ROR:
		cout << multiRotateOperands(byte) << endl;
		break;
	case MULTI_TEST:
		cout << multiTestOperands(byte) << endl;
		break;
	case AA_SKIP:
		readNumBits(8);		// skip next 8 bits
		break;
	case REG_MEM_8_ONLY:
		cout << instr.data << regMemOnly(0, segOverride) << endl;
		break;
	case REG_MEM_16_ONLY:
		cout << instr.data << regMemOnly(1, segOverride) << endl;
		break;
	case MULTI_INC:
		cout << multiIncOperands(byte) << endl;
		break;
	case NO_OP_ENC:
		cout << instr.data << endl;
		break;
	default:
		cout << "ERROR: printInstruction(): operand encoding not recognized" << endl;
		break;
	}
}

// returns the register that an integer corresponds to; can b 8- or 16-bit
string registerLookup(int reg, int W)
{
	if (W == 0) {
		switch (reg) {
			case 0b000:
				return "AL";
			case 0b001:
				return "CL";
			case 0b010:
				return "DL";
			case 0b011:
				return "BL";
			case 0b100:
				return "AH";
			case 0b101:
				return "CH";
			case 0b110:
				return "DH";
			case 0b111:
				return "BH";
		}
	}
	if (W == 1) {
		switch (reg) {
			case 0b000:
				return "AX";
			case 0b001:
				return "CX";
			case 0b010:
				return "DX";
			case 0b011:
				return "BX";
			case 0b100:
				return "SP";
			case 0b101:
				return "BP";
			case 0b110:
				return "SI";
			case 0b111:
				return "DI";
		}
	}

	return "[COULDN'T FIND REGISTER]";
}

// returns either a memory address (calculted with Effective Address Calculation) or a register, corresponding 
// to the MOD, R_M, and W bits
string EAClookup(int MOD, int R_M, int W, int segOverride)
{
	string output = "";

	switch (segOverride) {
		case SS:
			output += "SS:";
			break;
		case DS:
			output += "DS:";
			break;
		case CS:
			output += "CS:";
			break;
		case ES:
			output += "ES:";
			break;
	}

	switch (MOD) {
		case 0b00:		// MOD == 00; no displacement
			switch (R_M) {
				case 0b000:
					output += "[BX+SI]";
					break;
				case 0b001:
					output += "[BX+DI]";
					break;
				case 0b010:
					output += "[BP+SI]";
					break;
				case 0b011:
					output += "[BP+DI]";
					break;
				case 0b100:
					output += "[SI]";
					break;
				case 0b101:
					output += "[DI]";
					break;
				case 0b110: {
					int d16 = readNumBits(16);
					output += "[" + intToHexStr(d16, 16) + "]";
					break;
				}
				case 0b111:
					output += "[BX]";
					break;
			}
			break;	
		case 0b01: {	// MOD == 01; 8-bit displacement
			int d8 = readNumBits(8);
			int comp = get8bit2sComp(d8);

			switch (R_M) {
				case 0b000:
					output += "[BX+SI+" + intToHexStr(d8, 8) + "]";		
					break;
				case 0b001:
					output += "[BX+DI+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b010:
					output += "[BP+SI+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b011:
					output += "[BP+DI+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b100:
					output += "[SI+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b101:
					output += "[DI+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b110:
					output += "[BP+" + intToHexStr(d8, 8) + "]";
					break;
				case 0b111:
					output += "[BX+" + intToHexStr(d8, 8) + "]";
					break;
			}
			break;
		}
		case 0b10: {	// MOD == 10; 16-bit displacement
			int d16 = readNumBits(16);
			switch (R_M) {
				case 0b000:
					output += "[BX+SI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b001:
					output += "[BX+DI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b010:
					output += "[BP+SI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b011:
					output += "[BP+DI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b100:
					output += "[SI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b101:
					output += "[DI+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b110:
					output += "[BP+" + intToHexStr(d16, 16) + "]";
					break;
				case 0b111:
					output += "[BX+" + intToHexStr(d16, 16) + "]";
					break;
			}
			break;
		}
		case 0b11:		// MOD == 11; register mode (no displacement)
			output += registerLookup(R_M, W);
			break;
	}
	
	if (!output.empty())
		return output;

	return "[COULDN'T FIND EAC]";
}

// helper function for multiImmedOperands
string opExtendLookup(int opExtend)
{
	switch (opExtend) {
		case 0b000:
			return "ADD\t";
		case 0b001:
			return "OR\t";
		case 0b010:
			return "ADC\t";
		case 0b011:
			return "SBB\t";
		case 0b100:
			return "AND\t";
		case 0b101:
			return "SUB\t";
		case 0b110:
			return "XOR\t";
		case 0b111:
			return "CMP\t";
		default:
			return "OP_EXTEND NOT RECOGNIZED: opExtendLookup()";
	}
}

// encoding function
string regMemOnly(int W, int segOverride)
{
	string EACStr;

	int mod = readNumBits(2);
	int reg = readNumBits(3);
	int r_m = readNumBits(3);

	EACStr = EAClookup(mod, r_m, W, segOverride);

	return EACStr;
}

// encoding function
string regMemOperands(int W, int D, int segOverride)
{
	string regStr, EACStr;

	int mod = readNumBits(2);
	int reg = readNumBits(3);
	int r_m = readNumBits(3);

	EACStr = EAClookup(mod, r_m, W, segOverride);
	regStr = registerLookup(reg, W);

	if (D) {	// D == 1; reg first
		return regStr + "," + EACStr;
	}
	else {		// D == 0; EAC first
		return EACStr + "," + regStr;
	}
}

// encoding function
string immedData(int W)	
{	
	if (W)
		return intToHexStr(readNumBits(16), 16);
	return intToHexStr(readNumBits(8), 8);
}

// helper function; returns the register embedded within an opcode
string getRegFromOp(int opcode, int W)
{
	int reg = 0;
	int mask = 1;

	for (int i = 0; i < 3; ++i) {	// extract 3 rightmost bits into num
		reg += (opcode & mask);
		mask <<= 1;
	}

	return registerLookup(reg, W);
}

// encoding function
string immedToRegister(int opcode, int W)
{
	string reg = getRegFromOp(opcode, W);
	return reg + "," + immedData(W);
}

// returns the 8-bit 2's complement representation of a binary number
int get8bit2sComp(int rawBinary)
{
	int num = 0;
	int mask = 1;

	if (!(rawBinary >> 7)) {	// sign bit is positive
		num = rawBinary;
	}
	for (int i = 0; i < 7; ++i) {
		num += (rawBinary & mask);
		mask <<= 1;
	}
	num -= 128;

	return num;
}

// encoding function
string shortLabel()
{
	int num = 0;
	int data = readNumBits(8);

	num = get8bit2sComp(data);		// signed, but don't need to put +/-

	return intToHexStr(num + IP + 2, 16);		// add an extra two to account for the IP moving past this instruction as well
}

// encoding function
string multiIncOperands(int opcode)
{
	string outputStr;

	int mod = readNumBits(2);
	int opExtend = readNumBits(3);
	int r_m = readNumBits(3);

	switch (opcode) {
		case 0xFE:
			switch (opExtend) {
				case 0b000:
					outputStr += "INC\t" + EAClookup(mod, r_m, 0);
					break;
				case 0b001:
					outputStr += "DEC\t" + EAClookup(mod, r_m, 0);
					break;
			}
			break;
		case 0xFF:
			switch (opExtend) {
				case 0b000:
					outputStr += "INC\t" + EAClookup(mod, r_m, 1);
					break;
				case 0b001:
					outputStr += "DEC\t" + EAClookup(mod, r_m, 1);
					break;
				case 0b010:
				case 0b011:
					outputStr += "CALL\t" + EAClookup(mod, r_m, 1);
					break;
				case 0b100:
				case 0b101:
					outputStr += "JMP\t" + EAClookup(mod, r_m, 1);
					break;
				case 0b110:
					outputStr += "PUSH\t" + EAClookup(mod, r_m, 1);
					break;
			}
			break;
	}

	return outputStr;
}

// helper function for multiTestOperands
string testExtendLookup(int op)
{
	switch (op) {
		case 0b000:
			return "TEST\t";
		case 0b001:
			return "NOT USED\t";
		case 0b010:
			return "NOT\t";
		case 0b011:
			return "NEG\t";
		case 0b100:
			return "MUL\t";
		case 0b101:
			return "IMUL\t";
		case 0b110:
			return "DIV\t";
		case 0b111:
			return "IDIV\t";
		default:
			return "OP_EXTEND NOT RECOGNIZED: testExtendLookup()";
	}
}

// encoding function
string multiTestOperands(int opcode)
{
	string outputStr;

	int mod = readNumBits(2);
	int opExtend = readNumBits(3);
	int r_m = readNumBits(3);

	outputStr += testExtendLookup(opExtend);

	if (outputStr == "TEST\t") {
		switch (opcode) {
			case 0xF6:
				outputStr += EAClookup(mod, r_m, 0);
				outputStr += "," + immedData(0);
				return outputStr;
			case 0xF7:
				outputStr += EAClookup(mod, r_m, 1);
				outputStr += "," + immedData(1);
				return outputStr;
		}
	}

	switch (opcode) {
		case 0xF6:
			outputStr += EAClookup(mod, r_m, 0);
			break;
		case 0xF7:
			outputStr += EAClookup(mod, r_m, 1);
			break;
	}

	return outputStr;
}

// helper function for multiRotateOperands
string rotateExtendLookup(int op)
{
	switch (op) {
		case 0b000:
			return "ROL\t";
		case 0b001:
			return "ROR\t";
		case 0b010:
			return "RCL\t";
		case 0b011:
			return "RCR\t";
		case 0b100:
			return "SAL\t";
		case 0b101:
			return "SHR\t";
		case 0b110:
			return "NOT USED\t";
		case 0b111:
			return "SAR\t";
		default:
			return "OP_EXTEND NOT RECOGNIZED: rotateExtendLookup()";
	}
}

// encoding function
string multiRotateOperands(int opcode)
{
	string outputStr;

	int mod = readNumBits(2);
	int opExtend = readNumBits(3);
	int r_m = readNumBits(3);

	outputStr += rotateExtendLookup(opExtend);

	switch (opcode) {
		case 0xD0:
			outputStr += EAClookup(mod, r_m, 0) + ",1";
			break;
		case 0xD1:
			outputStr += EAClookup(mod, r_m, 1) + ",1";
			break;
		case 0xD2:
			outputStr += EAClookup(mod, r_m, 0) + ",CL";
			break;
		case 0xD3:
			outputStr += EAClookup(mod, r_m, 1) + ",CL";
			break;
	}

	return outputStr;
}

// encoding function
string multiImmedOperands(int opcode)
{
	// 80 instrs are REG8/MEM8,IMMED8
	// 81 instrs are REG16/MEM16,IMMED16
	// 82 instrs are REG8/MEM8,IMMED8
	// 83 instrs are REG16/MEM16,IMMED8
	// in all cases, going from IMMED into REG/MEM

	string outputStr;

	int mod = readNumBits(2);
	int opExtend = readNumBits(3);
	int r_m = readNumBits(3);

	outputStr += opExtendLookup(opExtend);

	switch (opcode) {
		case 0x80:
		case 0x82:
			outputStr += EAClookup(mod, r_m, 0);	// 8 bit register
			outputStr += "," + intToHexStr(readNumBits(8), 8);
			break;
		case 0x81:
			outputStr += EAClookup(mod, r_m, 1);	// 16 bit register
			outputStr += "," + intToHexStr(readNumBits(16), 16);
			break;
		case 0x83:
			outputStr += EAClookup(mod, r_m, 1);
			outputStr += "," + intToHexStr(readNumBits(8), 8);
			break;
	}

	return outputStr;
}

// returns the hexadecimal (string) representation of an integer
string intToHexStr(int num, int widthInBits)		// formatted with least significant byte first if 16-bit
{
	stringstream s;

	int leftByte = 0;
	int rightByte = 0;
	int mask = 1;
	int newNum = 0;

	if (widthInBits == 16) {

		leftByte = (num >> 8);		// extract 8 leftmost bits into leftByte

		for (int i = 0; i < 8; ++i) {	// extract 8 rightmost bits into rightByte
			rightByte += (num & mask);
			mask <<= 1;
		}

		newNum = rightByte & 0b11111111;
		newNum <<= 8;
		mask = 1;
		for (int i = 0; i < 8; ++i) {
			newNum += (leftByte & mask);
			mask <<= 1;
		}

		s << uppercase << setfill('0') << setw(widthInBits / 4) << std::hex << newNum;

		return s.str();
	}
	else {
		s << uppercase << setfill('0') << setw(widthInBits / 4) << std::hex << num;

		return s.str();
	}
}

// reads bits from the file
int readNumBits(int num)
{
	int bits = 0;
	char c = ' ';
	int counter = 0;

	while (file.good()) {
		file >> c;

		if (c == '1')
			bits += 1;

		if (counter < num - 1) {
			bits <<= 1;
			++counter;
		}

		else {
			bitsRead += num;
			if (bitsRead % 8 == 0 && bitsRead != 0) {
				++IP;
				bitsRead = 0;
			}

			return bits;
		}
	}

	cout << "file not good! readNumBits()" << endl;
	return -1;
}