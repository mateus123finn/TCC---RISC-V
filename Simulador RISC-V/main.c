#include <stdio.h>
#include <stdlib.h>

unsigned int* memoria_Instrucoes; //Armazenará os endereços de memória para as instruções
unsigned int TAM_Memoria_Instrucoes;

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
#define LUI (unsigned int)0b0110111 //Load Upper Imediate
#define AUIPC (unsigned int)0b0010111 //Add Uper Immediate to PC
#define OP (unsigned int)0b0110011 //Operações de Registradores com Registradores
#define JAL (unsigned int)0b1101111 //Jump and Link (PC atual + offset)
#define JALR (unsigned int)0b1100111 //Jump and Link Register (Rx + offset)

// Definições de Função

// OP-IMM
#define ADDI 0b000
#define SLTI 0b010
#define SLTIU 0b011
#define ANDI 0b111
#define ORI 0b110
#define XORI 0b100
#define SLLI 0b001
#define SRLI 0b101

// OP
#define ADD 0b000
#define SLT 0b010
#define SLTU 0b011
#define AND 0b111
#define OR 0b110
#define XOR 0b100
#define SLL 0b001
#define SRL 0b101

// Printa uma tabela com todos os Registradores
void dumpRegs(){
    printf("Rzero - %08X\n",registradores[0]);
    for(int i = 1; i < 32; i++){
        printf("R%i - %08X\n",i,registradores[i]);
    }
    printf("PC - %08X\n",registradores[32]);

    printf("\n");
}

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
    else if(tipo == U_type){
        resposta = (unsigned int *) malloc (sizeof(unsigned int) * 3); //Aloca o espaço para a separação dos valores
        resposta[0] = instrucao_Raw & 0b1111111; //Primeiros 7bits são para o OPCODE
        resposta[1] = (instrucao_Raw >> 7) & 0b11111; //5bits para o Registrador de Destino
        resposta[2] = (instrucao_Raw >> 12); //20bits para o Immediato
    }
    else if(tipo == R_type){
        resposta = (unsigned int *) malloc (sizeof(unsigned int) * 6); //Aloca o espaço para a separação dos valores
        resposta[0] = instrucao_Raw & 0b1111111; //Primeiros 7bits são para o OPCODE
        resposta[1] = (instrucao_Raw >> 7) & 0b11111; //5bits para o Registrador de Destino
        resposta[2] = (instrucao_Raw >> 12) & 0b111; //3bits para a funct3
        resposta[3] = (instrucao_Raw >> 15) & 0b11111; //5bits para o Registrador de Origem1
        resposta[4] = (instrucao_Raw >> 20) & 0b11111; //5bits para o Registrador de Origem2
        resposta[5] = (instrucao_Raw >> 25) & 0b1111111; //7bits para a funct7
    }
    else if(tipo == J_type){
        resposta = (unsigned int *) malloc (sizeof(unsigned int) * 3); //Aloca o espaço para a separação dos valores
        resposta[0] = instrucao_Raw & 0b1111111; //Primeiros 7bits são para o OPCODE
        resposta[1] = (instrucao_Raw >> 7) & 0b11111; //5bits para o Registrador de Destino
        unsigned int aux = (instrucao_Raw >> 12); //20bits para o Immediato (Modelo J - 20/10:1/11/19:12)
        //Arrumar a ordem dos bits
        resposta[2] = (aux >> 9) & 0b1111111111;
        resposta[2] = resposta[2] | (((aux >> 8) & 1) << 11);
        resposta[2] = resposta[2] | ((aux & 0b11111111) << 12);
        resposta[2] = resposta[2] | (aux & 0b10000000000000000000);
    }
    else if(tipo == B_type){ //TODO
        resposta = (unsigned int *) malloc (sizeof(unsigned int) * 6); //Aloca o espaço para a separação dos valores
        resposta[0] = instrucao_Raw & 0b1111111; //Primeiros 7bits são para o OPCODE
        resposta[1] = (instrucao_Raw >> 7) & 0b11111; //5bits para o Registrador de Destino
        resposta[2] = (instrucao_Raw >> 12) & 0b111; //3bits para a funct3
        resposta[3] = (instrucao_Raw >> 15) & 0b11111; //5bits para o Registrador de Origem1
        resposta[4] = (instrucao_Raw >> 20) & 0b11111; //5bits para o Registrador de Origem2
        resposta[5] = (instrucao_Raw >> 25) & 0b1111111; //7bits para a funct7
    }

    return resposta;
}

