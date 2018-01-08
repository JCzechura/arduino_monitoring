#include "GPS_lib.h"
#include <WString.h>
GPS::GPS()
{
	sentence_updated = false;
	loc_updated = false;
	time_updated = false;
	char_counter = 0;
	locWE = "";
	locNS = "";
	time_fix = "";
     is_working = false;

}
bool GPS::encode_char(char a)
{
	//new_sentence_string+= a;
	switch (a)
	{
		case '*' : 
			new_sentence[char_counter++]= a;
			//encode();
			return true;
		case '$' : 
			char_counter = 0;	
			return false;
		case '\r': 
			return false;
		case '\n': 
			return false;
		default : 
			new_sentence[char_counter++] = a;	
			if (char_counter>81)
				char_counter = 0;
			return false;
	}
	
}
bool GPS::encode()
{
	if (new_sentence[2] == 'G' &&  new_sentence[3] == 'G' && new_sentence[4] == 'A')
	{
		enum_sentence = GGA;
		if (checkData())
		{
			setLocation(parse(2), (parse(3) - 2), parse(4), (parse(5) - 2));
			setTime();
		}

	}	
	else if (new_sentence[2] == 'R' &&  new_sentence[3] == 'M' && new_sentence[4] == 'C')
	{
		enum_sentence = RMC;
		if (checkData())
		{
			setLocation(parse(3), (parse(4) - 2), parse(5), (parse(6)-2));
			setTime();
		}
	}
	else
	{
		enum_sentence = NONE;
		loc_updated = false;
		time_updated = false;
	}

	clearNewSentence();
	return true;

}

void GPS::setLocation(int start_loc_SN, int end_loc_SN, int start_loc_WE, int end_loc_WE)
{
	if ((end_loc_SN < start_loc_SN) || (end_loc_WE < start_loc_WE))
	{
		loc_updated = false;
		return;
	}
	else
	{
		locNS = "";
		for (int i = start_loc_SN; i <= end_loc_SN; i++)
		{
			if (i == (start_loc_SN+2))
				locNS += "#"; 
			locNS+=new_sentence[i];
		}
		locNS += "#";
		locNS += new_sentence[end_loc_SN + 2];

		locWE = "";
		for (int i = start_loc_WE; i <= end_loc_WE; i++)
		{
			if (i == (start_loc_WE+3))
				locWE += "#";
			locWE+=new_sentence[i];
		}
		locWE += "#";
		locWE += new_sentence [end_loc_WE + 2];

		loc_updated = true;
		return;
	}
}
bool GPS::checkData()
{
	bool good = false;
	if ((new_sentence[0] != 'G') && (new_sentence[1] != 'P'))
		return false;
	for (int i =2; i < 82; i++)
	{
		if (new_sentence[i] == '*')
			good = true;
	}
	if (!good) return false;
	if (enum_sentence == GGA)
	{
		if ((new_sentence[parse(6)] == '0') || (parse(6) == 0))
{
is_working = false;
			return false;
}
		else
{ 
is_working = true;
return true;
}
	}
	else if (enum_sentence == RMC)
	{
		if ((new_sentence[parse(2)] == 'V') || (parse(2) == 0))
{
is_working = false;
			return false;
}
		else 
{
is_working = true;
return true;
}
	}
}
void GPS::setTime()
{
	if (new_sentence[6] != ',')
	{
		time_fix= "" ;
		time_fix+=new_sentence[6];
		time_fix+=new_sentence[7];
		time_fix+= ":";
		time_fix+=new_sentence[8];
		time_fix+=new_sentence[9];
		time_fix+= ":";
		time_fix+=new_sentence[10];
		time_fix+=new_sentence[11];
		time_updated = true;
	}
}
bool GPS::fillNewSentence(char ch)
{
	if (char_counter<81)
	{
		new_sentence[char_counter] = ch;
		char_counter++;
	}
	sentence_updated = true;
	return true;
}
void GPS::clearNewSentence()
{
	//new_sentence_string = "";
	for (int i = 0; i < 82; i++)
		new_sentence[i] = 0;

}
int GPS::parse(int comma_number)
{
	int counter = 0;
	for (int i = 0; i < 82; i++)
	{
		if (new_sentence[i] == ',')
		{
			counter++;
			if (counter == comma_number)
				return (i + 1);
		}
	}
	return 0;
}

GPS::~GPS()
{
}
