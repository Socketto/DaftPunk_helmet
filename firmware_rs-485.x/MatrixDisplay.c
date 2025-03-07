#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "MCP23017.h"
#include "24lc256.h"

extern unsigned char Frase[100];

unsigned char Frasi[8][40] = {
    {"~5LOVE~5"},
    {"~6I love music~6"},
    {"DAFTPUNK"},
    {"~5Camilla i love you!~5"},
    {"WOW"},
    {"~6Daft music! ~6"},
    {"Special helmet"},
    {"Anni 90 forever!"}
};
unsigned char Display[16];  //8bit x 16columns
unsigned char i, count, abit;
extern unsigned char MusicValue;
extern unsigned char PhoneConnected;



unsigned char BarraVolume(unsigned char volume)
{
    if(volume&0x80)
        return 0xFF;
    if(volume&0x40)
        return 0x7F;
    if(volume&0x20)
        return 0x3F;
    if(volume&0x10)
        return 0x1F;
    if(volume&0x08)
        return 0x0F;
    if(volume&0x04)
        return 0x07;
    if(volume&0x02)
        return 0x03;
    if(volume&0x01)
        return 0x01;
    return 0;
}
unsigned char BarraVolume2(unsigned char volume)
{
    if(volume&0x80)
        return 0xFF;
    if(volume&0x40)
        return 0xFE;
    if(volume&0x20)
        return 0x7E;
    if(volume&0x10)
        return 0x7C;
    if(volume&0x08)
        return 0x3C;
    if(volume&0x04)
        return 0x38;
    if(volume&0x02)
        return 0x18;
    if(volume&0x01)
        return 0x08;
    return 0;
}

/*Board 002 reverse bit of second byte*/
unsigned char reverseByte(unsigned char num)
{
    unsigned char output;
    for(count=1;count<=8;count++)
    { 
        abit = (unsigned char) (num & 0x01);
        num = (unsigned char) (num>>1);
        output = (unsigned char) (output<<1); 
        if(abit==1) 
        output = (unsigned char) (output+1);
    }
    return output;
}

void Refresh()
{
	for(i = 0 ; i < MCP_COUNTS ; i++)
	{
		SetOutputs(MCPAddresses[i], /*reverseByte*/((unsigned char)(Display[ (unsigned char) (( i * 2 ) + 1) ])),reverseByte( Display[ (unsigned char) (i * 2) ]) );
	}
}

void Write_Point(unsigned char X, unsigned char Y)
{
	Display[X] ^=  Y;
}

unsigned char ReadColumn(unsigned char column)
{
    if(column < 16)
    {
        return Display[column];
    }
    return 0;
}

void Write_Column(unsigned char X, unsigned char Value)
{
	Display[X] = Value;
}

void ShiftScreen(unsigned char Value)
{
	for(i = 0 ; i < 15 ; i++)
	{
        Display[i] = Display[i+1];
    }
    Display[15] = Value;
}

void ClearScreen()
{
	memset(Display,0x00,16);
}

void InvertScreen()
{
	for(i = 0 ; i < 16 ; i++)
	{
		Display[i] ^= 0xFF;
	}
}





/*Games*/

