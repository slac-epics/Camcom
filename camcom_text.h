/*

Abs: CAMCOM parameter text for SHOW

Name: camcom_text.h

Auth: 26-Jan-2010  R. Chestnut

*/

#ifndef camcom_text_h

#ifndef camcom_parameters_h
#include "camcom_parameters.h"
#endif

char noval_names[][16] =
 {"EXIT","HEX","PIOP_CHECKSUM","RE_PACK","LONGWORD","REPEAT","P32","P24","P16","P8",
  "ILQ","QUIET","VERIFY","SA","SC","SN","QM1","QM2","QM3","XM1","XM2","XM3",
  "FUNCTION","FILL","BCNT","DATA","PROMPT","DEBUG","INPUT","OUTPUT"};

char val_names[][11] =
  {"BRANCH","MODULE","CRATE","FN","ADDR","CTLW","FILL","SLEEP","BCNT",
   "COMMAND","DATA","PROMPT","DEBUG","INPUT","OUTPUT"};

#define camcom_text_h
#endif
