
#pragma once
#include <WString.h>

class GPS
{
	enum sentence { GGA, RMC, NONE };
public:
	char new_sentence[82];
	bool loc_updated;
	bool time_updated;
	bool sentence_updated;
	bool is_working;
	String locWE;
	String locNS;
	String time_fix;
	String new_sentence_string;
	int char_counter;
	sentence enum_sentence;

	GPS();
	bool encode();
	bool encode_char(char a);
	void setLocation(int start_loc_SN, int end_loc_SN, int start_loc_WE, int end_loc_WE);
	bool checkData(); 
	void setTime();
	bool fillNewSentence(char ch);
	void clearNewSentence();
	int parse(int comma_number);
	~GPS();
};

