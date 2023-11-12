#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <cmath>
#include <chrono>


using namespace std;

ifstream file;


int IP = 0;
int bitsRead = 0;

//enum {		// test for segment override (only ever applies to EAC calc)
//	NO_SEG_OV = 0,
//	SS = 1,
//	DS = 2,
//	CS = 3,
//	ES = 4
//};

// Function prototypes
string registerLookup(int reg, int W);
string EAClookup(int MOD, int R_M, int W, int segOverride = 0);
string opExtendLookup(int opExtend);
string rotateExtendLookup(int op);
string testExtendLookup(int op);
string incExtendLookup(int op);
//string EAClookup(int MOD, int R_M, int W);
//string EAClookup(int MOD, int R_M, int W = 0);
string decodeInstr(int byte, int override/* = NO_SEG_OV*/);
string intToHexStr(int i, int widthInBits);
int readNumBits(int num);
int get8bit2sComp(int rawBinary);
string getRegFromOp(int opcode, int W);

// operand encoding
string regMemOperands(int W, int D, int segOverride);

string regMemOnly(int W, int segOverride);

string immedData(int W);
string immedToRegister(int opcode, int W);
string shortLabel();
string multiImmedOperands(int opcode);
string multiRotateOperands(int opcode);
string multiTestOperands(int opcode);
string multiIncOperands(int opcode);

void printInstruction(int byte, int segOverride = 0);


enum opModes {
	NO_OP_ENC,

	REG_MEM_8_0,	// two operands
	REG_MEM_16_0,
	REG_MEM_8_1,
	REG_MEM_16_1,

	REG_MEM_8_ONLY,	// one operand
	REG_MEM_16_ONLY,

	AL_IMMED8,
	AX_IMMED8,
	AX_IMMED16,
	IMMED8_TO_REG,
	IMMED16_TO_REG,

	SHORT_LABEL,
	MULTI_IMMED8,

	SEGREG_0,	// NEED TO IMPLEMENT
	SEGREG_1,	// differs by a D bit
	EXCHANGE,	// NEED TO IMPLEMENT

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

//enum segOverModes {		// test for segment override (only ever applies to EAC calc)
//	SS = 1,
//	DS = 2,
//	CS = 3,
//	ES = 4
//} segOverride;

struct Instruction {
	string data;
	opModes operMode;
	//int segOverride;
	//segOverModes segOverride;
};

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
	{"MULTI\t", MULTI_IMMED8},	// 0x80
	{"MULTI\t", MULTI_IMMED8},	// 0x81
	{"MULTI\t", MULTI_IMMED8},	// 0x82
	{"MULTI\t", MULTI_IMMED8},	// 0x83
	{"TEST\t", REG_MEM_8_0},
	{"TEST\t", REG_MEM_16_0},
	{"XCHG\t", REG_MEM_8_0},
	{"XCHG\t", REG_MEM_16_0},
	{"MOV\t", REG_MEM_8_0},
	{"MOV\t", REG_MEM_16_0},	// typo in manual??? idk
	{"MOV\t", REG_MEM_8_1},
	{"MOV\t", REG_MEM_16_1},
	{"MOV\t", SEGREG_0},		// NEED TO IMPLEMENT
	{"LEA\t", REG_MEM_16_1},	// idk, probably fine
	{"MOV\t", SEGREG_1},		// NEED TO IMPLEMENT
	{"POP\t", REG_MEM_16_1},	// idk, probably fine
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
	{"CALL\t", FAR_PROC},		// NEED TO IMPLEMENT
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
	{"MOV\t", IMMED16_TO_REG},		// 0xBF
	{"**NOT USED**", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"RET\t", IMMED16},
	{"RET\t", NO_OP_ENC},		// idk; says (intrasegment)
	{"LES\t", REG_MEM_16_1},	// idk, probably fine
	{"LDS\t", REG_MEM_16_1},	// idk, probably fine
	{"MOV\t", IMMED8_TO_MEM8},		// NEED TO IMPLEMENT
	{"MOV\t", IMMED16_TO_MEM16},		// NEED TO IMPLEMENT
	{"**NOT USED**", NO_OP_ENC},	// 0xC8
	{"**NOT USED**", NO_OP_ENC},	// 0xC9
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
	{"AAM", AA_SKIP},				// 0xD4
	{"AAD", NO_OP_ENC},	
	{"**NOT USED**", NO_OP_ENC},
	{"XLAT", SOURCE_TABLE},			// NEED TO IMPLEMENT
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"MULTI", OPCODE_SOURCE},		// NEED TO IMPLEMENT; ESC
	{"LOOPNE\t", SHORT_LABEL},		// 0xE0	
	{"LOOPE\t", SHORT_LABEL},	
	{"LOOP\t", SHORT_LABEL},
	{"JCXZ\t", SHORT_LABEL},	
	{"IN\t", AL_IMMED8},
	{"IN\t", AX_IMMED8},
	{"OUT\t", AL_IMMED8},
	{"OUT\t", AX_IMMED8},
	{"CALL\t", NEAR_PROC},		// NEED TO IMPLEMENT
	{"JMP\t", NEAR_LABEL},		// NEED TO IMPLEMENT
	{"JMP\t", FAR_LABEL},		// NEED TO IMPLEMENT
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


