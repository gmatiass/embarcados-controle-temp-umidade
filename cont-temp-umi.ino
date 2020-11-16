/*******************************************\
 **      PROJETO SISTEMAS EMBARCADOS       **
 **         AUFSCHNITT ZU HAUSE            **
 **     Gabriel Matias e Norton Lima       **
\*******************************************/
//================================================================================================================
// --- Bibliotecas --- 
#include <LiquidCrystal.h>                //Biblioteca do LCD
#include <dht.h>                          //Biblioteca para o sensor DHT11

//================================================================================================================
// --- Mapeamento do Hardware ---
LiquidCrystal lcd(11, 12, 13, 10, 9, 8);  //Mapeamento do hardware do LCD
#define botaoSel 6                        //Botao de selecao no pino digital 6
#define botaoConf 5                       //Botao de confirma no pino digital 5
#define dht_pin 4                         //Sensor DHT11 no pino digital 4
#define coolerHot 2                       //Rele 1 acionando o cooler 1 a partir da saida digital 2
#define coolerCold 3                      //Rele 2 acionando o cooler 2 a partir da saida digital 3
#define peltHot 0                         //Rele 3 acionando o peltier de aquecimento na saida digital 0
#define peltCold 1                        //Rele 4 acionando o peltier de esfriamento na saida digital 1
#define umidificador 7                    //Umidificador no pino digital 7

//================================================================================================================
// --- Constantes e Objetos ---

#define menu_max 4                        //Constante de tamanho do menu
#define time_1s 62                        //Constante para contagem de 1 segundo
#define time_30s 30                       //Constante para contagem de 30 segundos

dht sensorDHT;                            //Objeto para o sensor

//================================================================================================================
// --- Protótipo das Funções --- 
void buttons();                           //Função de uso dos botões
void menu_list();                         //Função de rotatividade na lista de menu
void menu_select();                       //Função de seleção do menu desejado
void temperatura();                       //Função para definir a temperatura desejada
void umidade();                           //Função para definir a umidade desejada
void predef();                            //Funcao para escolher as pre definicoes
void atuadoresOn();                       //Funcao para acionar e controlar os atuadores
void atuadoresOff();                      //Funcao para desligar os atuadores quando voltar ao menu
void leSensor();                          //Funcao para leitura do sensor DHT11 atraves da biblioteca "dht.h"

//================================================================================================================
// --- Flags ---
boolean flag_botaoSel = 0,              //Flag para debounce do botao de selecao          
        flag_botaoConf = 0,             //Flag para debounce do botao de confirma
        flag_menu = 0,                  //Flag para alternar menu de selecao e funcionalidades
        flag_temp = 0,                  //Flag para controle de funcao dos botoes
        flag_umi = 0,                   //Flag para controle de funcao dos botoes
        flag_exibir = 0,                //Flag para controle do lcd e atuadores
        flag_pre = 0,                   //Flag para controle do lcd
        flag_sensor = 0;                //Flag para leitura do sensor apos a interrupcao

//================================================================================================================
// --- Variáveis Globais ---
int line[menu_max] = {0,1,2,3},         //Vetor para gerenciamento da lista de menus
    line_bk[menu_max],                  //Vetor de backup de gerenciamento da lista para rotatividade
    indice,                             //Indice de controle
    menu_number = 1;                    //Seletor do menu

int temp = 25,                          //Variavel de temperatura
    umi  = 75,                          //Variavel de umidade
    tempDesejado = 20,                  //Variavel de mudanca do valor de temperatura desejado
    umiDesejado = 50,                   //Variavel de mudanca do valor de umidade desejado
    tempAtual = 0,                      //Variavel de leitura da temperatura atual
    umiAtual = 0,                       //Variavel de leitura da umidade atual
    pre = 1,                            //Variavel de controle das pre definicoes
    alce1s = 0,                         //Variavel auxiliar para contagem de estouro da interrupcao
    alce30s = 0;                        //Variavel auxiliar para contagem de estouro da interrupcao

