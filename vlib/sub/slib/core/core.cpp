/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <slib/core/perf.hpp>

using namespace slib;

sPerf sPerf::gPerf;

idx sPerf::gDebugStart=-1;
idx sPerf::gDebugEnd=-1;
idx sPerf::gDebugCurrent=-2;
bool sPerf::gPerfWallClock=false;
idx sTim_Global;

extern "C" {
sDef_ASCII sDef_ASCII_Set[]={
 {0,'\0',0,0}
,{1,'\0',0,0}
,{2,'\0',0,0}
,{3,'\0',0,0}
,{4,'\0',0,0}
,{5,'\0',0,0}
,{6,'\0',0,0}
,{7,'\0',0,0}
,{8,'\b',0,0}
,{9,'\t',0,0}
,{10,'\n',0,0}
,{11,'\0',0,0}
,{12,'\0',0,0}
,{13,'\0',0,0}
,{14,'\0',0,0}
,{15,'\0',0,0}
,{16,'\0',0,0}
,{17,'\0',0,0}
,{18,'\0',0,0}
,{19,'\0',0,0}
,{20,'\0',0,0}
,{21,'\0',0,0}
,{22,'\0',0,0}
,{23,'\0',0,0}
,{24,'\0',0,0}
,{25,'\0',0,0}
,{26,'\0',0,0}
,{27,'\0',0,0}
,{28,'\0',0,0}
,{29,'\0',0,0}
,{30,'\0',0,0}
,{31,'\0',0,0}
,{32,'\0',0,0}
,{33,'!',0,0}
,{34,'\"',0,0}
,{35,'#',0,0}
,{36,'$',0,0}
,{37,'%',0,0}
,{38,'&',0,0}
,{39,'\'',0,0}
,{40,'(',0,0}
,{41,')',0,0}
,{42,'*',0,0}
,{43,'+',0,0}
,{44,',',0,0}
,{45,'-',0,0}
,{46,'.',0,0}
,{47,'/',0,0}
,{48,'0',0,0}
,{49,'1',1,1}
,{50,'2',2,2}
,{51,'3',3,3}
,{52,'4',4,4}
,{53,'5',5,5}
,{54,'6',6,6}
,{55,'7',7,7}
,{56,'8',8,8}
,{57,'9',9,9}
,{58,':',0,0}
,{59,';',0,0}
,{60,'<',0,0}
,{61,'=',0,0}
,{62,'>',0,0}
,{63,'?',0,0}
,{64,'@',0,0}
,{65,'A',0,10}
,{66,'B',0,11}
,{67,'C',0,12}
,{68,'D',0,13}
,{69,'E',0,14}
,{70,'F',0,15}
,{71,'G',0,0}
,{72,'H',0,0}
,{73,'I',0,0}
,{74,'J',0,0}
,{75,'K',0,0}
,{76,'L',0,0}
,{77,'M',0,0}
,{78,'N',0,0}
,{79,'O',0,0}
,{80,'P',0,0}
,{81,'Q',0,0}
,{82,'R',0,0}
,{83,'S',0,0}
,{84,'T',0,0}
,{85,'U',0,0}
,{86,'V',0,0}
,{87,'W',0,0}
,{88,'X',0,0}
,{89,'Y',0,0}
,{90,'Z',0,0}
,{91,'[',0,0}
,{92,'\\',0,0}
,{93,']',0,0}
,{94,'^',0,0}
,{95,'_',0,0}
,{96,'`',0,0}
,{97,'a',0,10}
,{98,'b',0,11}
,{99,'c',0,12}
,{100,'d',0,13}
,{101,'e',0,14}
,{102,'f',0,15}
,{103,'g',0,0}
,{104,'h',0,0}
,{105,'i',0,0}
,{106,'j',0,0}
,{107,'k',0,0}
,{108,'l',0,0}
,{109,'m',0,0}
,{110,'n',0,0}
,{111,'o',0,0}
,{112,'p',0,0}
,{113,'q',0,0}
,{114,'r',0,0}
,{115,'s',0,0}
,{116,'t',0,0}
,{117,'u',0,0}
,{118,'v',0,0}
,{119,'w',0,0}
,{120,'x',0,0}
,{121,'y',0,0}
,{122,'z',0,0}
,{123,'{',0,0}
,{124,'|',0,0}
,{125,'}',0,0}
,{126,'~',0,0}
,{127,'\0',0,0}
};

};