//const Instruction instrs[256] {
//	{"ADD\t", REG_MEM_8_0, 0},
//	{"ADD\t", REG_MEM_16_0, 0},
//	{"ADD\t", REG_MEM_8_1, 0},
//	{"ADD\t", REG_MEM_16_1, 0},
//	{"ADD\t", IMMED8, 0},
//	{"ADD\t", IMMED16, 0},
//	{"PUSH\tES", NO_OP_ENC, 0},
//	{"POP\tES", NO_OP_ENC, 0},
//	{"OR\t", REG_MEM_8_0, 0},
//	{"OR\t", REG_MEM_16_0, 0},
//	{"OR\t", REG_MEM_8_1, 0},
//	{"OR\t", REG_MEM_16_1, 0},
//	{"OR\t", IMMED8, 0},
//	{"OR\t", IMMED16, 0},
//	{"PUSH\tCS", NO_OP_ENC, 0},
//	{"ADC\t", REG_MEM_8_0, 0},
//	{"ADC\t", REG_MEM_16_0, 0},
//	{"ADC\t", REG_MEM_8_1, 0},
//	{"ADC\t", REG_MEM_16_1, 0},
//	{"ADC\t", IMMED8, 0},
//	{"ADC\t", IMMED16, 0},
//	{"PUSH\tSS", NO_OP_ENC, 0},
//	{"POP\tSS", NO_OP_ENC, 0},
//	{"AND\t", REG_MEM_8_0, 0},
//	{"AND\t", REG_MEM_16_0, 0},
//	{"AND\t", REG_MEM_8_1, 0},
//	{"AND\t", REG_MEM_16_1, 0},
//	{"AND\t", IMMED8, 0},
//	{"AND\t", IMMED16, 0},
//	{"ES:", OVERRIDE, ES},
//	{"CS:", OVERRIDE, CS},
//	{"DAA", NO_OP_ENC, 0},
//
//	
//};