//================================================================================================================
// --- Rotina de Interrupção ---
ISR(TIMER2_OVF_vect){
  
  TCNT2 = 0;                            //Reinicializa o registrador do Timer2

  alce1s += 1;                          //Incrementa o contador de 1 segundo pelo estouro do timer2 
  
  if(alce1s == time_1s){                //Condicionamento da contagem maxima da variavel auxiliar de 1 segundo
    alce1s = 0;                         //Reseta a variavel auxiliar de 1 segundo
    alce30s += 1;                       //Incrementa a variavel auxiliar de 30 segundos
  }

  if(alce30s == time_30s){             //Condicionamento da contagem maxima da variavel auxiliar de 30 segundos
    alce30s = 0;                       //Reseta a variavel auxiliar de 30 segundos
    flag_sensor = !flag_sensor;        //Seta a flag responsavel pela execucao da rotina de leitura do sensor a cada 30 segundos
  }

} //end rotina de interrupcao

//================================================================================================================
// --- Configurações Iniciais ---
void setup() {

  pinMode(botaoSel, INPUT_PULLUP);    //Definicao de modo do pino 8
  pinMode(botaoConf, INPUT_PULLUP);   //Definicao de modo do pino 9
  
  pinMode(coolerHot, OUTPUT);         //Definicao de modo do pino de controle do cooler 1
  pinMode(coolerCold, OUTPUT);        //Definicao de modo do pino de controle do cooler 2
  pinMode(peltHot, OUTPUT);           //Definicao de modo do pino de controle do peltier de aquecimento
  pinMode(peltCold, OUTPUT);          //Definicao de modo do pino de controle do peltier de esfriamento
  pinMode(umidificador, OUTPUT);      //Definicao de modo do pino de controle do umidificador

  TCCR2A = 0x00;                      //Timer operando em modo normal
  TCCR2B = 0x07;                      //Prescaler 1:1024
  TCNT2  = 0;                         //Iniciando o contador de 8 bits com 0
  TIMSK2 = 0x01;                      //Habilitando interrupção do Timer2
  
  lcd.begin(16,4);                    //Iniciando display
  
  lcd.setCursor(3,0);                 //Apresentação de inicio
  lcd.print("AUFSCHNITT");
  lcd.setCursor(4,1);
  lcd.print("ZU HAUSE");
  delay(4000);
  lcd.clear();

  
} //end setup

//================================================================================================================
// --- Loop ---
void loop() {
 
  if(!flag_menu){                                                  //if de ihm para exibir as opcoes do menu
    lcd.setCursor(0,0);
    lcd.print(">");
    lcd.setCursor(0,1);
    lcd.print(" ");
    lcd.setCursor(1,line[0]);
    lcd.print("1. Temperatura ");
    lcd.setCursor(1,line[1]);
    lcd.print("2. Umidade     ");
    lcd.setCursor(1,line[2]);
    lcd.print("3. Pre-Definido");
    lcd.setCursor(1,line[3]);
    lcd.print("4. Exibir/Atuar");
    
  }
  
  else if(flag_temp && flag_menu){                                  //if para exibicao do valor desejado
    lcd.setCursor(6,1);
    lcd.print(tempDesejado);
    
  }
  
  else if(flag_umi && flag_menu){                                  //if para exibicao do valor desejado
    lcd.setCursor(6,1);
    if(umiDesejado < 10) lcd.print("0");
    lcd.print(umiDesejado);
    
  }
  
  else if(flag_exibir && flag_menu){                              //if para exibicao de desejado e temp atual
    lcd.setCursor(13,0);                                          //e acionar os atuadores
    if(tempAtual < 10) lcd.print("0");
    lcd.print(tempAtual);
    lcd.setCursor(13,1);
    if(umiAtual < 10) lcd.print("0");
    lcd.print(umiAtual);
    atuadoresOn();
  }
  
  else if(flag_pre && flag_menu){                                 //if para exibicao do menu seção pre definidos
    switch(pre)
      {
        case 1:
          lcd.setCursor(0,1);
          lcd.print("1.Temp:25 Umi:90");
          break;

        case 2:
          lcd.setCursor(0,1);
          lcd.print("2.Temp:30 Umi:85");
          break;
          
        case 3:
          lcd.setCursor(0,1);
          lcd.print("3.Temp:30 Umi:75");
          break;
          
        case 4:
          lcd.setCursor(0,1);
          lcd.print("4.Temp:28 Umi:85");
          break;
      }
  }

  if(!flag_exibir) atuadoresOff();                                //Desligando os atuadores para definicao de novos valores
  if(!flag_sensor) leSensor();                                    //Fazendo a leitura do sensor a partir da interrupcao
  
  buttons();                                                      //Chamando a função de controle dos botões

} //end loop