// Conjunto de Instruções referentes ao OPCODE OP-IMM
void operacoesImediato(unsigned int* instrucao){

    bitExtend(&instrucao[4]); //Faz um bit Extend para 32 bits

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

        setValueReg(!logical_arit ? registradores[instrucao[3]] >> valor : (int)registradores[instrucao[3]] >> valor, instrucao[1]); //SRLI rx, ry, IMM - SRAI rx, ry, IMM
    }
}

void operacoesRegtoReg(unsigned int* instrucao){
    if(!(instrucao[2] ^ ADD)){
        int add_sub = (instrucao[5] >> 5) & 1; //Verifica se a operação é uma Soma ou uma Subtração de acordo com funct7
        setValueReg(!add_sub ? registradores[instrucao[4]] + registradores[instrucao[3]] : (~registradores[instrucao[4]] + 1) + registradores[instrucao[3]], instrucao[1]);
    }
    else if(!(instrucao[2] ^ SLT)){
        setValueReg(registradores[(int)instrucao[3]] < (int)registradores[instrucao[4]] ? 1 : 0, instrucao[1]);
    }
    else if(!(instrucao[2] ^ SLTU)){
        setValueReg(registradores[instrucao[3]] < registradores[instrucao[4]] ? 1 : 0, instrucao[1]);
    }
    else if(!(instrucao[2] ^ AND)){
        setValueReg(registradores[instrucao[3]] & registradores[instrucao[4]], instrucao[1]);
    }
    else if(!(instrucao[2] ^ OR)){
        setValueReg(registradores[instrucao[3]] | registradores[instrucao[4]], instrucao[1]);
    }
    else if(!(instrucao[2] ^ XOR)){
        setValueReg(registradores[instrucao[3]] ^ registradores[instrucao[4]], instrucao[1]);
    }
    else if(!(instrucao[2] ^ SLL)){
        setValueReg(registradores[instrucao[3]] << registradores[instrucao[4]], instrucao[1]);
    }
    else if(!(instrucao[2] ^ SRL)){
        int logical_arit = (instrucao[5] >> 5) & 1;
        int valor = registradores[instrucao[4]] & 0x11111;
        setValueReg(!logical_arit ? registradores[instrucao[3]] >> valor : (int)registradores[instrucao[3]] >> valor, instrucao[1]);
    }

}

// Carrega um valor para os 20 bits mais a esquerda de um Registrador e soma com o PC
void addUpperImmediatePC(unsigned int* instrucao){
    setValueReg((instrucao[2] << 12) + registradores[32],instrucao[1]);
}

// Carrega um Valor para os os 20 bits mais a esuqerda de um Registrador (Upper)
void loadUpperImmediate(unsigned int* instrucao){ 
    setValueReg(instrucao[2] << 12,instrucao[1]);
}

// Realiza um salto na memória de intruções e carrega um novo valor, salvando o valor anterior + 4 (PC + 4)
void jumpAndLink(unsigned int* instrucao){
    setValueReg(registradores[32],instrucao[1]); //Salva o valor de PC + 4 no Registrador especidficado
    registradores[32] = (registradores[32] - 4) + ((int)instrucao[2]*2); //Base+Offset - Base+(v*2)
}

// Realiza um salto na memória de intruções e carrega um novo valor, utilizando um Registrador como Base
void jumpAndLinkRegister(unsigned int* instrucao){
    setValueReg(registradores[32],instrucao[1]); //Salva o valor de PC + 4 no Registrador especidficado
    bitExtend(&instrucao[4]); //Extende o Valor para 32 bits
    instrucao[4] = instrucao[4] & (~(unsigned int)0b1); //Seta o LSB como 0
    registradores[32] = registradores[instrucao[3]] + (int)instrucao[4]; //Base+Offset - Rx+(v & ~1)
}

