/*
 * attiny13_upr_nasosom_02.cpp
 *
 * Created: 15.12.2022 19:14:00
 *  Author: san
 */ 
#define F_CPU 1200000UL


#include <avr/io.h>
#include <util/delay.h>

//подільник напруги 22к/10кОМ 
#define VOLTAGE_UP 199     //напруга вмикання насосу  12.6 в
#define VOLTAGE_DOWN 191   //напруга вимикання насосу 12 в
#define TIME_POWER_UP 1000 //час на який вмикається насос при кроткочасному замиканні верхнього датчика
#define TIME_POWER_LED 750 //період блимання світлодіода що відображює напругу

int adc_read (void)
{
	// Start the conversion
	ADCSRA |= (1 << ADSC);
	
	// Wait for it to finish
	while (ADCSRA & (1 << ADSC));
	
	return ADCH;
}



int main(void)
{
/* PB0 - вихід на світлодіод out
 * PB1 - вихід на керування ключом насосу out
 * PB2 - вхід на АЦП з акумулятора in
 * PB3 - вхід датчику рівня води верхній in (спрацьовує коли води достатньо)
 * PB4 - вхід датчику рівня води нижний in (спрацьовує коли мало води)
 *
 * Напруга на акумуляторі 11.5 - 14 V. Вибрано для більшого строку служби елементів. 
 * 
 * 3.0-3.35 V, - 90% ємності (х 4 шт. = 12 - 13.4 V)
 * Зарядка забезпчується подаванням стабілізованної напруги 14 V на батарею. 
 *
*/
	DDRB |= (0 << PB4)|(0 << PB3)|(0 << PB2)|(1 << PB1)|(1 << PB0);
	PORTB = (1<<PORTB4) | (1<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0); //підтягуючи резистори до датчиків PB3, PB4


	// Set the ADC input to PB2/ADC1
	ADMUX |= (1 << MUX0);
	ADMUX |= (1 << ADLAR);
	ADMUX &= (~(1<<REFS0));  //опорне - напруга живлення
	DIDR0 |= (1 << PB2);   //заборона цифрового входу на контакті PB2
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);



	unsigned int adc_in;    //напруга на кумуляторі
	unsigned char i;        //змінна для циклу
	unsigned int time_on=0; //затримка на гарантований час увімкнення насосу
	unsigned char water_up; //рівень води в нормі
	unsigned char power_up; //напруга в нормі
	unsigned int power_led = TIME_POWER_LED; //тривалість горіння світлодіода
	
	while(1)
    {
		_delay_ms(1);
		adc_in=0;
		for (i= 0; i<10; i++){		
			adc_in = adc_in + adc_read();		//робимо 10 замірів напруги на акумуляторі
		}
		adc_in = adc_in / 10;
		
		if (adc_in > VOLTAGE_UP){    //якщо напруга більше ніж мінімально допустима для вмикання
			power_up = 1;
		}
		if(adc_in < VOLTAGE_DOWN){
			power_up = 0;            //інакше нуль
		}
			
		if (time_on > 0){
			time_on = time_on-1;     //підраховуємо час увімкнення
			water_up = 1;
		}else{	
			if (!(PINB & (1<<PINB3))){ //верхній рівень
				water_up = 1;
				time_on = TIME_POWER_UP; 
			}else{
				water_up = 0;
				
			}
		}
		
		if (!(PINB & (1<<PINB4))){ //ніжній рівень
			water_up = 0;
			time_on = 0;
		}
		
		
		if((water_up == 1) && (power_up == 1)){  // якщо рівень води та напруга в нормі
			PORTB |= (1<<PORTB1);                // вмикаємо насос
			
		}else{
			PORTB &= ~(1<<PORTB1);               // вимикаємо насос
			
		}

        //блимаємо світлодіодом-показчиком напруги
		if (power_led > (TIME_POWER_LED)){	//новий цикл відображення
			power_led = 0;
		}
		power_led = power_led + 1;
		if (adc_in < VOLTAGE_DOWN){
			adc_in = VOLTAGE_DOWN;
		}
		if (((adc_in - VOLTAGE_DOWN) * 25) > power_led){
			PORTB |= (1<<PORTB0);
			}else{
			PORTB &= ~(1<<PORTB0);
		}
		
		      
    }
}