void printInstruction(int byte, int segOverride)
{
	Instruction instr = instrs[byte];

	/*if (byte == 0x26) {
		instr = instrs[29];
	}*/

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
			//cout << instr.data << immedOperands(0) << endl;
			break;
		case IMMED16_TO_REG:
			cout << instr.data << immedToRegister(byte, 1) << endl;
			//cout << instr.data << immedOperands(1) << endl;
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

/*
	NOTE: I'm not sure if I'm handling 16-bit sign extension in immedOperands()
	correctly. Currently, I'm just reading in 16 bits and not doing any adjustments to
	the read value. This might be fine, it might not. I recall needing to put a '+'
	or a '-' somewhere.
*/
int main()
{
	file.open("test.txt", ios::in | ios::out);

	if (!file) {
		std::cout << "Error: readFile: file \"" << "test.txt" << "\" could not be opened" << endl;
		return -1;
	}

	//int byte = readNumBits(8);		// 0000000011010110
	//string instruction = decodeInstr(byte);
	//cout << instruction << endl;

	
	//auto t1 = std::chrono::high_resolution_clock::now();
	//for (int i = 0; i < 5; ++i) {
	//	int byte = readNumBits(8);		
	//	string instruction = decodeInstr(byte);
	//	cout << instruction << endl;
	//}
	///*for (int i = 0; i < 5; ++i) {
	//	int byte = readNumBits(8);
	//	printInstruction(byte);
	//}*/
	//auto t2 = std::chrono::high_resolution_clock::now();

	//std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	//std::cout << ms_double.count() << "ms\n";

	/*for (int i = 0; i < 5; ++i) {
		int byte = readNumBits(8);
		cout << IP << ": ";
		printInstruction(byte);
	}*/

	/*int byte = readNumBits(8);
	printInstruction(byte);*/
	
	int byte = 0;
	while ((byte = readNumBits(8)) >= 0) {
		printInstruction(byte);
	}

	return 0;
}

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

//*************************
// TO-DO:
//		1) want to see how segment override prefix works
//		   will probably have to change all returns ins EAClookup
//		   to just appending to string; add new argument to 
//		   EAClookup as well to test for segment override
//*************************

//string EAClookup(int MOD, int R_M, int W)
//{
//	string output = "";
//
//	switch (MOD) {
//	case 0b00:		// MOD == 00; no displacement
//		switch (R_M) {
//		case 0b000:
//			output += "[BX+SI]";
//			break;
//		case 0b001:
//			output += "[BX+DI]";
//			break;
//		case 0b010:
//			output += "[BP+SI]";
//			break;
//		case 0b011:
//			output += "[BP+DI]";
//			break;
//		case 0b100:
//			output += "[SI]";
//			break;
//		case 0b101:
//			output += "[DI]";
//			break;
//		case 0b110: {
//			/*DIRECT ADDRESS case; need to do readNumBits(16) into an int and then convert it to hex and return that as a string*/
//			int d16 = readNumBits(16);
//			output += "[" + intToHexStr(d16, 16) + "]";
//			break;
//		}
//		case 0b111:
//			output += "[BX]";
//			break;
//		}
//		break;	// break statements superfluous???
//	case 0b01: {	// MOD == 01; 8-bit displacement
//		int d8 = readNumBits(8);
//		//stringstream s;
//		//s << hex << d8;
//		switch (R_M) {
//		case 0b000:
//			output += "[BX+SI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b001:
//			output += "[BX+DI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b010:
//			output += "[BP+SI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b011:
//			output += "[BP+DI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b100:
//			output += "[SI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b101:
//			output += "[DI+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b110:
//			output += "[BP+" + intToHexStr(d8, 8) + "]";
//			break;
//		case 0b111:
//			output += "[BX+" + intToHexStr(d8, 8) + "]";
//			break;
//		}
//		break;
//	}
//	case 0b10: {	// MOD == 10; 16-bit displacement
//		int d16 = readNumBits(16);
//		//stringstream s;
//		//s << hex << d16;
//		switch (R_M) {
//		case 0b000:
//			output += "[BX+SI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b001:
//			output += "[BX+DI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b010:
//			output += "[BP+SI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b011:
//			output += "[BP+DI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b100:
//			output += "[SI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b101:
//			output += "[DI+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b110:
//			output += "[BP+" + intToHexStr(d16, 16) + "]";
//			break;
//		case 0b111:
//			output += "[BX+" + intToHexStr(d16, 16) + "]";
//			break;
//		}
//		break;
//	}
//	case 0b11:		// MOD == 11; register mode (no displacement)
//		output += registerLookup(R_M, W);
//		break;
//	}
//
//	if (!output.empty())
//		return output;
//
//	return "[COULDN'T FIND EAC]";
//}

string EAClookup(int MOD, int R_M, int W, int segOverride)
{
	string output = "";

	switch (segOverride) {
		//case NO_SEG_OV:		// override defaults to 0 (NO_SEG_OV)
		//	break;
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
					/*DIRECT ADDRESS case; need to do readNumBits(16) into an int and then convert it to hex and return that as a string*/
					int d16 = readNumBits(16);
					output += "[" + intToHexStr(d16, 16) + "]";
					break;
				}
				case 0b111:
					output += "[BX]";
					break;
			}
			break;	// break statements superfluous???
		case 0b01: {	// MOD == 01; 8-bit displacement
			int d8 = readNumBits(8);
			//stringstream s;
			//s << hex << d8;

			// TO-DO: if 8 bit displacement, need to SIGN EXTEND
			int comp = get8bit2sComp(d8);
			switch (R_M) {
				case 0b000:
					output += "[BX+SI";		// this case's code is correct. can add this to the others.
					if (comp < 0)
						output += "-";
					else
						output += "+";
					output += intToHexStr(abs(get8bit2sComp(d8)), 8) + "]";
					//output += "[BX+SI+" + intToHexStr(d8, 8) + "]";
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
			//stringstream s;
			//s << hex << d16;

			// TO-DO: if 16-bit displacement, need to put most significant bit second (not a big deal)

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

//string EAClookup(int MOD, int R_M, int W)
//{
//	switch (MOD) {
//		case 0b00:		// MOD == 00; no displacement
//			switch (R_M) {
//				case 0b000:
//					return "[BX+SI]";
//				case 0b001:
//					return "[BX+DI]";
//				case 0b010:
//					return "[BP+SI]";
//				case 0b011:
//					return "[BP+DI]";
//				case 0b100:
//					return "[SI]";
//				case 0b101:
//					return "[DI]";
//				case 0b110: {
//					/*DIRECT ADDRESS case; need to do readNumBits(16) into an int and then convert it to hex and return that as a string*/
//					int d16 = readNumBits(16);
//					return "[" + intToHexStr(d16, 16) + "]";
//				}
//				case 0b111:
//					return "[BX]";
//			}
//			break;	// break statements superfluous???
//		case 0b01: {	// MOD == 01; 8-bit displacement
//			int d8 = readNumBits(8);
//			//stringstream s;
//			//s << hex << d8;
//			switch (R_M) {
//				case 0b000:
//					return "[BX+SI+" + intToHexStr(d8, 8) + "]";
//				case 0b001:
//					return "[BX+DI+" + intToHexStr(d8, 8) + "]";
//				case 0b010:
//					return "[BP+SI+" + intToHexStr(d8, 8) + "]";
//				case 0b011:
//					return "[BP+DI+" + intToHexStr(d8, 8) + "]";
//				case 0b100:
//					return "[SI+" + intToHexStr(d8, 8) + "]";
//				case 0b101:
//					return "[DI+" + intToHexStr(d8, 8) + "]";
//				case 0b110:
//					return "[BP+" + intToHexStr(d8, 8) + "]";
//				case 0b111:
//					return "[BX+" + intToHexStr(d8, 8) + "]";
//			}
//			break;
//		}
//		case 0b10: {	// MOD == 10; 16-bit displacement
//			int d16 = readNumBits(16);
//			//stringstream s;
//			//s << hex << d16;
//			switch (R_M) {
//				case 0b000:
//					return "[BX+SI+" + intToHexStr(d16, 16) + "]";
//				case 0b001:
//					return "[BX+DI+" + intToHexStr(d16, 16) + "]";
//				case 0b010:
//					return "[BP+SI+" + intToHexStr(d16, 16) + "]";
//				case 0b011:
//					return "[BP+DI+" + intToHexStr(d16, 16) + "]";
//				case 0b100:
//					return "[SI+" + intToHexStr(d16, 16) + "]";
//				case 0b101:
//					return "[DI+" + intToHexStr(d16, 16) + "]";
//				case 0b110:
//					return "[BP+" + intToHexStr(d16, 16) + "]";
//				case 0b111:
//					return "[BX+" + intToHexStr(d16, 16) + "]";
//			}
//			break;
//		}
//		case 0b11:		// MOD == 11; register mode (no displacement)
//			return registerLookup(R_M, W);
//			break;
//	}
//
//	return "[COULDN'T FIND EAC]";
//}

// test cases
//000000010101000111101111 = ADD     [BX+DI+EF],DX		
//000001011110111110101011 = ADD     AX,[EFAB]
//000010000101010011101111 = OR      [SI+EF],DL
//00100110000000010101000111101111 =	ADD     ES:[BX+DI+EF],DX

string regMemOnly(int W, int segOverride)
{
	string EACStr;

	int mod = readNumBits(2);
	int reg = readNumBits(3);
	int r_m = readNumBits(3);

	EACStr = EAClookup(mod, r_m, W, segOverride);

	return EACStr;
}

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

string immedData(int W)	// can delete this function, it doesn't do much. DO LATER THOUGH, DONT WORRY ABOUT IT NOW
{	// need brackets??? ( i dont think so)
	if (W)
		return /*"[" +*/ intToHexStr(readNumBits(16), 16) /*+ "]"*/;
	return /*"[" +*/ intToHexStr(readNumBits(8), 8)/* + "]"*/;
}

string getRegFromOp(int opcode, int W)
{
	int reg = 0;
	//int W = 0;
	int mask = 1;

	for (int i = 0; i < 3; ++i) {	// extract 3 rightmost bits into num
		reg += (opcode & mask);
		mask <<= 1;
	}
	/*W += (opcode & mask);*/

	/*cout << "reg: " << reg << endl;
	cout << "w: " << W << endl;*/

	return registerLookup(reg, W);
}

string immedToRegister(int opcode, int W)
{
	string reg = getRegFromOp(opcode, W);
	return reg + "," + immedData(W);
	//if (W) {	// W == 1; 16-bit
	//	int data = readNumBits(16);
	//	return "AX,[" + intToHexStr(data, 16) + "]";
	//}
	//else {		// W == 0; 8-bit
	//	int data = readNumBits(8);
	//	return "AL,[" + intToHexStr(data, 8) + "]";
	//}
}

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

string shortLabel()
{
	int num = 0;
	int data = readNumBits(8);

	num = get8bit2sComp(data);		// signed, but don't need to put +/-

	return intToHexStr(num + IP + 2, 16);		// add an extra two to account for the IP moving past this instruction as well
}
//string regMemOperands(int W, int D, int segOverride)
//{
//	string regStr, EACStr;
//
//	int mod = readNumBits(2);
//	int reg = readNumBits(3);
//	int r_m = readNumBits(3);
//
//	EACStr = EAClookup(mod, r_m, W, segOverride);
//	regStr = registerLookup(reg, W);
//
//	if (D) {	// D == 1; reg first
//		return regStr + "," + EACStr;
//	}
//	else {		// D == 0; EAC first
//		return EACStr + "," + regStr;
//	}
//}

string incExtendLookup(int op)
{
	switch (op) {
		case 0b000:
			return "INC\t";
		case 0b001:
			return "DEC\t";
		case 0b010:
			return "CALL\t";
		case 0b011:
			return "CALL\t";
		case 0b100:
			return "JMP\t";
		case 0b101:
			return "JMP\t";
		case 0b110:
			return "PUSH\t";
		case 0b111:
			return "NOT USED\t";
		default:
			return "OP_EXTEND NOT RECOGNIZED: incExtendLookup()";
	}
}

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
	//outputStr += incExtendLookup(opExtend);
	


	return outputStr;
}

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

// ONLY NEED TO ADD +/- ON DATA-SX
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
			//TO-DO: need to put +/-
			//outputStr += "," + intToHexStr(get8bit2sComp(readNumBits(8)), 8);
			break;
	}

	return outputStr;
}

