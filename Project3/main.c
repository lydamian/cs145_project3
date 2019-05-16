/*
 * Project2.c
 *
 * Created: 4/21/2019 7:49:20 PM
 * Author : lydam
 */ 

#include "avr.h"
#include "lcd.h"
#include <stdio.h>

//globals
	
const int MONTHS_MAX_DAYS[12] = {31, 28, 31,30, 31, 30, 31, 31, 30, 31, 30, 31};
int keypad_pressed[16];
/*const int notes[12][2] = 
{
	{45,45},{43,43},{40,40},
		{38,38},{36,36},{34,34},
			{32,32},{30,30},{29,29},
				{27,27},{26,26},{24,24}
};
*/
// determines if a key on the keypad is pressed
int is_pressed(int r, int c){
	// set all pins of c to N/C - weak 0.
	DDRC = 0;
	PORTC = 0;

	// set PORTC's # r GPIO to Weak 1
	SET_BIT(PORTC, r);
	// set PORTC's # c GPIO to strong 0
	SET_BIT(DDRC, c); // set direction to output
	CLR_BIT(PORTC, c); // strong 0
	if( GET_BIT(PINC, r) != 0){//check if r is not 0)
		// not pressed
		return 0;
	}
	else{
		// is pressed
		return 1;
	}
}

int get_key(){
	int r, c;
	for(r = 0; r < 4; r++){
		for(c = 0; c < 4; c++){
			if(is_pressed(r, c+4)){
				return (r*4 + c) + 1; // pin number
			}
		}
	}
	return 0;
}

/*
	INPUT: the year
	OUTPUT: 0 or 1 for (true or false)
	Leap Year Criteria:
	- The year can be evenly divided by 4;
	- If the year can be evenly divided by 100, it is NOT a leap year, unless;
	- The year is also evenly divisible by 400. Then it is a leap year.
*/
int isLeapYear(int year){
	if(year%100 == 0){
		if (year%400 == 0) { // may still possibly be a leap year
			return 1;
		}
		else{
			return 0;
		}
	}
	else if(year%4 == 0){
		return 1;
	}
	else{
		return 0;
	}
}

/*
	Description: Check if the given input year, 
	causes the given days of the month to be exceeded
*/
int exceedMonth(int year, int month, int day){
	int maxDays = MONTHS_MAX_DAYS[month];
	if (month == 2){
		if(isLeapYear(year)){
			if(30 <= day){
				return 1;
			}
		}
		else if(29 <= day){
			return 1;
		}
		else{
			return 0;
		}
	}
	else{
		if(day >= maxDays+1){
			return 1;
		}
		else{
			return 0;
		}
	}
	return 0;
}

int setup()
{
	SET_BIT(DDRA, 0);
	CLR_BIT(PORTA, 0);
	lcd_init();
	lcd_clr();
	SET_BIT(DDRA, 0);
	
	/*
	for(int i = 0; i < 16; i++){
		keypad_pressed[i] = 0;
	}
	*/
	
	return 1;
}

struct tm{
		int year; //year
		int month; //month
		int day; // day
		int hour;
		int minute;
		int second; //s
		int subsec; //ms
};

void initTm(struct tm *myTm, int y, int m, int d, int h, int min, int s, int ms){
	myTm->year = y;
	myTm->month = m;
	myTm->day = d;
	myTm->minute = min;
	myTm->second = s;
	myTm->hour = h;
	myTm->subsec = ms;
}
	

/*
	[INPUT] a pointer to a tm struct
	DISPLAY: MM/DD/YYYY on the top row,
	 bottom row: HH:MM:SS
*/
void display_time(struct tm *myTime){
	//local variables
	char top_row[11];
	char bottom_row[9];
	
	// TOP ROW
	sprintf(top_row, "%02d/%02d/%04d", myTime->month, myTime->day, myTime->year);
	lcd_pos(0, 2);
	lcd_puts2(top_row);
	
	// BOTTOM ROW
	sprintf(bottom_row, "%02d:%02d:%02d", myTime->hour, myTime->minute, myTime->second);
	lcd_pos(1, 2);
	lcd_puts2(bottom_row);
	
}

