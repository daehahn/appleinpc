// 65c02.cpp: implementation of the C6502 class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "appleclock.h"
#include "memory.h"
#include "6502.h"
#include "65c02_op.h"

#define RebootSig		0x01
#define ResetSig		0x02
#define DebugStepSig	0x04
#define EnterDebugSig	0x08

#define C_Flag			0x1			/* 6502 Carry	 	   */
#define Z_Flag			0x2			/* 6502 Zero		   */
#define I_Flag			0x4			/* 6502 Interrupt disable  */
#define D_Flag			0x8			/* 6502 Decimal mode	   */
#define B_Flag			0x10		/* 6502 Break		   */
#define X_Flag			0x20		/* 6502 Xtra		   */
#define V_Flag			0x40		/* 6502 Overflow	   */
#define N_Flag			0x80		/* 6502 Neg		   */

#define NZ_Flag			0x82
#define NZC_Flag		0x83
#define NV_Flag			0xC0
#define NVZ_Flag		0xC2
#define NVZC_Flag		0xC3
#define C_Flag_Bit	0		/* 6502 Carry		   */
#define Z_Flag_Bit	1		/* 6502 Zero		   */
#define I_Flag_Bit	2		/* 6502 Interrupt disable  */
#define D_Flag_Bit	3		/* 6502 Decimal mode	   */
#define B_Flag_Bit	4		/* 6502 Break		   */
#define X_Flag_Bit	5		/* 6502 Xtra		   */
#define V_Flag_Bit	6		/* 6502 Overflow	   */
#define N_Flag_Bit	7		/* 6502 Neg		   */

#define REFER_NMIB			0xFFFA
#define REFER_RESET			0xFFFC
#define REFER_IRQ			0xFFFE

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


C6502::C6502()
	: C65c02()
{
	m_initialized=TRUE;

#ifdef _DEBUG
	m_current=0;
#endif
//	init_6502();
}

C6502::~C6502()
{

}

