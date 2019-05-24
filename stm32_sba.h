#ifndef STM32_SBA_H
#define STM32_SBA_H


// Установка 0 или 1 на пине.
void LAT_A (char xb, char bul);   //  Write bit to pin:  
void LAT_B (char xb, char bul);   //  Write bit to pin:  
void LAT_C (char xb, char bul);   //  Write bit to pin:  
void LAT_D (char xb, char bul);   //  Write bit to pin:  
// xb = pin number
// bul  = 0 or 1

// Установка подтяжки к vdd или vss.
void PULL_A (char xb, char bul); // PIN Pull-up or pull-down 
void PULL_B (char xb, char bul); // PIN Pull-up or pull-down 
void PULL_C (char xb, char bul); // PIN Pull-up or pull-down 
void PULL_D (char xb, char bul); // PIN Pull-up or pull-down 
// xb = pin number
// bul = 0 (pull-down) or 1 (pull-up) 


// Чтение пина.
int PORT_A (char xb); // Read bit from pin
int PORT_B (char xb); // Read bit from pin
int PORT_C (char xb); // Read bit from pin
int PORT_D (char xb); // Read bit from pin
// xb - pin number

// Конфигурация пина.
void CONF_A (char pin, char cnf, char mode); // PORT CONFIG
void CONF_B (char pin, char cnf, char mode); // PORT CONFIG
void CONF_C (char pin, char cnf, char mode); // PORT CONFIG
void CONF_D (char pin, char cnf, char mode); // PORT CONFIG
// pin - Номер пина.
// mode - режим вход / выход (IN, OUT10, OUT20, OUT50)
//
// cnf (IN):
//
//	00 – аналоговый вход.
//	01 – вход в третьем состоянии. (Устанавливается после сброса).
//	10 – вход с подтягивающим резистором (направление устанавливается в GPIOx->ODR).
//	11 – зарезервировано для будущих применений.
//
// cnf (OUT10, OUT20 или OUT50):
//
//	00 – цифровой выход
//	01 – цифровой выход с открытым стоком
//	10 – цифровой выход, подключенный специализированным блокам
//	11 – цифровой выход, подключенный специализированным блокам с открытым стоком 
//
//

#define IN 0
#define OUT10 1 
#define OUT20 2
#define OUT50 3

#define CNF_ANALOG_IN 0
#define CNF_TRISTATE_IN 1
#define CNF_PULL_IN 2

#define CNF_DIGITAL_OUT 0
#define CNF_OPEN_DRAIN_OUT 1
#define CNF_SPECIAL_OUT 2
#define CNF_SPECIAL_OPENDRAIN_OUT 3



#endif