/*
	Conversions:
	1 year = 12 months, 1 month = ?days, 1 day = 24 hours; 
	1 hour = 60 minutes; 1 minute = 60 seconds; 1 second = 1000ms
*/
void keeptime(struct tm *myTime){
	myTime->subsec += 100;
	
	if(1000 == myTime->subsec){
		myTime->subsec = 0;
		myTime->second +=1;
		if(60 == myTime->second){
			myTime->second = 0;
			myTime->minute+=1;
			if(60 == myTime->minute){
				myTime->minute = 0;
				myTime->hour+=1;
			
					if(24 == myTime->hour){
						myTime->hour = 0;
						myTime->day+=1;
						
						// days are variable depending on the month
						if(exceedMonth(myTime->year, myTime->month, myTime->day)) {// handle days in a month)
							myTime->day = 1;
							myTime->month+=1;
						
							if(13 == myTime->month){
								myTime->month=1;
								myTime->year+=1;
							
								if(myTime->year > 3000){
									// error
									myTime->year = 2000;
								}
						}
					}
				}
			}
		}
	}
}

void blink_led(int k){
	for(int i = 0; i < k; i++){
		SET_BIT(PORTA, 0);
		avr_wait(200);
		CLR_BIT(PORTA, 0);
		avr_wait(2000);
	}
}

void decrement_tm(struct tm* tm, int row, int col){
	int trash;
	if(row == 0){
		switch(col){
			case 2: // decrement months by 10
				if(tm->month-10 > 0){
					tm->month -= 10;
				}
				break;
			case 3: // decrement months by 1
				if(tm->month-1 > 0){
					tm->month -= 1;
				}
				break;
			case 5: // decrement days by 10
				if(tm->day -10 > 0){
					tm->day -= 10;
				}
				break;
			case 6: // decrement days by 1
				if(tm->day-1 > 0){
					tm->day -= 1;
				}
				break;
			case 8: // decrement years by 1000, DO NOT IMPLEMENT THIS!
				break;
			case 9: // decrement years by 100
				if(tm->year - 100 > 2000 ){
					tm->year -= 100;
				}
				break;
			case 10: // decerement years by 10
				if(tm->year - 10 > 2000){
					tm->year -= 10;
				}
				break;
			case 11: // decrement years by 1
				if(tm->year -1 > 2000){
					tm->year -= 1;
				}
				break;
			default: // random statement so i dont get an error.
			trash = 1;
		}
	}
	else if (row == 1){
		switch(col){
			case 2: // decrement hours by 10
				if(tm->hour-10 >= 0){
					tm->hour -= 10;
				}
				break;
			case 3: // decrement hours by 1
				if(tm->hour-1 >= 0){
					tm->hour -= 1;
				}
				break;
			case 5: // decrement minutes by 10
				if(tm->minute-10 >= 0){
					tm->minute -= 10;
				}
				break;
			case 6: // decrement minutes by 1
				if(tm->minute-1 >= 0){
					tm->minute -= 1;
				}
				break;
			case 8: // decrement seconds by 10
				if(tm->second -10 >= 0){
					tm->second -= 10;
				}
				break;
			case 9: // decrement seconds by 1
				if(tm->second -1 >= 0){
					tm->second -= 1;
				}
				break;
			default: // random statement so i dont get an error
			trash = 1;
		}		
	}
}

