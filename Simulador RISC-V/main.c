#include <stdio.h>
#include <stdlib.h>

// RISC-V 32 registradores de uso geral e o PC
// Uso Geral = 0 - 31, PC - 32
unsigned int registradores[33];

// Tipos de Decodificação
#define R_type 0
#define I_type 1
#define S_type 2
#define B_type 3
#define U_type 4
#define J_type 5

// Definição de OPCodes
#define OP_IMM (unsigned int)0b0010011 //Operações com Imediato

// Definições de Função
#define ADDI 0b000
#define SLTI 0b010
#define SLTIU 0b011
#define ANDI 0b111
#define ORI 0b110
#define XORI 0b100
#define SLLI 0b001
#define SRLI 0b101

// Seta o valor para os Registradores, mantendo o Registrador r0 como zero
void setValueReg(unsigned int value, int indice){
    if(indice > 0 && indice < 32){
        registradores[indice] = value;
    }
}

// Realiza uma extensão dos bits de um número para o temanho de 32 bits
void bitExtend(unsigned int* valor){
    unsigned int aux = *valor;
    aux = aux << 20;
    *valor = (int)aux >> 20;
}

// Decodifica as instruções de acordo com os tipos referentes no Manual da Arquitetura
unsigned int* decodificaInstrucao(unsigned int instrucao_Raw, int tipo){
    unsigned int* resposta;
    if(tipo == I_type){
        resposta = (unsigned int *) malloc (sizeof(unsigned int) * 5); //Aloca o espaço para a separação dos valores
        resposta[0] = instrucao_Raw & 0b1111111; //Primeiros 7bits são para o OPCODE
        resposta[1] = (instrucao_Raw >> 7) & 0b11111; //5bits para o Registrador de Destino
        resposta[2] = (instrucao_Raw >> 12) & 0b111; //3bits para a funct3
        resposta[3] = (instrucao_Raw >> 15) & 0b11111; //5bits para o Registrador de Origem
        resposta[4] = instrucao_Raw >> 20; //12bits para o Imediato
    }

    return resposta;
}

// Conjunto de Instruções referentes ao OPCODE OP-IMM
void operacoesImediato(unsigned int* instrucao){

    bitExtend(instrucao[4]); //Faz um bit Extend para 32 bits

    if(!(instrucao[2] ^ ADDI)){ //Soma com Imediato
        setValueReg(registradores[instrucao[3]] + (int)instrucao[4], instrucao[1]); //ADDI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ SLTI)){ //Set Less Than Immediate (Seta o registrador rx como 1 se o ry < IMM)
        setValueReg(registradores[instrucao[3]] < (int)instrucao[4] ? 1 : 0, instrucao[1]); //SLTI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ SLTIU)){ //Set Less Than Immediate Unsigned
        setValueReg(registradores[instrucao[3]] < instrucao[4] ? 1 : 0, instrucao[1]); //SLTIU rx, ry, IMM
    }
    else if(!(instrucao[2] ^ ANDI)){ //AND Immediate
        setValueReg(registradores[instrucao[3]] & instrucao[4], instrucao[1]); //ANDI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ ORI)){ //OR Immediate
        setValueReg(registradores[instrucao[3]] | instrucao[4], instrucao[1]); //ORI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ XORI)){ //XOR Immediate
        setValueReg(registradores[instrucao[3]] ^ instrucao[4], instrucao[1]); //XORI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ SLLI)){ //Shift Left Logical Immediate
        int valor = 0b11111 & instrucao[4]; //Especialização do I-type, na qual os 5 primeiros bits são referentes à quantidade do shift
        setValueReg(registradores[instrucao[3]] << valor, instrucao[1]); //SLLI rx, ry, IMM
    }
    else if(!(instrucao[2] ^ SRLI)){ //Shift Right Logical/Arithmetic Immediate
        int valor = 0b11111 & instrucao[4]; //Especialização do I-type, na qual os 5 primeiros bits são referentes à quantidade do shift
        int logical_arit = (instrucao[4] >> 11) & 1; //Tipo de Shift é definido no bit 20 do I-type

        setValueReg(logical_arit ? registradores[instrucao[3]] >> valor : (int)registradores[instrucao[3]] >> valor, instrucao[1]); //SRLI rx, ry, IMM - SRAI rx, ry, IMM
    }
}

void decodificaOPCODE(unsigned int* instrucao){
    if(!(instrucao[0] ^ OP_IMM)){
        operacoesImediato(instrucao);
    }
}


int main(int argc, char const *argv[])
{
    unsigned int teste = 4094;
    printf("%08x\n", teste);
    bitExtend(&teste);
    printf("%08x\n", teste);
}