//string decodeInstr(int byte, int segOverride)
//{
//	switch (byte) {
//		case 0x00:	// ADD	REG8/MEM8,REG8
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "ADD\t" + regMemOperands(0, 0, segOverride);
//			//string output = "ADD\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
//			//return output;
//		}
//		case 0x01:	// ADD	REG16/MEM16,REG16
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//			
//			return "ADD\t" + regMemOperands(1, 0, segOverride);
//			/*string output = "ADD\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
//			return output;*/
//		}
//		case 0x02:	// ADD	REG8,REG8/MEM8
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "ADD\t" + regMemOperands(0, 1, segOverride);
//			/*string output = "ADD\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
//			return output;*/
//		}
//		case 0x03:	// ADD	REG16,REG16/MEM16
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "ADD\t" + regMemOperands(1, 1, segOverride);
//			/*string output = "ADD\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
//			return output;*/
//		}
//		case 0x04:	// ADD	AL,IMMED8
//		{
//			return "ADD\t" + immedOperands(0);
//			/*int data = readNumBits(8);
//
//			string output = "ADD\tAL,[" + intToHexStr(data, 8) + "]";
//			return output;*/
//		}
//		case 0x05:	// ADD	AX,IMMED16
//		{
//			return "ADD\t" + immedOperands(16);
//			/*int data = readNumBits(16);
//
//			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
//			return output;*/
//		}
//		case 0x06:	// PUSH	ES
//		{
//			return "PUSH\tES";
//		}
//		case 0x07:	// POP	ES
//		{
//			return "POP\tES";
//		}
//		case 0x08:	// OR	REG8/MEM8,REG8
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "OR\t" + regMemOperands(0, 0, segOverride);
//			/*string output = "OR\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
//			return output;*/
//		}
//		case 0x09:	// OR	REG16/MEM16,REG16
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "OR\t" + regMemOperands(1, 0, segOverride);
//			/*string output = "OR\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
//			return output;*/
//		}
//		case 0x0A:	// OR	REG8,REG8/MEM8
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "OR\t" + regMemOperands(0, 1, segOverride);
//			/*string output = "OR\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
//			return output;*/
//		}
//		case 0x0B:	// OR	REG16,REG16/MEM16
//		{
//			/*int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);*/
//
//			return "OR\t" + regMemOperands(1, 1, segOverride);
//			/*string output = "OR\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
//			return output;*/
//		}
//		case 0x0C:	// OR	AL,IMMED8
//		{
//			return "OR\t" + immedOperands(0);
//			/*int data = readNumBits(8);
//
//			string output = "OR\tAL,[" + intToHexStr(data, 8) + "]";
//			return output;*/
//		}
//		case 0x0D:	// OR	AX,IMMED16
//		{
//			return "OR\t" + immedOperands(1);
//			/*int data = readNumBits(16);
//
//			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
//			return output;*/
//		}
//		case 0x0E:	// PUSH	CS
//		{
//			return "PUSH\tCS";
//		}
//		case 0x0F:	//***NOT USED***
//		{
//			return "NOT USED";
//		}
//		case 0x10:	// ADC	REG8/MEM8,REG8
//		{
//			return "ADC\t" + regMemOperands(0, 0, segOverride);
//		}
//		case 0x11:	// ADC	REG16/MEM16,REG16
//		{
//			return "ADC\t" + regMemOperands(1, 0, segOverride);
//		}
//		case 0x12:	// ADC	REG8,REG8/MEM8
//		{
//			return "ADC\t" + regMemOperands(0, 1, segOverride);
//		}
//		case 0x13:	// ADC REG16,REG16/MEM16
//		{
//			return "ADC\t" + regMemOperands(1, 1, segOverride);
//		}
//		case 0x14:	// ADC	AL,IMMED8
//		{
//			return "ADC\t" + immedOperands(0);
//		}
//		case 0x15:	// ADC	AL,IMMED16
//		{
//			return "ADC\t" + immedOperands(1);
//		}
//		case 0x16:	// PUSH	SS
//		{
//			return "PUSH\tSS";
//		}
//		case 0x17:	// POP SS
//		{
//			return "POP\tSS";
//		}
//		case 0x18:	// SBB	REG8/MEM8,REG8
//		{
//			return "SBB\t" + regMemOperands(0, 0, segOverride);
//		}
//		case 0x19:	// SBB	REG16/MEM16,REG16
//		{
//			return "SBB\t" + regMemOperands(1, 0, segOverride);
//		}
//		case 0x1A:	// SBB	REG8,REG8/MEM8
//		{
//			return "SBB\t" + regMemOperands(0, 1, segOverride);
//		}
//		case 0x1B:	// SBB	REG16,REG16/MEM16
//		{
//			return "SBB\t" + regMemOperands(1, 1, segOverride);
//		}
//		case 0x1C:	// SBB	AL,IMMED8
//		{
//			return "SBB\t" + immedOperands(0);
//		}
//		case 0x1D:	// SBB	AX,IMMED16
//		{
//			return "SBB\t" + immedOperands(1);
//		}
//		case 0x1E:	// PUSH	DS
//		{
//			return "PUSH\tDS";
//		}
//		case 0x1F:	// POP	DS
//		{
//			return "POP\tDS";
//		}
//		case 0x20:	// AND	REG8/MEM8,REG8
//		{
//			return "AND\t" + regMemOperands(0, 0, segOverride);
//		}
//		case 0x21:	// AND	REG16/MEM16,REG16
//		{
//			return "AND\t" + regMemOperands(1, 0, segOverride);
//		}
//		case 0x22:	// AND	REG8,REG8/MEM8
//		{
//			return "AND\t" + regMemOperands(0, 1, segOverride);
//		}
//		case 0x23:	// AND	REG16,REG16/MEM16
//		{
//			return "AND\t" + regMemOperands(1, 1, segOverride);
//		}
//		case 0x24:	// AND	AL,IMMED8
//		{
//			return "AND\t" + immedOperands(0);
//		}
//		case 0x25:	// AND	AX,IMMED16
//		{
//			return "AND\t" + immedOperands(1);
//		}
//		case 0x26:	// ES: segment override prefix
//		{
//			return decodeInstr(readNumBits(8), ES);
//		}
//		case 0x27:	// DAA
//		{
//			return "DAA";
//		}
//		case 0x28:	// SUB	REG8/MEM8,REG8
//		{
//			return "SUB\t" + regMemOperands(0, 0, segOverride);
//		}
//		case 0x29:	// SUB	REG16/MEM16,REG16
//		{
//			return "SUB\t" + regMemOperands(1, 0, segOverride);
//		}
//		case 0x2A:	// SUB	REG8,REG8/MEM8
//		{
//			return "SUB\t" + regMemOperands(0, 1, segOverride);
//		}
//		case 0x2B:	// SUB	REG16,REG16/MEM16
//		{
//			return "SUB\t" + regMemOperands(1, 1, segOverride);
//		}
//		case 0x2C:	// SUB	AL,IMMED8
//		{
//			return "SUB\t" + immedOperands(0);
//		}
//		case 0x2D:	// SUB	AX,IMMED16
//		{
//			return "SUB\t" + immedOperands(1);
//		}
//		default:
//			return "COULDN'T DECODE INSTRUCTION";
//			break;
//	}
//}