int C6502::Process()
{
	register BYTE data;
	register WORD addr;
	register WORD result;
	BYTE offset;
	BYTE opcode;
	int clock = 0;
	int preclock = 0;
#ifdef _DEBUG
	WORD opcode_addr = m_regPC;
#endif
	if ( ( m_uException_Register & SIG_CPU_IRQ ) )
	{
		m_uException_Register &= ~SIG_CPU_IRQ;
		m_uException_Register &= ~SIG_CPU_WAIT;
		if ( !( m_regF & I_Flag ) && !( m_regF & B_Flag ) )
		{
			TRACE_CALL;
			PUSH( m_regPC >> 8 );
			PUSH( m_regPC & 0xFF );
			PUSH( m_regF );
			m_regPC = READMEM16( 0xFFFE );
			clock += 7;
			m_regF |= I_Flag;
			return clock;
		}
		else
		{
//			PendingIRQ++;
		}
	}

	if ( ( m_uException_Register & SIG_CPU_SHUTDOWN )
		|| ( m_uException_Register & SIG_CPU_WAIT ) )
	{
		clock += 3;
		return clock;
	}

	opcode = READOPCODE8;

	switch (opcode)
	{
		/* ADC Immediate */
	case 0x69:
		IMM; ADC; WACC; CLOCK(2); break;
		/* ADC ZeroPage */
	case 0x65:
		ZP; MEM; ADC; WACC; CLOCK(3); break;
		/* ADC ZeroPage, X */
	case 0x75:
		ZP_X; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute */
	case 0x6d:
		ABS; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute, X */
	case 0x7d:
		ABS_X; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute, Y */
	case 0x79:
		ABS_Y; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC (Indirect, X) */
	case 0x61:
		IND_X; MEM; ADC; WACC; CLOCK(6); break;
		/* ADC (Indirect), Y */
	case 0x71:
		IND_Y; MEM; ADC; WACC; CLOCK(5); break;
		/* AND Immediate */
	case 0x29:
		IMM; AND; WACC; CLOCK(2); break;
		/* AND ZeroPage */
	case 0x25:
		ZP; MEM; AND; WACC; CLOCK(3); break;
		/* AND ZeroPage, X */
	case 0x35:
		ZP_X; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute */
	case 0x2d:
		ABS; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute, X */
	case 0x3d:
		ABS_X; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute, Y */
	case 0x39:
		ABS_Y; MEM; AND; WACC; CLOCK(4); break;
		/* AND (Indirect, X) */
	case 0x21:
		IND_X; MEM; AND; WACC; CLOCK(6); break;
		/* AND (Indirect), Y */
	case 0x31:
		IND_Y; MEM; AND; WACC; CLOCK(5); break;
		/* ASL Accumulator */
	case 0x0a:
		ACC; ASL; WACC; CLOCK(2); break;
		/* ASL ZeroPage */
	case 0x06:
		ZP; MEM; ASL; WMEM; CLOCK(5); break;
		/* ASL ZeroPage, X */
	case 0x16:
		ZP_X; MEM; ASL; WMEM; CLOCK(6); break;
		/* ASL Absolute */
	case 0x0e:
		ABS; MEM; ASL; WMEM; CLOCK(6); break;
		/* ASL Absolute, X */
	case 0x1e:
		ABS_X; MEM; ASL; WMEM; CLOCK(7); break;
		/* BCC rr */
	case 0x90:
		BRA_NCOND(C_Flag); CLOCK(2); break;
		/* BCS rr */
	case 0xb0:
		BRA_COND(C_Flag); CLOCK(2); break;
		/* BEQ rr */
	case 0xf0:
		BRA_COND(Z_Flag); CLOCK(2); break;
		/* BIT ZeroPage */
	case 0x24:
		ZP; MEM; BIT; CLOCK(3); break;
		/* BIT Absolute */
	case 0x2c:
		ABS; MEM; BIT; CLOCK(3); break;
		/* BMI rr */
	case 0x30:
		BRA_COND(N_Flag); CLOCK(2); break;
		/* BNE rr */
	case 0xd0:
		BRA_NCOND(Z_Flag); CLOCK(2); break;
		/* BPL rr */
	case 0x10:
		BRA_NCOND(N_Flag); CLOCK(2); break;
		/* BRK */
	case 0x00:
		TRACE_CALL;
#ifdef _DEBUG
			TRACE("BRK at $%04X\n", m_regPC - 1);
		{
			int d_i, d_current;
			d_current = m_current;
			TRACE("Trace PC register\n");
			for (d_i = 0; d_i < 10; d_i++)
			{
				TRACE("    jmp at $%04X\n", m_trace[(--d_current) & 0xFF]);
			}
			TRACE("Stack : $%02X\n    ", m_regS);
			for (d_i = 0; d_i < 10; d_i++)
			{
				TRACE("%02X ", READMEM8(((m_regS + d_i) & 0xFF) + 0x100));
			}
			TRACE("\n");
		}
#endif
		BRK; CLOCK(7); break;
		/* BVC rr */
	case 0x50:
		BRA_NCOND(V_Flag); CLOCK(2); break;
		/* BVS rr */
	case 0x70:
		BRA_COND(V_Flag); CLOCK(2); break;
		/* CLC */
	case 0x18:
		CLEAR_FLAG(C_Flag); CLOCK(2); break;
		/* CLD */
	case 0xd8:
		CLEAR_FLAG(D_Flag); CLOCK(2); break;
		/* CLI */
	case 0x58:
		CLEAR_FLAG(I_Flag); CHECK_IRQ; CLOCK(2); break;
		/* CLV */
	case 0xb8:
		CLEAR_FLAG(V_Flag); CLOCK(2); break;
		/* CMP Immediate */
	case 0xc9:
		IMM; CMP; CLOCK(2); break;
		/* CMP ZeroPage */
	case 0xc5:
		ZP; MEM; CMP; CLOCK(3); break;
		/* CMP ZeroPage, X */
	case 0xd5:
		ZP_X; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute */
	case 0xcd:
		ABS; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute, X */
	case 0xdd:
		ABS_X; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute, Y */
	case 0xd9:
		ABS_Y; MEM; CMP; CLOCK(4); break;
		/* CMP (Indirect, X) */
	case 0xc1:
		IND_X; MEM; CMP; CLOCK(6); break;
		/* CMP (Indirect), Y */
	case 0xd1:
		IND_Y; MEM; CMP; CLOCK(5); break;
		/* CPX Immediate */
	case 0xe0:
		IMM; CPX; CLOCK(2); break;
		/* CPX ZeroPage */
	case 0xe4:
		ZP; MEM; CPX; CLOCK(3); break;
		/* CPX Absolute */
	case 0xec:
		ABS; MEM; CPX; CLOCK(4); break;
		/* CPY Immediate */
	case 0xc0:
		IMM; CPY; CLOCK(2); break;
		/* CPY ZeroPage */
	case 0xc4:
		ZP; MEM; CPY; CLOCK(3); break;
		/* CPY Absolute */
	case 0xcc:
		ABS; MEM; CPY; CLOCK(4); break;
		/* DEC ZeroPage */
	case 0xc6:
		ZP; MEM; DEC; WMEM; CLOCK(5); break;
		/* DEC ZeroPage, X */
	case 0xd6:
		ZP_X; MEM; DEC; WMEM; CLOCK(6); break;
		/* DEC Absolute */
	case 0xce:
		ABS; MEM; DEC; WMEM; CLOCK(6); break;
		/* DEC Absolute, X */
	case 0xde:
		ABS_X; MEM; DEC; WMEM; CLOCK(7); break;
		/* DEX */
	case 0xca:
		XREG; DEC; WXREG; CLOCK(2); break;
		/* DEY */
	case 0x88:
		YREG; DEC; WYREG; CLOCK(2); break;
		/* EOR Immediate */
	case 0x49:
		IMM; EOR; WACC; CLOCK(2); break;
		/* EOR ZeroPage */
	case 0x45:
		ZP; MEM; EOR; WACC; CLOCK(3); break;
		/* EOR ZeroPage, X */
	case 0x55:
		ZP_X; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute */
	case 0x4d:
		ABS; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute, X */
	case 0x5d:
		ABS_X; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute, Y */
	case 0x59:
		ABS_Y; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR (Indirect, X) */
	case 0x41:
		IND_X; MEM; EOR; WACC; CLOCK(6); break;
		/* EOR (Indirect), Y */
	case 0x51:
		IND_Y; MEM; EOR; WACC; CLOCK(5); break;
		/* INC ZeroPage */
	case 0xe6:
		ZP; MEM; INC; WMEM; CLOCK(5); break;
		/* INC ZeroPage, X */
	case 0xf6:
		ZP_X; MEM; INC; WMEM; CLOCK(6); break;
		/* INC Absolute */
	case 0xee:
		ABS; MEM; INC; WMEM; CLOCK(6); break;
		/* INC Absolute, X */
	case 0xfe:
		ABS_X; MEM; INC; WMEM; CLOCK(7); break;
		/* INX */
	case 0xe8:
		XREG; INC; WXREG; CLOCK(2); break;
		/* INY */
	case 0xc8:
		YREG; INC; WYREG; CLOCK(2); break;
		/* JMP Absolute */
	case 0x4c:
		ABS; JMP; CLOCK(3); break;
		/* JMP (Indirect16) */
	case 0x6c:
		// 65c02 : 6 cycle
		// 6502 : 5 cycle
		IND16; JMP; CLOCK(5); break;
		/* JSR Absolute */
	case 0x20:
		ABS; JSR; CLOCK(6); break;
		/* LDA Immediate */
	case 0xa9:
		IMM; LOAD; WACC; CLOCK(2); break;
		/* LDA ZeroPage */
	case 0xa5:
		ZP; MEM; LOAD; WACC; CLOCK(3); break;
		/* LDA ZeroPage, X */
	case 0xb5:
		ZP_X; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute */
	case 0xad:
		ABS; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute, X */
	case 0xbd:
		ABS_X; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute, Y */
	case 0xb9:
		ABS_Y; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA (Indirect, X) */
	case 0xa1:
		IND_X; MEM; LOAD; WACC; CLOCK(6); break;
		/* LDA (Indirect), Y */
	case 0xb1:
		IND_Y; MEM; LOAD; WACC; CLOCK(5); break;
		/* LDX Immediate */
	case 0xa2:
		IMM; LOAD; WXREG; CLOCK(2); break;
		/* LDX ZeroPage */
	case 0xa6:
		ZP; MEM; LOAD; WXREG; CLOCK(3); break;
		/* LDX ZeroPage, Y */
	case 0xb6:
		ZP_Y; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDX Absolute */
	case 0xae:
		ABS; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDX Absolute, Y */
	case 0xbe:
		ABS_Y; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDY Immediate */
	case 0xa0:
		IMM; LOAD; WYREG; CLOCK(2); break;
		/* LDY ZeroPage */
	case 0xa4:
		ZP; MEM; LOAD; WYREG; CLOCK(3); break;
		/* LDY ZeroPage, X */
	case 0xb4:
		ZP_X; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LDY Absolute */
	case 0xac:
		ABS; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LDY Absolute, X */
	case 0xbc:
		ABS_X; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LSR Accmulator */
	case 0x4a:
		ACC; LSR; WACC; CLOCK(2); break;
		/* LSR ZeroPage */
	case 0x46:
		ZP; MEM; LSR; WMEM; CLOCK(5); break;
		/* LSR ZeroPage, X */
	case 0x56:
		ZP_X; MEM; LSR; WMEM; CLOCK(6); break;
		/* LSR Absolute */
	case 0x4e:
		ABS; MEM; LSR; WMEM; CLOCK(6); break;
		/* LSR Absolute, X */
	case 0x5e:
		ABS_X; MEM; LSR; WMEM; CLOCK(7); break;
		/* NOP */
	case 0xea:
		CLOCK(2); break;
		/* ORA Immediate */
	case 0x09:
		IMM; ORA; WACC; CLOCK(2); break;
		/* ORA ZeroPage */
	case 0x05:
		ZP; MEM; ORA; WACC; CLOCK(3); break;
		/* ORA ZeroPage, X */
	case 0x15:
		ZP_X; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute */
	case 0x0d:
		ABS; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute, X */
	case 0x1d:
		ABS_X; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute, Y */
	case 0x19:
		ABS_Y; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA (Indirect, X) */
	case 0x01:
		IND_X; MEM; ORA; WACC; CLOCK(6); break;
		/* ORA (Indirect), Y */
	case 0x11:
		IND_Y; MEM; ORA; WACC; CLOCK(5); break;
		/* PHA */
	case 0x48:
		PUSH(this->m_regA); CLOCK(3); break;
		/* PHP */
	case 0x08:
		PUSH(this->m_regF); CLOCK(3); break;
		/* PLA */
	case 0x68:
		POPR; WACC; CLOCK(4); break;
		/* PLP */
	case 0x28:
		POPF; CLOCK(4); break;
		/* ROL Accumulator */
	case 0x2a:
		ACC; ROL; WACC; CLOCK(2); break;
		/* ROL ZeroPage */
	case 0x26:
		ZP; MEM; ROL; WMEM; CLOCK(5); break;
		/* ROL ZeroPage, X */
	case 0x36:
		ZP_X; MEM; ROL; WMEM; CLOCK(6); break;
		/* ROL Absolute */
	case 0x2e:
		ABS; MEM; ROL; WMEM; CLOCK(6); break;
		/* ROL Absolute, X */
	case 0x3e:
		ABS_X; MEM; ROL; WMEM; CLOCK(7); break;
		/* ROR Accumulator */
	case 0x6a:
		ACC; ROR; WACC; CLOCK(2); break;
		/* ROR ZeroPage */
	case 0x66:
		ZP; MEM; ROR; WMEM; CLOCK(5); break;
		/* ROR ZeroPage, X */
	case 0x76:
		ZP_X; MEM; ROR; WMEM; CLOCK(6); break;
		/* ROR Absolute */
	case 0x6e:
		ABS; MEM; ROR; WMEM; CLOCK(6); break;
		/* ROR Absolute, X */
	case 0x7e:
		ABS_X; MEM; ROR; WMEM; CLOCK(7); break;
		/* RTI */
	case 0x40:
		RTI; CHECK_IRQ; CLOCK(6); break;
		/* RTS */
	case 0x60:
		RTS; CLOCK(6); break;
		/* SBC Immediate */
	case 0xe9:
		IMM; SBC; WACC; CLOCK(2); break;
		/* SBC ZeroPage */
	case 0xe5:
		ZP; MEM; SBC; WACC; CLOCK(3); break;
		/* SBC ZeroPage, X */
	case 0xf5:
		ZP_X; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute */
	case 0xed:
		ABS; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute, X */
	case 0xfd:
		ABS_X; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute, Y */
	case 0xf9:
		ABS_Y; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC (Indirect, X) */
	case 0xe1:
		IND_X; MEM; SBC; WACC; CLOCK(6); break;
		/* SBC (Indirect), Y */
	case 0xf1:
		IND_Y; MEM; SBC; WACC; CLOCK(5); break;
		/* SEC */
	case 0x38:
		SET_FLAG(C_Flag); CLOCK(2); break;
		/* SED */
	case 0xf8:
		SET_FLAG(D_Flag); CLOCK(2); break;
		/* SEI */
	case 0x78:
		SET_FLAG(I_Flag); CLOCK(2); break;
		/* STA ZeroPage */
	case 0x85:
		ZP; STA; CLOCK(3); break;
		/* STA ZeroPage, X */
	case 0x95:
		ZP_X; STA; CLOCK(4); break;
		/* STA Absolute */
	case 0x8d:
		ABS; STA; CLOCK(4); break;
		/* STA Absolute, X */
	case 0x9d:
		ABS_X; STA; CLOCK(5); break;
		/* STA Absolute, Y */
	case 0x99:
		ABS_Y; STA; CLOCK(5); break;
		/* STA (Indirect, X) */
	case 0x81:
		IND_X; STA; CLOCK(6); break;
		/* STA (Indirect), Y */
	case 0x91:
		IND_Y; STA; CLOCK(6); break;
		/* STX ZeroPage */
	case 0x86:
		ZP; STX; CLOCK(3); break;
		/* STX ZeroPage, Y */
	case 0x96:
		ZP_Y; STX; CLOCK(4); break;
		/* STX Absolute */
	case 0x8e:
		ABS; STX; CLOCK(4); break;
		/* STY ZeroPage */
	case 0x84:
		ZP; STY; CLOCK(3); break;
		/* STY ZeroPage, X */
	case 0x94:
		ZP_X; STY; CLOCK(4); break;
		/* STY Absolute */
	case 0x8c:
		ABS; STY; CLOCK(4); break;
		/* TAX */
	case 0xaa:
		TAX; CLOCK(2); break;
		/* TAY */
	case 0xa8:
		TAY; CLOCK(2); break;
		/* TSX */
	case 0xba:
		TSX; CLOCK(2); break;
		/* TXA */
	case 0x8a:
		TXA; CLOCK(2); break;
		/* TXS */
	case 0x9a:
		TXS; CLOCK(2); break;
		/* TYA */
	case 0x98:
		TYA; CLOCK(2); break;


	default:
		CLOCK(2); break;
	}

	return clock;
}


void C6502::Serialize(CArchive &ar)
{
	C65c02::Serialize(ar);
}