/*---------*/
/*-ColumnDraw-*/
/*---------*/
unsigned char ColumnDraw = 0,RowDraw = 0, ColumnDrawandamento = 0, RowDrawandamento = 0;
unsigned char ColumnDraw2 = 0,RowDraw2 = 0, ColumnDrawandamento2 = 0, RowDrawandamento2 = 0;
unsigned short volteColumnDraw = 0;
unsigned char SuperCar(unsigned char mode)
{
    volteColumnDraw++;
    if(volteColumnDraw >= 500)
    {
        volteColumnDraw=0;
        return 1;
    }
    if(ColumnDrawandamento == 0)
    {
        if(mode == 2)
        {
            ColumnDrawandamento = 1;
            ColumnDraw = 15;
        }
        ColumnDraw++;
        if(ColumnDraw == 15)
        {
            ColumnDrawandamento = 1;
        }
    }
    else
    {
        if(mode == 1)
        {
            ColumnDrawandamento = 0;
            ColumnDraw = 0;
        }
        else
        {
            ColumnDraw--;
            if(ColumnDraw == 0)
            {   
                ColumnDrawandamento = 0;
            }
        }
    }
    if(mode == 0)
        ClearScreen();
    if(mode >= 3)
    {
        if(mode == 5 || mode == 6)
            ClearScreen();
        if(PhoneConnected)
        {
            if(mode == 3)
                Write_Point(ColumnDraw,BarraVolume(MusicValue));
            if(mode == 4)
                Write_Point(ColumnDraw,BarraVolume2(MusicValue));
            if(mode == 5)
                Write_Point(ColumnDraw,BarraVolume(MusicValue));
            if(mode == 6)
                Write_Point(ColumnDraw,BarraVolume2(MusicValue));
        }
        else
        {
            return 1;
        }
    }
    else
    {
        Write_Point(ColumnDraw,0xFF);
    }
    Refresh();
    return 0;
}



unsigned int internalCountExecution = 0;
unsigned char TestWriteMusic(unsigned char mode)
{
    if(PhoneConnected == 0)
    {
        return 1;
    }
    if(internalCountExecution == 0)
    {
        ClearScreen();
    }
    if(internalCountExecution % 2 == 0)
    {
        Display[0] = Display[1];
        Display[1] = Display[2];
        Display[2] = Display[3];
        Display[3] = Display[4];
        Display[4] = Display[5];
        Display[5] = Display[6];
        Display[6] = Display[7];
        Display[7] = Display[8];
        Display[8] = Display[9];
        Display[9] = Display[10];
        Display[10] = Display[11];
        Display[11] = Display[12];
        Display[12] = Display[13];
        Display[13] = Display[14];
        Display[14] = Display[15];
        if(mode == 0)
        {
            Display[15] = BarraVolume(MusicValue);
        }
        if(mode == 1)
        {
            Display[15] = BarraVolume2(MusicValue);
        }
        Refresh();
    }
    internalCountExecution++;
    if(internalCountExecution > 600)
    {
        ClearScreen();
        internalCountExecution = 0;
        return 1;
    }
    return 0;
}

unsigned char BoingPoint(unsigned char p)
{    
	volteColumnDraw++;
    if(volteColumnDraw >= 500)
    {
        volteColumnDraw=0;
        return 1;
    }
    
    if(ColumnDrawandamento == 0)
    {
        ColumnDraw++;
        if(ColumnDraw == 15)
        {
            ColumnDrawandamento = 1;
        }
    }
    else
    {
        ColumnDraw--;
        if(ColumnDraw == 0)
        {   
            ColumnDrawandamento = 0;
        }
    }
	if(RowDraw == 0)
	{
		RowDraw = 1;
	}
	else
	{
		if(RowDrawandamento == 0)
		{
			RowDraw = RowDraw * 2;
			if(RowDraw == 0x80)
			{
				RowDrawandamento = 1;
			}
		}
		else
		{
			RowDraw = RowDraw / 2;
			if(RowDraw == 0)
			{   
				RowDrawandamento = 0;
			}
		}
	}
    if(p > 0)
    {
        if(ColumnDrawandamento2 == 0)
        {
            ColumnDraw2++;
            if(ColumnDraw2 == 15)
            {
                ColumnDrawandamento2 = 1;
            }
        }
        else
        {
            ColumnDraw2--;
            if(ColumnDraw2 == 0)
            {   
                ColumnDrawandamento2 = 0;
            }
        }
        if(RowDraw2 == 0)
        {
            RowDraw2 = 1;
        }
        else
        {
            if(RowDrawandamento2 == 0)
            {
                RowDraw2 = RowDraw2 * 2;
                if(RowDraw2 == 0x80)
                {
                    RowDrawandamento2 = 1;
                }
            }
            else
            {
                RowDraw2 = RowDraw2 / 2;
                if(RowDraw2 == 0)
                {   
                    RowDrawandamento2 = 0;
                }
            }
        }
        if(ColumnDraw2 == ColumnDraw)
        {
            Write_Point(ColumnDraw,RowDraw | RowDraw2);
        }
        else
        {
            Write_Point(ColumnDraw2,RowDraw2);
            Write_Point(ColumnDraw,RowDraw);
        }
    }
    else
    {
        Write_Point(ColumnDraw,RowDraw);
    }
    Refresh();
    return 0;
}

