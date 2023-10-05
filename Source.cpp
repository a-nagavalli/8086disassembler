#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>


#include <chrono>


using namespace std;

ifstream file;

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
//string EAClookup(int MOD, int R_M, int W);
//string EAClookup(int MOD, int R_M, int W = 0);
string decodeInstr(int byte, int override/* = NO_SEG_OV*/);
string intToHexStr(int i, int widthInBits);
int readNumBits(int num);

// operand encoding
string regMemOperands(int W, int D, int segOverride);
string immedOperands(int W);
string shortLabel();

void printInstruction(int byte, int segOverride = 0);


enum opModes {
	NO_OP_ENC,
	REG_MEM_8_0,
	REG_MEM_16_0,
	REG_MEM_8_1,
	REG_MEM_16_1,
	IMMED8,
	IMMED16,
	SHORT_LABEL,


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
	{"ADD\t", IMMED8},
	{"ADD\t", IMMED16},
	{"PUSH\tES", NO_OP_ENC},
	{"POP\tES", NO_OP_ENC},
	{"OR\t", REG_MEM_8_0},
	{"OR\t", REG_MEM_16_0},
	{"OR\t", REG_MEM_8_1},
	{"OR\t", REG_MEM_16_1},
	{"OR\t", IMMED8},
	{"OR\t", IMMED16},
	{"PUSH\tCS", NO_OP_ENC},
	{"**NOT USED**", NO_OP_ENC},
	{"ADC\t", REG_MEM_8_0},
	{"ADC\t", REG_MEM_16_0},
	{"ADC\t", REG_MEM_8_1},
	{"ADC\t", REG_MEM_16_1},
	{"ADC\t", IMMED8},
	{"ADC\t", IMMED16},
	{"PUSH\tSS", NO_OP_ENC},
	{"POP\tSS", NO_OP_ENC},
	{"SBB\t", REG_MEM_8_0},
	{"SBB\t", REG_MEM_16_0},
	{"SBB\t", REG_MEM_8_1},
	{"SBB\t", REG_MEM_16_1},
	{"SBB\t", IMMED8},
	{"SBB\t", IMMED16},
	{"PUSH\tDS", NO_OP_ENC},
	{"POP\tDS", NO_OP_ENC},
	{"AND\t", REG_MEM_8_0},
	{"AND\t", REG_MEM_16_0},
	{"AND\t", REG_MEM_8_1},
	{"AND\t", REG_MEM_16_1},
	{"AND\t", IMMED8},
	{"AND\t", IMMED16},
	{"ES:", ES},
	{"DAA", NO_OP_ENC},
	{"SUB\t", REG_MEM_8_0},
	{"SUB\t", REG_MEM_16_0},
	{"SUB\t", REG_MEM_8_1},
	{"SUB\t", REG_MEM_16_1},
	{"SUB\t", IMMED8},
	{"SUB\t", IMMED16},
	{"CS:", CS},
	{"DAS", NO_OP_ENC},
	{"XOR\t", REG_MEM_8_0},
	{"XOR\t", REG_MEM_16_0},
	{"XOR\t", REG_MEM_8_1},
	{"XOR\t", REG_MEM_16_1},
	{"XOR\t", IMMED8},
	{"XOR\t", IMMED16},
	{"SS:", SS},
	{"AAA", NO_OP_ENC},
	{"CMP\t", REG_MEM_8_0},
	{"CMP\t", REG_MEM_16_0},
	{"CMP\t", REG_MEM_8_1},
	{"CMP\t", REG_MEM_16_1},
	{"CMP\t", IMMED8},
	{"CMP\t", IMMED16},
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
		case IMMED8:
			cout << instr.data << immedOperands(0) << endl;
			break;
		case IMMED16:
			cout << instr.data << immedOperands(1) << endl;
			break;
		case SHORT_LABEL:
			cout << instr.data << shortLabel() << endl;
			break;
	}
}

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

	int byte = readNumBits(8);
	printInstruction(byte);


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
			//stringstream s;
			//s << hex << d16;
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

string immedOperands(int W)
{
	if (W) {	// W == 1; 16-bit
		int data = readNumBits(16);
		return "AX,[" + intToHexStr(data, 16) + "]";
	}
	else {		// W == 0; 8-bit
		int data = readNumBits(8);
		return "AL,[" + intToHexStr(data, 8) + "]";
	}
}