void increment_tm(struct tm *myTm, int row, int col){
	int trash;
	if(row == 0){
		switch(col){
			case 2: // add 10 to months
				if(myTm->month + 10 < 13){
					myTm->month+=10;
				} 
			break;
			case 3: // add 1 to months
				if(myTm->month + 1 < 13){
					myTm->month+=1;
				}
			break;
			case 5: // add 10 to days
				if(!(exceedMonth(myTm->year, myTm->month, myTm->day+10))){
					myTm->day+=10;
				}
			break;
			case 6: // add 1 to days
				if(!(exceedMonth(myTm->year, myTm->month, myTm->day+1))){
					myTm->day+=1;
				}
			break;
			case 8: // add 1000 years, DO NOT change this....
			break;
			case 9: // add 100 years
			if(!isLeapYear(myTm->year+100) && myTm->day >= 29){
				// change the month to 28?
				myTm->day = 28;
			}
			myTm->year+=100;
			break;
			case 10: // add 10 years
				if(!isLeapYear(myTm->year+10) && myTm->day >= 29){
					// change the month to 28?
					myTm->day = 28;
				}			
			myTm->year+=10;
			case 11: // add 1 year
				if(!isLeapYear(myTm->year+1) && myTm->day >= 29){
					// change the month to 28?
					myTm->day = 28;
				}
				myTm->year+=1;
			break;
			default: //do nothing
			trash = 1;
		}
	}
	else if (row == 1){
		switch(col){
			case 2: // add 10 to hours
				if(myTm->hour+10 < 24){
					myTm->hour+=10;
				}
			break;
			case 3: // add 1 to hours
				if(myTm->hour+1 < 24){
					myTm->hour+=1;
				}
				break;
			case 5: // increment minutes by 10
				if(myTm->minute+10 < 60){
					myTm->minute+=10;
				}
				break;
			case 6: // increment minutes by 1
				if(myTm->minute + 1 < 60){
					myTm->minute+=1;
				}
				break;
			case 8: // increment seconds by 10
				if(myTm->second + 10 < 60){
					myTm->second += 10;
				}
				break;
			case 9: // increment seconds by 1
				if(myTm->second + 1 < 60){
					myTm->second+=1;
				}
				break;
			default:
			trash = 1;
		}
	}
}

int get_lcd_value(struct tm* myTm, int r, int c){
	int var = 0;
	
	if(r == 0){ // MM:DD:YYYY
		switch(c){
			case 3:
				return myTm->month/10;
			case 4:
				return myTm->month%10;
			case 6:
				return myTm->day/10;
			case 7:
				return myTm->day%10;
			case 9:
				return myTm->year/1000;
			case 10:
				var = myTm->year%10;
				var = var%10;
				var = var%10;
				return var;
			case 11:
				var = myTm->year%10;
				var = var%10;
				return var;
			case 12:
				var = myTm->year%10;
				return var;			
		}
	}
	else{ // HH:MIN:SS
		switch(c){
		case 3:
			return myTm->hour/10;
		case 4:
			return myTm->hour%10;
		case 6:
			return myTm->minute/10;
		case 7:
			return myTm->minute%10;
		case 9:
			return myTm->second/10;
		case 10:	
			return myTm->second%10;
		
		}
	}
}

/*
	Description: Blinks the current position
	on the lcd.
*/
void toggle_blink(struct tm* myTm, int r, int c){
	// get the value of the integer at r and c and store it to a variable x
	//int val = get_lcd_value(myTm, r, c);
	// set the value of the position at r and c to ' ' (space)
	lcd_pos(r, c);
	lcd_put(' ');
	avr_wait(60);
	display_time(myTm);
	avr_wait(60);
}