unsigned char puntLecter = 0;
unsigned char puntLecterEEPROM =0;
unsigned int CurrentValueChar  =0;
unsigned char timeOutDisplayRefreshScrivi = 3;
unsigned char CurrentExecution = 0;
void StopScriviFrase()
{
	ClearScreen();
	puntLecter = 0;
    puntLecterEEPROM = 0;
    CurrentValueChar  =0;
    timeOutDisplayRefreshScrivi = 3;
}

unsigned char ScriviFrase(unsigned char UseFrase)
{
    unsigned char* FF = Frase;
    if(UseFrase > 0)
        FF = Frasi[UseFrase-1];
    timeOutDisplayRefreshScrivi--;
    if(timeOutDisplayRefreshScrivi == 0)
    {
        timeOutDisplayRefreshScrivi = 3;
        if((puntLecterEEPROM == 6) && (FF[puntLecter] != 0))
        {
           puntLecterEEPROM = 0;
           puntLecter++;
        }

        if((puntLecter == 0) && (puntLecterEEPROM==0))
        {
            ClearScreen();
        }
        if(FF[puntLecter] == 0)
        {   
            if(puntLecterEEPROM < 16)
            {
                puntLecterEEPROM++;
                ShiftScreen(0x00);
                Refresh();
                return 0;
            }
            else
            {
                return 1;
            }
        }
        if(FF[puntLecter] == '~')
        {
            puntLecter++;
            return 0;
        }
        if(puntLecter>0 && FF[puntLecter-1] == '~')
        {
            /*special character!!!*/ 
            switch(FF[puntLecter])
            {
                case '1' : CurrentValueChar =  (72*6) + puntLecterEEPROM; break;
                case '2' : CurrentValueChar =  (73*6) + puntLecterEEPROM; break;
                case '3' : CurrentValueChar =  (75*6) + puntLecterEEPROM; break;
                case '4' : CurrentValueChar =  (76*6) + puntLecterEEPROM; break;
                case '5' : CurrentValueChar =  (79*6) + puntLecterEEPROM; break;
                case '6' : CurrentValueChar =  (81*6) + puntLecterEEPROM; break;
                default: CurrentValueChar = -1;
            }
        }
        else
        {
            /*normal char*/    
            if((FF[puntLecter] >= 65) && (FF[puntLecter] <= 90))
            {
                /*A--Z*/
                CurrentValueChar = ((FF[puntLecter] - 65)*6) + puntLecterEEPROM;
            }
            else
            {
                if((FF[puntLecter] >= 97) && (FF[puntLecter] <= 122))
                {
                    /*a--z*/
                    CurrentValueChar =  FF[puntLecter];
                    CurrentValueChar = CurrentValueChar + 26;/*TABELLA EEPROM*/
                    CurrentValueChar = CurrentValueChar - 97;/*Tabella ASCII*/
                    CurrentValueChar =  (CurrentValueChar*6) + puntLecterEEPROM;
                }
                else
                {
                    if((FF[puntLecter] >= 49) && (FF[puntLecter] <= 57))
                    {
                        /*1--9*/
                        CurrentValueChar =  FF[puntLecter];
                        CurrentValueChar = CurrentValueChar + 52;/*TABELLA EEPROM*/
                        CurrentValueChar = CurrentValueChar - 49;/*Tabella ASCII*/
                        CurrentValueChar =  (CurrentValueChar*6) + puntLecterEEPROM;
                    }
                    else
                    {
                        switch(FF[puntLecter])
                        {
                            case '?' : CurrentValueChar =  (61*6) + puntLecterEEPROM; break;
                            case '!' : CurrentValueChar =  (62*6) + puntLecterEEPROM; break;
                            case '0' : CurrentValueChar =  (63*6) + puntLecterEEPROM; break;
                            case '@' : CurrentValueChar =  (64*6) + puntLecterEEPROM; break;
                            case '#' : CurrentValueChar =  (65*6) + puntLecterEEPROM; break;
                            case '&' : CurrentValueChar =  (66*6) + puntLecterEEPROM; break;
                            case '(' : CurrentValueChar =  (67*6) + puntLecterEEPROM; break;
                            case ')' : CurrentValueChar =  (68*6) + puntLecterEEPROM; break;
                            case '+' : CurrentValueChar =  (69*6) + puntLecterEEPROM; break;
                            case '-' : CurrentValueChar =  (70*6) + puntLecterEEPROM; break;
                            case '$' : CurrentValueChar =  (74*6) + puntLecterEEPROM; break;
                            case '=' : CurrentValueChar =  (71*6) + puntLecterEEPROM; break;
                            case '*' : CurrentValueChar =  (80*6) + puntLecterEEPROM; break;
                            case '.' : CurrentValueChar =  (77*6) + puntLecterEEPROM; break;
                            case ',' : CurrentValueChar =  (78*6) + puntLecterEEPROM; break;
                            default: CurrentValueChar = -1;
                        }
                    }
                }
            }
        }
        if(CurrentValueChar!=-1)
        {
            ShiftScreen((unsigned char)EE_byte_read(CurrentValueChar));
        }
        else
        {
            ShiftScreen(0);
        }
        Refresh();
        puntLecterEEPROM++;
    }
    return 0;
}

