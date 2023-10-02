#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

ifstream file;

enum {		// test for segment override (only ever applies to EAC calc)
	NO_SEG_OV = 0,
	SS = 1,
	DS = 2,
	CS = 3,
	ES = 4
};


// Function prototypes
string registerLookup(int reg, int W);
string EAClookup(int MOD, int R_M, int W, int segOverride = 0);
//string EAClookup(int MOD, int R_M, int W = 0);
string decodeInstr(int byte, int override = NO_SEG_OV);
string intToHexStr(int i, int widthInBits);
int readNumBits(int num);

int main()
{
	file.open("test.txt", ios::in | ios::out);

	if (!file) {
		std::cout << "Error: readFile: file \"" << "test.txt" << "\" could not be opened" << endl;
		return -1;
	}

	int byte = readNumBits(8);		// 0000000011010110
	string instruction = decodeInstr(byte);
	cout << instruction << endl;

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

string EAClookup(int MOD, int R_M, int W, int segOverride)
{
	string output = "";

	switch (segOverride) {
		case NO_SEG_OV:		// override defaults to 0 (NO_SEG_OV)
			break;
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
//000000010101000111101111 = ADD     DX,[BX+DI+EF]		
//000001011110111110101011 = ADD     AX,[EFAB]
//000010000101010011101111 = OR      DL,[SI+EF]

string decodeInstr(int byte, int segOverride)
{
	switch (byte) {
		case 0x00:	// ADD	REG8/MEM8,REG8
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "ADD\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
			return output;
		}
		case 0x01:	// ADD	REG16/MEM16,REG16
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "ADD\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
			return output;
		}
		case 0x02:	// ADD	REG8,REG8/MEM8
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "ADD\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
			return output;
		}
		case 0x03:	// ADD	REG16,REG16/MEM16
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "ADD\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
			return output;
		}
		case 0x04:	// ADD	AL,IMMED8
		{
			int data = readNumBits(8);

			string output = "ADD\tAL,[" + intToHexStr(data, 8) + "]";
			return output;
		}
		case 0x05:	// ADD	AX,IMMED16
		{
			int data = readNumBits(16);

			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
			return output;
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
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "OR\t" + registerLookup(reg, 0) + "," + EAClookup(mod, r_m, 0, segOverride);
			return output;
		}
		case 0x09:	// OR	REG16/MEM16,REG16
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "OR\t" + registerLookup(reg, 1) + "," + EAClookup(mod, r_m, 1, segOverride);
			return output;
		}
		case 0x0A:	// OR	REG8,REG8/MEM8
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "OR\t" + EAClookup(mod, r_m, 0, segOverride) + "," + registerLookup(reg, 0);
			return output;
		}
		case 0x0B:	// OR	REG16,REG16/MEM16
		{
			int mod = readNumBits(2);
			int reg = readNumBits(3);
			int r_m = readNumBits(3);

			string output = "OR\t" + EAClookup(mod, r_m, 1, segOverride) + "," + registerLookup(reg, 1);
			return output;
		}
		case 0x0C:	// OR	AL,IMMED8
		{
			int data = readNumBits(8);

			string output = "OR\tAL,[" + intToHexStr(data, 8) + "]";
			return output;
		}
		case 0x0D:	// OR	AX,IMMED16
		{
			int data = readNumBits(16);

			string output = "ADD\tAX,[" + intToHexStr(data, 16) + "]";
			return output;
		}
		case 0x0E:	// PUSH	CS
		{
			return "PUSH\tCS";
		}
		case 0x0F:	//***NOT USED***
		{
			return "NOT USED";
		}


		// testing segment overrides
		case 0x26:
		{
			return decodeInstr(readNumBits(8), ES);
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