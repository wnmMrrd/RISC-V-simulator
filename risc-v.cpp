#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<iostream>
using namespace std;
typedef unsigned int uint;
const int N=0x20000;
int getval(char ch)
{
	return (int)(ch>='0'&&ch<='9'?ch-'0':ch-'A'+10);
}
int getnum(uint x,int l,int r)
{
	return (int)((x>>l)&((1<<(r-l+1))-1));
}
bool end_flag,ID_lock,EX_lock,MEM_lock,WB_lock;
int RAM[N];
int pc;
int xreg[32],used[32];
bool check(int x)
{
	return !x||!used[x];
}
int BHT[N];
void change(int Pc,int f)
{
	if(f)
	{
		if(BHT[Pc]==0) BHT[Pc]=1;
		else BHT[Pc]=3;
	}
	else{
		if(BHT[Pc]==3) BHT[Pc]=2;
		else BHT[Pc]=0;
	}
}
struct Register
{
	uint op;
	bool pre;
	int opcode,func3,func7,rs1,rs2,rd,imm;
	int pc0,A,B,ALUoutput;
	void clear()
	{
		op=0;
		pre=false;
		opcode=func3=func7=rs1=rs2=rd=imm=0;
		pc0=A=B=ALUoutput=0;
	}
	void push(uint x)
	{
		op=(op<<8)+x;
	}
	void getopcode()
	{
		opcode=getnum(op,0,6);
	}
	void getfunc3()
	{
		func3=getnum(op,12,14);
	}
	void getfunc7()
	{
		func7=getnum(op,25,31);
	}
	void getrs1()
	{
		rs1=getnum(op,15,19);
		A=xreg[rs1];
	}
	void getrs2()
	{
		rs2=getnum(op,20,24);
		B=xreg[rs2];
	}
	void getrd()
	{
		rd=getnum(op,7,11);
	}
	void getimm_I()
	{
		imm=getnum(op,20,31);
		if((imm&1<<11)!=0) imm|=(((1<<20)-1)<<12);
	}
	void getimm_S()
	{
		imm=getnum(op,7,11)+(getnum(op,25,31)<<5);
		if((imm&1<<11)!=0) imm|=(((1<<20)-1)<<12);
	}
	void getimm_B()
	{
		imm=(getnum(op,8,11)<<1)+(getnum(op,25,30)<<5)+(getnum(op,7,7)<<11)+(getnum(op,31,31)<<12);
		if((imm&1<<12)!=0) imm|=(((1<<19)-1)<<13);
	}
	void getimm_U()
	{
		imm=(getnum(op,12,31)<<12);
	}
	void getimm_J()
	{
		imm=(getnum(op,21,30)<<1)+(getnum(op,20,20)<<11)+(getnum(op,12,19)<<12)+(getnum(op,31,31)<<20);
		if((imm&1<<20)!=0) imm|=(((1<<11)-1)<<21);
	}
	void ADD()
	{
		ALUoutput=A+B;
	}
	void SUB()
	{
		ALUoutput=A-B;
	}
	void SLL()
	{
		ALUoutput=(A<<(B&31));
	}
	void SLT()
	{
		ALUoutput=(A<B);
	}
	void SLTU()
	{
		ALUoutput=((uint)A<(uint)B);
	}
	void XOR()
	{
		ALUoutput=(A^B);
	}
	void SRL()
	{
		ALUoutput=(((uint)A)>>(B&31));
	}
	void SRA()
	{
		int k=(B&31);
		ALUoutput=(((uint)A)>>k);
		if((A&(1<<31))!=0) ALUoutput|=(((1<<k)-1)<<(31-k+1));
	}
	void OR()
	{
		ALUoutput=(A|B);
	}
	void AND()
	{
		ALUoutput=(A&B);
	}
	void ADDI()
	{
		ALUoutput=A+imm;
	}
	void SLTI()
	{
		ALUoutput=(A<imm);
	}
	void SLTIU()
	{
		ALUoutput=((uint)A<(uint)imm);
	}
	void XORI()
	{
		ALUoutput=(A^imm);
	}
	void ORI()
	{
		ALUoutput=(A|imm);
	}
	void ANDI()
	{
		ALUoutput=(A&imm);
	}
	void SLLI()
	{
		ALUoutput=A<<imm;
	}
	void SRLI()
	{
		ALUoutput=((uint)A)>>imm;
	}
	void SRAI()
	{
		int k=(imm&31);
		ALUoutput=(((uint)A)>>k);
		if((A&(1<<31))!=0) ALUoutput|=(((1<<k)-1)<<(31-k+1));
	}
	void BEQ()
	{
		if(A==B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void BNE()
	{
		if(A!=B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void BLT()
	{
		if(A<B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void BGE()
	{
		if(A>=B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void BLTU()
	{
		if((uint)A<(uint)B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void BGEU()
	{
		if((uint)A>=(uint)B)
		{
			if(!pre) pc=pc0+imm,ID_lock=true;
			change(pc0,1);
		}
		else{
			if(pre) pc=pc0+4,ID_lock=true;
			change(pc0,0);
		}
	}
	void LB()
	{
		int addr=ALUoutput;
		ALUoutput=RAM[addr];
		if(((uint)RAM[addr])>>7) ALUoutput|=(((1<<24)-1)<<8);
	}
	void LH()
	{
		int addr=ALUoutput;
		ALUoutput=(RAM[addr+1]<<8)+RAM[addr];
		if(((uint)RAM[addr+1])>>7) ALUoutput|=(((1<<16)-1)<<16);
	}
	void LW()
	{
		int addr=ALUoutput;
		ALUoutput=(RAM[addr+3]<<24)+(RAM[addr+2]<<16)+(RAM[addr+1]<<8)+RAM[addr];
	}
	void LBU()
	{
		int addr=ALUoutput;
		ALUoutput=RAM[addr];
	}
	void LHU()
	{
		int addr=ALUoutput;
		ALUoutput=(RAM[addr+1]<<8)+RAM[addr];
	}
	void SB()
	{
		RAM[ALUoutput]=(B&255);
	}
	void SH()
	{
		RAM[ALUoutput]=(B&255);
		RAM[ALUoutput+1]=(B&255);
	}
	void SW()
	{
		RAM[ALUoutput]=(B&255);
		RAM[ALUoutput+1]=((((uint)B)>>8)&255);
		RAM[ALUoutput+2]=((((uint)B)>>16)&255);
		RAM[ALUoutput+3]=((((uint)B)>>24)&255);
	}
}IF_ID,ID_EX,EX_MEM,MEM_WB;
void IF()
{
	if(RAM[pc]==19&&RAM[pc+1]==5&&RAM[pc+2]==240&&RAM[pc+3]==15)
	{
		end_flag=true;
		return;
	}
	IF_ID.clear();
	for(int i=pc+3;i>=pc;i--)
		IF_ID.push((uint)RAM[i]);
	IF_ID.pc0=pc;pc+=4;
	ID_lock=false;
}
void ID()
{
	ID_EX=IF_ID;
	ID_EX.getopcode();
	if(ID_EX.opcode==0b0110011) //R
	{
		ID_EX.getrs1();
		ID_EX.getrs2();
		ID_EX.getrd();
		ID_EX.getfunc3();
		ID_EX.getfunc7();
		
		if(!check(ID_EX.rs1)||!check(ID_EX.rs2)) return;
		ID_lock=true; EX_lock=false;
		if(ID_EX.rd) used[ID_EX.rd]++;
	}
	else if(ID_EX.opcode==0b1100111||ID_EX.opcode==0b0000011||ID_EX.opcode==0b0010011) //I
	{
		ID_EX.getrs1();
		ID_EX.getrd();
		ID_EX.getfunc3();
		ID_EX.getimm_I();
		
		if(!check(ID_EX.rs1)) return;
		ID_lock=true; EX_lock=false;
		if(ID_EX.rd) used[ID_EX.rd]++;
		
		//if(FLAG) cout<<xreg[ID_EX.rs1]<<endl;
		if(ID_EX.opcode==0b1100111) pc=((ID_EX.A+ID_EX.imm)&(-1));
	}
	else if(ID_EX.opcode==0b0100011) //S
	{
		ID_EX.getrs1();
		ID_EX.getrs2();
		ID_EX.getimm_S();
		ID_EX.getfunc3();
		
		if(!check(ID_EX.rs1)||!check(ID_EX.rs2)) return;
		ID_lock=true; EX_lock=false;
	}
	else if(ID_EX.opcode==0b1100011) //B
	{
		ID_EX.getrs1();
		ID_EX.getrs2();
		ID_EX.getimm_B();
		ID_EX.getfunc3();
		
		if(!check(ID_EX.rs1)||!check(ID_EX.rs2)) return;
		ID_lock=true; EX_lock=false;
		
		ID_EX.pre=(BHT[ID_EX.pc0]>>1);
		if(ID_EX.pre) pc=ID_EX.pc0+ID_EX.imm;
	}
	else if(ID_EX.opcode==0b0110111||ID_EX.opcode==0b0010111) //U
	{
		ID_EX.getrd();
		ID_EX.getimm_U();
		
		ID_lock=true; EX_lock=false;
		if(ID_EX.rd) used[ID_EX.rd]++;
	}
	else if(ID_EX.opcode==0b1101111) //J
	{
		ID_EX.getrd();
		ID_EX.getimm_J();
		
		ID_lock=true; EX_lock=false;
		if(ID_EX.rd) used[ID_EX.rd]++;
		
		pc=ID_EX.pc0+ID_EX.imm;
	}
}
int clk;
void EX()
{
	EX_MEM=ID_EX;
	EX_lock=true; MEM_lock=false;
	clk=0;
	if(EX_MEM.opcode==0b0110011)
	{
		int f1=EX_MEM.func3,f2=EX_MEM.func7;
		if(!f1)
		{
			if(!f2) EX_MEM.ADD();
			else EX_MEM.SUB();
		}
		else if(f1==1) EX_MEM.SLL();
		else if(f1==2) EX_MEM.SLT();
		else if(f1==3) EX_MEM.SLTU();
		else if(f1==4) EX_MEM.XOR();
		else if(f1==5)
		{
			if(!f2) EX_MEM.SRL();
			else EX_MEM.SRA();
		}
		else if(f1==6) EX_MEM.OR();
		else if(f1==7) EX_MEM.AND();
	}
	else if(EX_MEM.opcode==0b1100111||EX_MEM.opcode==0b1101111)
		EX_MEM.ALUoutput=EX_MEM.pc0+4;
	else if(EX_MEM.opcode==0b0000011||EX_MEM.opcode==0b0100011)
		EX_MEM.ALUoutput=EX_MEM.A+EX_MEM.imm;
	else if(EX_MEM.opcode==0b0010011)
	{
		int f=EX_MEM.func3;
		if(!f) EX_MEM.ADDI();
		else if(f==2) EX_MEM.SLTI();
		else if(f==3) EX_MEM.SLTIU();
		else if(f==4) EX_MEM.XORI();
		else if(f==6) EX_MEM.ORI();
		else if(f==7) EX_MEM.ANDI();
		else if(f==1) EX_MEM.SLLI();
		else if(f==5&&!(((uint)EX_MEM.imm)>>5)) EX_MEM.SRLI();
		else if(f==5&&(((uint)EX_MEM.imm)>>5)) EX_MEM.SRAI();
	}
	else if(EX_MEM.opcode==0b1100011)
	{
		int f=EX_MEM.func3;
		if(!f) EX_MEM.BEQ();
		else if(f==1) EX_MEM.BNE();
		else if(f==4) EX_MEM.BLT();
		else if(f==5) EX_MEM.BGE();
		else if(f==6) EX_MEM.BLTU();
		else if(f==7) EX_MEM.BGEU();
	}
	else if(EX_MEM.opcode==0b0110111)
		EX_MEM.ALUoutput=EX_MEM.imm;
	else if(ID_EX.opcode==0b0010111)
		EX_MEM.ALUoutput=EX_MEM.pc0+EX_MEM.imm;
}
void MEM()
{
	MEM_WB=EX_MEM;
	if(MEM_WB.opcode==0b0000011||MEM_WB.opcode==0b0100011)
	{
		clk++;
		if(clk>=3)
		{
			int f=MEM_WB.func3;
			if(MEM_WB.opcode==0b0000011)
			{
				if(!f) MEM_WB.LB();
				else if(f==1) MEM_WB.LH();
				else if(f==2) MEM_WB.LW();
				else if(f==4) MEM_WB.LBU();
				else if(f==5) MEM_WB.LHU();
			}
			else{
				if(!f) MEM_WB.SB();
				else if(f==1) MEM_WB.SH();
				else if(f==2) MEM_WB.SW();
			}
			MEM_lock=true; WB_lock=false;
		}
	}
	else MEM_lock=true,WB_lock=false;
}
void WB()
{
	WB_lock=1;
	if(MEM_WB.rd!=0)
	{
		xreg[MEM_WB.rd]=MEM_WB.ALUoutput;
		used[MEM_WB.rd]--;
	}
}
int main()
{
	char str[10];
	int pos;
	while(scanf("%s",str)!=EOF)
	{
		if(str[0]=='@')
		{
			pos=0;
			for(int i=1;i<=8;i++)
				pos=(pos<<4)+getval(str[i]);
		}
		else RAM[pos++]=(getval(str[0])<<4)+getval(str[1]);
	}
	IF_ID.clear();
	ID_EX.clear();
	EX_MEM.clear();
	MEM_WB.clear();
	ID_lock=EX_lock=MEM_lock=WB_lock=true;
	while(!end_flag||!ID_lock||!EX_lock||!MEM_lock||!WB_lock)
	{
		if(!WB_lock) WB();
		if(WB_lock&&!MEM_lock) MEM();
		if(MEM_lock&&!EX_lock) EX();
		if(EX_lock&&!ID_lock) ID();
		if(ID_lock) IF();
	}
	printf("%u\n",(((uint)(xreg[10]))&255u));
	return 0;
}