//================================================================================================================
// --- Desenvolvimento das Funções ---
void buttons(){
  
   if(!digitalRead(botaoSel))   flag_botaoSel   = 0x01;           //Seta flag que o botao seletor foi apertado
   if(!digitalRead(botaoConf))  flag_botaoConf  = 0x01;           //Seta flag que o botao confirma foi apertado

  
  if(digitalRead(botaoSel) && flag_botaoSel)                      //Teste do botão
  {
    flag_botaoSel = 0x00;                                         //Reseta flag do botao seletor

    if(!flag_menu)                                                //Controle da funcao do botao seletor
    {
      menu_list();                                                //Chama a funcao menu lista para a rotatividade do menu
      menu_number += 1;                                           //Indice do menu
      if(menu_number > menu_max)  menu_number = 1;                //Reinicia o indice do menu para manter em apenas 4
    
    }
    
    else if(flag_temp && flag_menu)                               //Controle da funcao do botao seletor
    {
      tempDesejado += 1;                                          //Incrementa o valor da temperatura desejada
      if(tempDesejado > 60) tempDesejado = 20;                    //Limitação do valor da temperatura desejada
    }
    
    else if(flag_umi && flag_menu)                               //Controle da funcao do botao seletor                             
    {
      umiDesejado += 1;                                          //Incrementa o valor da umidade desejada
      if(umiDesejado > 95) umiDesejado = 5;                      //Limitação do valor da umidade desejada
    }

    else if(flag_pre && flag_menu)                              //Controle da funcao do botao seletor 
    {
      pre += 1;                                                 //Incrementa o valor da variavel de controle das pre definicoes
      if(pre > 4) pre = 1;                                      //Limitacao das pre definicoes
    }
      
  } //end if botaoSel

  if(digitalRead(botaoConf) && flag_botaoConf)                 //Controle da funcao do botao confirma
  {
    flag_botaoConf = 0x00;                                     //Reseta flag bota confirma

    if(!flag_menu)  menu_select();                            //Define pra qual função o botao confirma deve levar o programa dependendo
    else if(flag_temp)    temperatura();                      //de qual flag estará setada
    else if(flag_umi)     umidade();
    else if(flag_pre)     predef();
    else if(flag_exibir)  flag_exibir = !flag_exibir;

    flag_menu = !flag_menu;                                   //Reseta a flag fazendo com que o programa volte ao menu principal
  
  } //end if botaoConf
    
} //end funcao buttons

/*======================//======================*/

void menu_list(){                                              //Função para rotatividade do menu 
  
  for(int i=(menu_max - 1); i>-1; i--)                      
  {
    indice = i-1;                                             //Indice de rotatividade
    line_bk[i] = line[i];                                     //Faz o backup da linha

    if(indice < 0)  line[i] = line_bk[i+(menu_max - 1)];      //Teste se é o ultimo menu para a rotatividade
    else  line[i] = line[i-1];                        
  }
  
} //end funcao menu_list

/*======================//======================*/