#define START_EXECUTION 3
void UpdateDisplay()
{
    /*switch per la selezione di cosa fare sul display*/
    switch(CurrentExecution)
    {
        case 0:
                strcpy(Frase,"SYSTEM ONLINE");
                CurrentExecution++;
            break;
        case 1:
            if(ScriviFrase(0) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 2:
            strcpy(Frase,"Daft Punk Helmet project!, by Sambin");
            CurrentExecution++;
            break;
        case 3:
            if(ScriviFrase(0) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 4:
            if(SuperCar(0) == 1)
            {
                CurrentExecution++;
                ClearScreen();
            }
            break;
        case 5:
            if(SuperCar(1) == 1)
            {
                CurrentExecution++;
                ClearScreen();
            }
            break;
        case 6:
            if(TestWriteMusic(0) == 1)
            {
                CurrentExecution++;
                ClearScreen();
            }
            break;
        case 7:
            if(TestWriteMusic(1) == 1)
            {
                CurrentExecution++;
                ClearScreen();
            }
            break;
        case 8:
            if(ScriviFrase(1) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 9:
            if(SuperCar(2) == 1)
            {
                ClearScreen();
                CurrentExecution++;
            }
            break;
        case 10:
            if(ScriviFrase(8) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 11:
            if(SuperCar(3) == 1)
            {
                ClearScreen();
                CurrentExecution++;
            }
            break;
        case 12:
            if(ScriviFrase(2) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 13:
            if(SuperCar(4) == 1)
            {
                ClearScreen();
                CurrentExecution++;
            }
            break;
        case 14:
            if(ScriviFrase(3) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 15:
            if(SuperCar(5) == 1)
            {
                ClearScreen();
                CurrentExecution++;
            }
            break;
        case 16:
            if(ScriviFrase(4) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 17:
            if(SuperCar(6) == 1)
            {
                ClearScreen();
                CurrentExecution++;
            }
            break;
        case 18:
            if(ScriviFrase(5) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 19:
            ClearScreen();
            if(BoingPoint(0) == 1)
                CurrentExecution++;
            break;
        case 20:
            if(ScriviFrase(6) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 21:
            ClearScreen();
            if(BoingPoint(1) == 1)
                CurrentExecution++;
            break;
        case 22:
            if(BoingPoint(0) == 1)
                CurrentExecution++;
            break;
        case 23:
            if(ScriviFrase(7) == 1)
            {
                StopScriviFrase();
                CurrentExecution++;
            }
            break;
        case 24:
            if(BoingPoint(1) == 1)
                CurrentExecution=START_EXECUTION;
            break;
        default:
                CurrentExecution=START_EXECUTION;
            break;
    }
}