string shortLabel()
{
	int data = readNumBits(8);
	return 
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



string decodeInstr(int byte, int segOverride)
{
	switch (byte) {
		case 0x00:	// ADD	REG8/MEM8,REG8
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "ADD\t" + regMemOperands(0, 0, segOverride);
			//string output = "ADD\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
			//return output;
		}
		case 0x01:	// ADD	REG16/MEM16,REG16
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/
			
			return "ADD\t" + regMemOperands(1, 0, segOverride);
			/*string output = "ADD\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
			return output;*/
		}
		case 0x02:	// ADD	REG8,REG8/MEM8
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "ADD\t" + regMemOperands(0, 1, segOverride);
			/*string output = "ADD\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
			return output;*/
		}
		case 0x03:	// ADD	REG16,REG16/MEM16
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "ADD\t" + regMemOperands(1, 1, segOverride);
			/*string output = "ADD\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
			return output;*/
		}
		case 0x04:	// ADD	AL,IMMED8
		{
			return "ADD\t" + immedOperands(0);
			/*int data = readNumBits(8);

			string output = "ADD\tAL,[" + intToHexStr(data, 8) + "]";
			return output;*/
		}
		case 0x05:	// ADD	AX,IMMED16
		{
			return "ADD\t" + immedOperands(16);
			/*int data = readNumBits(16);

			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
			return output;*/
		}
		case 0x06:	// PUSH	ES
		{
			return "PUSH\tES";
		}
		case 0x07:	// POP	ES
		{
			return "POP\tES";
		}
		case 0x08:	// OR	REG8/MEM8,REG8
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "OR\t" + regMemOperands(0, 0, segOverride);
			/*string output = "OR\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
			return output;*/
		}
		case 0x09:	// OR	REG16/MEM16,REG16
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "OR\t" + regMemOperands(1, 0, segOverride);
			/*string output = "OR\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
			return output;*/
		}
		case 0x0A:	// OR	REG8,REG8/MEM8
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "OR\t" + regMemOperands(0, 1, segOverride);
			/*string output = "OR\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
			return output;*/
		}
		case 0x0B:	// OR	REG16,REG16/MEM16
		{
			/*int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);*/

			return "OR\t" + regMemOperands(1, 1, segOverride);
			/*string output = "OR\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
			return output;*/
		}
		case 0x0C:	// OR	AL,IMMED8
		{
			return "OR\t" + immedOperands(0);
			/*int data = readNumBits(8);

			string output = "OR\tAL,[" + intToHexStr(data, 8) + "]";
			return output;*/
		}
		case 0x0D:	// OR	AX,IMMED16
		{
			return "OR\t" + immedOperands(1);
			/*int data = readNumBits(16);

			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
			return output;*/
		}
		case 0x0E:	// PUSH	CS
		{
			return "PUSH\tCS";
		}
		case 0x0F:	//***NOT USED***
		{
			return "NOT USED";
		}
		case 0x10:	// ADC	REG8/MEM8,REG8
		{
			return "ADC\t" + regMemOperands(0, 0, segOverride);
		}
		case 0x11:	// ADC	REG16/MEM16,REG16
		{
			return "ADC\t" + regMemOperands(1, 0, segOverride);
		}
		case 0x12:	// ADC	REG8,REG8/MEM8
		{
			return "ADC\t" + regMemOperands(0, 1, segOverride);
		}
		case 0x13:	// ADC REG16,REG16/MEM16
		{
			return "ADC\t" + regMemOperands(1, 1, segOverride);
		}
		case 0x14:	// ADC	AL,IMMED8
		{
			return "ADC\t" + immedOperands(0);
		}
		case 0x15:	// ADC	AL,IMMED16
		{
			return "ADC\t" + immedOperands(1);
		}
		case 0x16:	// PUSH	SS
		{
			return "PUSH\tSS";
		}
		case 0x17:	// POP SS
		{
			return "POP\tSS";
		}
		case 0x18:	// SBB	REG8/MEM8,REG8
		{
			return "SBB\t" + regMemOperands(0, 0, segOverride);
		}
		case 0x19:	// SBB	REG16/MEM16,REG16
		{
			return "SBB\t" + regMemOperands(1, 0, segOverride);
		}
		case 0x1A:	// SBB	REG8,REG8/MEM8
		{
			return "SBB\t" + regMemOperands(0, 1, segOverride);
		}
		case 0x1B:	// SBB	REG16,REG16/MEM16
		{
			return "SBB\t" + regMemOperands(1, 1, segOverride);
		}
		case 0x1C:	// SBB	AL,IMMED8
		{
			return "SBB\t" + immedOperands(0);
		}
		case 0x1D:	// SBB	AX,IMMED16
		{
			return "SBB\t" + immedOperands(1);
		}
		case 0x1E:	// PUSH	DS
		{
			return "PUSH\tDS";
		}
		case 0x1F:	// POP	DS
		{
			return "POP\tDS";
		}
		case 0x20:	// AND	REG8/MEM8,REG8
		{
			return "AND\t" + regMemOperands(0, 0, segOverride);
		}
		case 0x21:	// AND	REG16/MEM16,REG16
		{
			return "AND\t" + regMemOperands(1, 0, segOverride);
		}
		case 0x22:	// AND	REG8,REG8/MEM8
		{
			return "AND\t" + regMemOperands(0, 1, segOverride);
		}
		case 0x23:	// AND	REG16,REG16/MEM16
		{
			return "AND\t" + regMemOperands(1, 1, segOverride);
		}
		case 0x24:	// AND	AL,IMMED8
		{
			return "AND\t" + immedOperands(0);
		}
		case 0x25:	// AND	AX,IMMED16
		{
			return "AND\t" + immedOperands(1);
		}
		case 0x26:	// ES: segment override prefix
		{
			return decodeInstr(readNumBits(8), ES);
		}
		case 0x27:	// DAA
		{
			return "DAA";
		}
		case 0x28:	// SUB	REG8/MEM8,REG8
		{
			return "SUB\t" + regMemOperands(0, 0, segOverride);
		}
		case 0x29:	// SUB	REG16/MEM16,REG16
		{
			return "SUB\t" + regMemOperands(1, 0, segOverride);
		}
		case 0x2A:	// SUB	REG8,REG8/MEM8
		{
			return "SUB\t" + regMemOperands(0, 1, segOverride);
		}
		case 0x2B:	// SUB	REG16,REG16/MEM16
		{
			return "SUB\t" + regMemOperands(1, 1, segOverride);
		}
		case 0x2C:	// SUB	AL,IMMED8
		{
			return "SUB\t" + immedOperands(0);
		}
		case 0x2D:	// SUB	AX,IMMED16
		{
			return "SUB\t" + immedOperands(1);
		}
		default:
			return "COULDN'T DECODE INSTRUCTION";
			break;
	}
}

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

string intToHexStr(int i, int widthInBits)
{
	stringstream s;
	s << uppercase << setfill('0') << setw(widthInBits / 4) << std::hex << i;
	return s.str();
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

		else
			return bits;
	}

	cout << "file not good! readNumBits()" << endl;
	return -1;
}