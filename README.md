# Software
O software foi desenvolvido na IDE do arduino e gravado no microcontrolador através de um arduino uno.

## Bibliotecas
Apenas duas bibliotecas foram utilizadas, sendo estas: 
1. [LiquidCrystal.h](https://www.arduino.cc/en/Reference/LiquidCrystal), para utilização do display 16x2. 
2. [dht.h](https://www.arduino.cc/reference/en/libraries/dht-sensor-library/), para leitura dos dados do sensor DHT11.

## Interrupção
A interrupção foi desenvolvida a partir do timer2 do microcontrolador e utilizada para a amostragem do sensor a cada trinta segundos.
Duas variáveis auxiliares são incrementadas, uma a cada estouro do timer para a contagem de 1 segundo e outra a cada segundo. Após os 30 segundos uma flag é 
setada para que a rotina de leitura do sensor seja executada.