// this function sets the time
void set_time(struct tm *myTime){
	//configure a button on keypad -> programming mode
	int row = 0; // initial row position
	int col = 2; // initial column position
	int bVal = 0; // condition for while loop
	int trash;
	
	lcd_pos(row, col);
	// avr_wait(500);
	int k = get_key();
	if (k == 16) //D key enter programming mode
	{
		bVal = 1;
		while (bVal) //freezes main loop
		{
			toggle_blink(myTime, row, col);
			k = get_key();
			if (k == 8) //B key
			{
				// incremement
				increment_tm(myTime, row, col);	
			}
			else if (k == 4) //A key
			{
				//decrement
				decrement_tm(myTime, row, col);	
			}
			else if (k == 3) //3 key
			{
				//move left
				if(row == 0){
					if (row == 0 && (col ==  5|| col == 8))
					{
						col -= 2;
					}
					else if (row == 0 && col == 2)
					{
						col = 9;
						row = 1;
					}
					else
					{
						--col;
					}
					lcd_pos(row, col);
				}
				else if(row == 1){
					if(col == 2){ // go to top row
						row = 0;
						col = 10;
					}
					else if(col == 8 || col == 5){ // skip delimiter
						col -= 2;
					}
					else{
						col--;
					}
					lcd_pos(row, col);
				}
			}
			else if (k == 2) // 2 key
			{	
				//move right
				if(row == 0){
					if(col == 3 || col == 6){ // skip deliiter
						col += 2;
					}
					else if(col == 11){ // go to next row
						row = 1;
						col = 2;
					}
					else{
						col++;
					}
					lcd_pos(row, col);
				}
				else if(row == 1){
					if (row == 1 && (col == 3 || col == 6))
					{
						col += 2;
					}
					else if (row == 1 && col == 9)
					{
						col = 2;
						row = 0;
					}
					else
					{
						++col;
					}
					lcd_pos(row, col);
				}
			}
			else if(k == 12) // C key
			{
				// exiting
				bVal = 0;
				
			}
			display_time(myTime);
		}
	}
}

void test_keypad(){
	int k;
	k = get_key();
	avr_wait(10);
	//blink LED k times
	if(k != 0){
		blink_led(k);
	}
	//long time led on (1sec)
	avr_wait(300);
	//led off
}

struct note{
	int freq;
	int duration;
};

struct note notes[10];
int song_length = 27;
int some_notes[] = {2,2,2,-1,2,2,2,-1,2,4,0,1,2,-1,3,3,3,3,3,2,2,2,2,2,1,1,2,1,4};

void wait(int duration){
	CLR_BIT(PORTA, 0);
	avr_wait(duration);	
}

//contains all 5 notes required to play Jingle Bells
//instantiates the notes: frequency and duration
void inst_Jingle(int i){
	notes[0].freq = i*261; //C
	notes[0].duration = 200;
	notes[1].freq = i*293; //D
	notes[1].duration = 200;
	notes[2].freq = i*330;//E
	notes[2].duration = 200;
	notes[3].freq = i*349; //F
	notes[3].duration = 200;
	notes[4].freq = i*392; //G
	notes[4].duration = 200;
	
	notes[5].freq = 1;
	notes[5].duration = 200;
}

void play_note(struct note myNote){
	int freq = myNote.freq;
	int duration = myNote.duration;
	
	int i,n;
	double period = 1.0/freq;
	n = duration;
	
	double high = (period/2)*100000;
	double low = (period/2)*100000;
	int TH = (int)high;
	int TL = (int)low;

    //1  cycle
	for (i = 0; i < n; i++) {
		SET_BIT(PORTA,0);
		avr_wait_u(TH);
		CLR_BIT(PORTA,0);
		avr_wait_u(TL);
	}
	//return;
}

void play_song(int song[], int length){
	for(int i = 0; i < length; i++){
		if(song[i] == -1){
			wait(500);
			continue;
		}
		play_note(notes[song[i]]);
	}
	return ;
}

int main(void)
{
	//local variables
	int k;
	struct note note1, note2, note3, note4;
	note1.freq = 440;
	note1.duration = 150;
	note2.freq = 466.16;
	note2.duration = 150;
	note3.freq = 493;
	note3.duration = 150;
	note4.freq = 600;
	note4.duration = 150;
	
	// setting up
	setup();
	inst_Jingle(2);
	// main logic
    while (1) 
    {	
		play_song(some_notes, song_length);
		wait(150);
    }
}