void menu_select(){                                         //Funcao de seleção de qual menu deve ser executado e
                                                            //modificacoes do que é apresentado na IHM
  switch(menu_number)
  {
    case 1:
      flag_temp = !flag_temp;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Defina a temp.:");
      lcd.setCursor(5,1);
      lcd.print(">");
      lcd.setCursor(8,1);
      lcd.print("*C");
      break;
      
    case 2:
      flag_umi = !flag_umi;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Defina a umid.:");
      lcd.setCursor(5,1);
      lcd.print(">");
      lcd.setCursor(8,1);
      lcd.print("%");
      break;

    case 3:
      flag_pre = !flag_pre;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Escolha o setup:");
      break;
      
    case 4:
      flag_exibir = !flag_exibir;
      
      lcd.clear();
      
      lcd.setCursor(0,0);
      lcd.print("Temp(D-A):");
      if(temp < 10) lcd.print("0");
      lcd.print(temp);
      lcd.setCursor(12,0);
      lcd.print("-");
      lcd.setCursor(15,0);
      lcd.print("C");
      
      lcd.setCursor(0,1);
      lcd.print("Umid(D-A):");
      if(umi < 10) lcd.print("0");
      lcd.print(umi);
      lcd.setCursor(12,1);
      lcd.print("-");
      lcd.setCursor(15,1);
      lcd.print("%");
      break;
      
  } //end switch menu
  
} //end funcao menu_select

/*======================//======================*/

void temperatura(){                                       //Função para confirmar a temperatura desejada e 
                                                          //resetar a variavel auxilar e a flag
  flag_temp = !flag_temp;
  temp = tempDesejado;
  tempDesejado = 20;
  
} //end funcao temperatura

/*======================//======================*/

void umidade(){                                           //Função para confirmar a umidade desejada e 
                                                          //resetar a variavel auxilar e a flag  
  flag_umi = !flag_umi;
  umi = umiDesejado;
  umiDesejado = 50;
  
} //end funcao umidade

/*======================//======================*/

void predef(){                                            //Função que seta os valores dos modos pré definidos

  flag_pre = !flag_pre;

  switch(pre)
  {
    case 1:
      temp = 25;
      umi = 90;
      break;
      
    case 2:
      temp = 30;
      umi = 85;
      break;
    
    case 3:
      temp = 30;
      umi = 75;
      break;

    case 4:
      temp = 28;
      umi = 85;
      break;
  }

  pre = 1;
 
} //end funcao predef

/*======================//======================*/

void atuadoresOn(){                                   //Função para ativar os atuadores dependendo das condições de
                                                      //temperatura e umidade desejados e atuais
  if(tempAtual <= temp - 1) {
    digitalWrite(peltHot, HIGH);
    digitalWrite(coolerHot, HIGH);
    digitalWrite(peltCold, LOW); 
    digitalWrite(coolerCold, LOW);
  }
  else if(tempAtual >= temp + 1){
    digitalWrite(peltCold, HIGH);
    digitalWrite(coolerCold, HIGH);
    digitalWrite(peltHot, LOW); 
    digitalWrite(coolerHot, LOW);
  }
  else{
    digitalWrite(peltHot, LOW); 
    digitalWrite(coolerHot, LOW);
    digitalWrite(peltCold, LOW); 
    digitalWrite(coolerCold, LOW);
  }

  if(umiAtual < umi) digitalWrite(umidificador, HIGH);
  else digitalWrite(umidificador, LOW);
  
} //end funcao atuadoresOn

/*======================//======================*/

void atuadoresOff(){                                //Função para desativar os atuadores quando o programa voltar ao menu principal
  
  digitalWrite(peltHot, LOW);
  digitalWrite(coolerHot, LOW);
  digitalWrite(peltCold, LOW);
  digitalWrite(coolerCold, LOW);
  digitalWrite(umidificador, LOW);
  
} //end funcao atuadoresOff

/*======================//======================*/

void leSensor(){

  flag_sensor = !flag_sensor;
  
  sensorDHT.read11(dht_pin);                       //Leitura do sensor DHT11 pela biblioteca "dht.h"
  tempAtual = sensorDHT.temperature;               //Variável recebe o valor da temperatura atual
  umiAtual = sensorDHT.humidity;                   //Variável recebe o valor da umidade atual

} //end funcao leSensor