// Verifica o OPCODE da Intrução e chama a função referente
void decodificaOPCODE(unsigned int instrucao){
    unsigned int* instrucao_decoded;
    if(!((instrucao & 0b1111111) ^ OP_IMM)){ //Operações com Immediato
        instrucao_decoded = decodificaInstrucao(instrucao,I_type);
        operacoesImediato(instrucao_decoded);
    } 
    else if(!((instrucao & 0b1111111) ^ LUI)){ //Load Upper Immediate
        instrucao_decoded = decodificaInstrucao(instrucao,U_type);
        loadUpperImmediate(instrucao_decoded);
    }
    else if(!((instrucao & 0b1111111) ^ AUIPC)){ //ADD Upper Immediate to PC
        instrucao_decoded = decodificaInstrucao(instrucao,U_type);
        addUpperImmediatePC(instrucao_decoded);
    }
    else if(!((instrucao & 0b1111111) ^ OP)){ //Operações de Registrador para Registrador
        instrucao_decoded = decodificaInstrucao(instrucao,R_type);
        addUpperImmediatePC(instrucao_decoded);
    }
    else if (!((instrucao & 0b1111111) ^ JAL)){
        instrucao_decoded = decodificaInstrucao(instrucao,J_type);
        jumpAndLink(instrucao_decoded);
    }
    else if (!((instrucao & 0b1111111) ^ JALR)){
        instrucao_decoded = decodificaInstrucao(instrucao,I_type);
        jumpAndLinkRegister(instrucao_decoded);
    }
    
    free(instrucao_decoded);
}

// Lê os valores do arquivo e empilha a chamada da função, ao ler todos, aloca o espaço e desempilha na memória de instruções
void carregaInstrucoes(int indice, FILE* arq){
    if(feof(arq)){ //Chegou no Final do Arquivo
        memoria_Instrucoes = (unsigned int *)malloc(sizeof(unsigned int) * indice-1); //Aloca o espaço necessário para as instruções
        TAM_Memoria_Instrucoes = indice-1; //Contador de quantas instruções foram lidas
        fclose(arq); //Fecha o arquivo, já que não precisa mais de ler dele
        return;
    }
    //Enquanto não está no fim do arquivo
    unsigned int instrucao = 0; //Instrução Lida do Arquivo
    fread(&instrucao,sizeof(unsigned int),1,arq); //Leitura dos 4Bytes de Instrução
    carregaInstrucoes(indice + 1,arq); //Empilha a Chamada de função para carregar todas as instruções
    memoria_Instrucoes[indice] = instrucao; //Ao carregar todas as instruções e alocar o espaço na memória, seta os valores
}

// Lê uma intrução a partir do endereco do PC - REG[32]
unsigned int fetchInstrucao(){
    int indice = registradores[32] / 4; //0x00 = 0 \ 0x04 = 1 \ ...
    return indice < TAM_Memoria_Instrucoes ? memoria_Instrucoes[indice] : 0; //Endereco Inválido ou Fim da Execução
}

// Printa os valores carregados na memória de Intruções
void dumpMEMInst(){
    for(int i = 0; i < TAM_Memoria_Instrucoes; i++){
        printf("%X - %08X\n",i * 4,memoria_Instrucoes[i]);
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{

    if(argc < 2){
        printf("Uso Incorreto!!!\n%s <Nome do Arquivo>\n",argv[0]);
        return 2;
    }

    FILE* arq_raw = fopen(argv[1],"rb");

    if(arq_raw == NULL){
        printf("Arquivo não encontrado!!!\n");
        return 3;
    }

    carregaInstrucoes(0,arq_raw);
    dumpMEMInst();

    unsigned int instrucao = fetchInstrucao(); //Primeira Intrucao
    dumpRegs();
    while (instrucao != 0){
        registradores[32] += 4;
        decodificaOPCODE(instrucao);
        dumpRegs();
        instrucao = fetchInstrucao();
    }
    

    // 00000000 -> 00000F0F
    //decodificaOPCODE(0x00F00093); //ADDI R1, Rzero, 0x00F
    //decodificaOPCODE(0x00809113); //SLLI R2, R1, 8
    //decodificaOPCODE(0x00F16193); //ORI R3, R2, 0x00F

    //decodificaOPCODE(0x0FF00293); //ADDI R5, R0, 255
    //decodificaOPCODE(0xFF600293); //ADDI R5, R0, -10
    //decodificaOPCODE(0x00200293); //ADDI R5, R0, 2
    //decodificaOPCODE(0x00128313); //ADDI R6, R5, 1
    //decodificaOPCODE(0x0012A313); //SLTI R6, R5, 1 (R6 = R5 < 1)

    //Fim da Execução
    free(memoria_Instrucoes); //Desaloca da memória as Instruções
}