//string decodeInstr(int byte)
//{
//	switch (byte) {
//		case 0x00:	// ADD	REG8/MEM8,REG8
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "ADD\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0);
//			return output;
//		}
//		case 0x01:	// ADD	REG16/MEM16,REG16
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "ADD\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1);
//			return output;
//		}
//		case 0x02:	// ADD	REG8,REG8/MEM8
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "ADD\t" + EAClookup(mod, r_m, 0) + "," + registerLookup(reg, 0);
//			return output;
//		}
//		case 0x03:	// ADD	REG16,REG16/MEM16
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "ADD\t" + EAClookup(mod, r_m, 1) + "," + registerLookup(reg, 1);
//			return output;
//		}
//		case 0x04:	// ADD	AL,IMMED8
//		{
//			int data = readNumBits(8);
//
//			string output = "ADD\tAL,[" + intToHexStr(data, 8) + "]";
//			return output;
//		}
//		case 0x05:	// ADD	AX,IMMED16
//		{
//			int data = readNumBits(16);
//
//			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
//			return output;
//		}
//		case 0x06:	// PUSH	ES
//		{
//			return "PUSH\tES";
//		}
//		case 0x07:	// POP	ES
//		{
//			return "POP\tES";
//		}
//		case 0x08:	// OR	REG8/MEM8,REG8
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "OR\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0);
//			return output;
//		}
//		case 0x09:	// OR	REG16/MEM16,REG16
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "OR\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1);
//			return output;
//		}
//		case 0x0A:	// OR	REG8,REG8/MEM8
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "OR\t" + EAClookup(mod, r_m, 0) + "," + registerLookup(reg, 0);
//			return output;
//		}
//		case 0x0B:	// OR	REG16,REG16/MEM16
//		{
//			int mod = readNumBits(2);
//			int reg = readNumBits(3);
//			int r_m = readNumBits(3);
//
//			string output = "OR\t" + EAClookup(mod, r_m, 1) + "," + registerLookup(reg, 1);
//			return output;
//		}
//		case 0x0C:	// OR	AL,IMMED8
//		{
//			int data = readNumBits(8);
//
//			string output = "OR\tAL,[" + intToHexStr(data, 8) + "]";
//			return output;
//		}
//		case 0x0D:	// OR	AX,IMMED16
//		{
//			int data = readNumBits(16);
//
//			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
//			return output;
//		}
//		case 0x0E:	// PUSH	CS
//		{
//			return "PUSH\tCS";
//		}
//		case 0x0F:	//***NOT USED***
//		{
//			return "NOT USED";
//		}
//
//
//		case 0x26:
//		{
//			string output = decodeInstr(readNumBits(8));
//		}
//		default:
//			return "COULDN'T DECODE INSTRUCTION";
//			break;
//	}
//}

//string intToHexStr(int i, int widthInBits)
//{
//	stringstream s;
//	s << uppercase << setfill('0') << setw(widthInBits / 4) << std::hex << i;
//	return s.str();
//}
string intToHexStr(int num, int widthInBits)		// formatted with least significant byte first
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
			//cout << "reading num bits: " << num << endl;
			return bits;
		}
	}

	cout << "file not good! readNumBits()" << endl;
	return -1;
}