/* CAMCOM lexical analyzer */
%{
#define YYSTYPE char *
    #include "y.tab.h" 
    #include <stdlib.h>
    #include <ctype.h>
    #include <string.h>
    void yyerror(char *);
    char * yytoupper(const char *);
    char * yytoupper4(const char *);
    void  start_include(const int);
    #include "camcom_proto.h"
    #include "camcom_data.h"
    #include "camcom_parameters.h"
    #define MAX_INCLUDE_DEPTH 4
    YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
    int include_stack_ptr=0;
%}

%s CV
%s DT
%s INCL
%s INCL2
%s RHS
%%

[ \t]                                                      ;


S(E(T)?)?                            { BEGIN CV; yylval="SET"; verb_do_set_add(yylval); return TOKVERB1; }
ADD                                  { BEGIN CV; yylval="ADD"; verb_do_set_add(yylval); return TOKVERB1; }
DO(I(T)?)?                           { BEGIN CV; yylval="DOIT"; verb_do_set_add(yylval); return TOKVERB1;}

SHOW|RESET|EXIT|DUMP                 {  yylval=yytoupper(yytext);  return TOKVERB2; }

GO                                   { yylval="GO"; verb_do_set_add(yylval); return TOKVERB4;}

GPIB                                 { yylval=yytoupper(yytext); return TOKDEP; }

SLEEP                                { yylval="SLEEP"; return TOKVERB5; }

FOREVER                              { yylval="FOREVER"; return TOKNYI; }

(TYPE|HELP).*                        { yylval=yytoupper4(yytext);  return TOKVERB3; }

(\$).*                                 { yylval=yytext;  return TOKVERB3; } /* No uppercase on purpose */

REW(I(N(D)?)?)?                      { yylval="REWIND";  return TOKNYI; }


\@    BEGIN INCL;  /* Indirect file specification */

<INCL>[ \t]*     /* eat the whitespace */

<INCL>[^ \t\n]+   BEGIN INCL2; start_include(1);    

<INCL2>[ \t]*

<INCL2>[0-9]+      start_include(2);  /* Numeric value parameter */     

<INCL2>\'P[0-9]\'? start_include(3);  /* Parameter being passed on */

<INCL2>[a-zA-Z0-9_]+  start_include(4);  /* String parameter */

<INCL2>\n        start_include(5); /* Go process the line */

[;\n]                                      return TOKTERM;

\^                                         BEGIN INITIAL;

<CV>\/                                     return TOKSLASH;

<CV>=                                      { BEGIN RHS; return TOKEQUAL; }

<CV>INTERFACE|GPIB_PERIOD|GPIB_WAIT|DADRESS|DSADDRESS    { yylval=yytoupper(yytext); return TOKDEP;}

<CV>NO(INTERFACE|GPIB_PERIOD|GPIB_WAIT|DADRESS|DSADDRESS) { yylval=yytoupper(yytext); return TOKDEP;}

<CV>GET|INIT|PUT                            { yylval=yytoupper(yytext); return TOKDEP;}

<CV>NO(GET|INIT|PUT)                        { yylval=yytoupper(yytext); return TOKDEP;}

<CV>INP(U(T)?)?                             { yylval="INPUT"; store_index(input_index); return TOKVALREQ;}

<CV>NOINP(U(T)?)?                           { yylval="NOINPUT"; store_index(noinput_index); return TOKNEGATED;}

<CV>OUTP(U(T)?)?                             { yylval="OUTPUT"; store_index(output_index); return TOKVALREQ;}

<CV>NOOUTP(U(T)?)?                           { yylval="NOOUTPUT"; store_index(nooutput_index); return TOKNEGATED;}

<CV>FI(L(L)?)?                              { store_index(fill_index); yylval="FILL"; return TOKNYI;}

<CV>BC(N(T)?)?                              { store_index(bcnt_index); yylval="BCNT"; return TOKVALREQ;}

<CV>CT(L(W)?)?                              { store_index(ctlw_index); yylval="CTLW"; return TOKVALREQ;}

<CV>DEBUG                                   { store_index(debug_index); yylval="DEBUG"; return TOKVALREQ;}

<CV>NODEBUG                                 { store_index(nodebug_index); yylval="DEBUG"; return TOKNEGATED;}

<CV>NOFI(L(L)?)?                             { store_index(nofill_index); yylval="NOFILL"; return TOKNYI;}

<CV>NOBC(N(T)?)?                             { store_index(nobcnt_index); yylval="NOBCNT"; return TOKNEGATED;}

<CV>NOCT(L(W)?)?                             { store_index(ctlw_index); yylval="NOCTLW"; return TOKNEGATED;}

<CV>CO(M(M(A(N(D)?)?)?)?)?                  { store_index(command_index); yylval="COMMAND";  return TOKNYI;}

<CV>DA(T(A)?)?                              { BEGIN DT; store_index(data_index); yylval="DATA";  return TOKDATA;}

<DT>=                                       return TOKEQUAL;

<DT>\(

<DT>[%A-Z0-9,]+                              { BEGIN CV; yylval=yytoupper(yytext); return TOKMSTRING;}

<CV>\)

<CV>NODA(T(A)?)?                            { store_index(nodata_index); yylval="NODATA";  return TOKNEGATED; }

<CV>NOF                                     { store_index(nofill_index); yylval="F"; return TOKNEGATED; }

<CV>F(N(C(O(D(E)?)?)?)?)?                   { store_index(fn_index); yylval="FNCODE";  return TOKVALREQ; }

<CV>A(D(D(R(E(S(S)?)?)?)?)?)?               { store_index(address_index); yylval="ADDRESS"; return TOKVALREQ; }

<CV>CR(A(T(E)?)?)?                          { store_index(crate_index); yylval="CRATE"; return TOKVALREQ; }

<CV>BR(A(N(C(H)?)?)?)?                      { store_index(branch_index); yylval="BRANCH"; return TOKVALREQ; }

<CV>M(O(D(U(L(E)?)?)?)?)?                    { store_index(module_index);  yylval="MODULE"; return TOKVALREQ; }

<CV>SA                      { store_index(sa_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>SC                      { store_index(sc_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>SN                      { store_index(sn_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>QM1                      { store_index(qm1_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>QM2                     { store_index(qm2_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>QM3                     { store_index(qm3_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>XM1                     { store_index(xm1_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>XM2                     { store_index(xm2_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>XM3                     { store_index(xm3_index); yylval=yytoupper(yytext);  return TOKNOVAL; }

<CV>NOSA                    { store_index(sa_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOSC                      { store_index(sc_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOSN                      { store_index(sn_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOQM1                      { store_index(qm1_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOQM2                     { store_index(qm2_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOQM3                     { store_index(qm3_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOXM1                     { store_index(xm1_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOXM2                     { store_index(xm2_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>NOXM3                     { store_index(xm3_index); yylval=yytoupper(yytext);  return TOKNEGATED; }

<CV>H(E(X)?)?                               { store_index(hex_index); yylval="HEX";   return TOKNOVAL; }

<CV>PIOP(_(C(H(E(C(K(S(U(M)?)?)?)?)?)?)?)?)?    { store_index(piop_checksum_index); yylval="PIOP_CHECKSUM";   return TOKNYI; }

<CV>RE_PACK                 { store_index(re_pack_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>LONG(W(O(R(D)?)?)?)?                  { store_index(longword_index); yylval="LONGWORD";   return TOKNOVAL; }

<CV>REPEAT                  { store_index(repeat_index); yylval=yytoupper(yytext);   return TOKNYI; }

<CV>NOH(E(X)?)?                                { store_index(hex_index); yylval="NOHEX";   return TOKNEGATED; }

<CV>NOPIOP(_(C(H(E(C(K(S(U(M)?)?)?)?)?)?)?)?)?    { store_index(piop_checksum_index); yylval="NO_PIOP_CHECKSUM";   return TOKNYI; }

<CV>NORE_PACK                  { store_index(re_pack_index); yylval=yytoupper(yytext);   return TOKNEGATED; }

<CV>NOLONGWORD                  { store_index(longword_index); yylval=yytoupper(yytext);   return TOKNEGATED; }

<CV>NOREPEAT                  { store_index(repeat_index); yylval=yytoupper(yytext);   return TOKNYI; }

<CV>P32                            { store_index(p32_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>P24                            { store_index(p24_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>NOP24                            { store_index(p24_index); yylval=yytoupper(yytext);   return TOKNEGATED; }

<CV>P16                            { store_index(p16_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>P8                            { store_index(p8_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>ILQ                    { store_index(ilq_index); yylval=yytoupper(yytext);   return TOKNOVAL; }

<CV>NOILQ                    { store_index(ilq_index); yylval=yytoupper(yytext);   return TOKNEGATED; }

<CV>Q(U(I(E(T)?)?)?)?                      { store_index(quiet_index); yylval="QUIET";   return TOKNOVAL; }

<CV>PR(O(M(P(T)?)?)?)?                     { store_index(prompt_index); yylval="PROMPT";  return TOKVALREQ; }

<CV>NOPR(O(M(P(T)?)?)?)?                     { store_index(noprompt_index); yylval="NOPROMPT";  return TOKNEGATED; }

<CV>VER(I(F(Y)?)?)?                        { store_index(verify_index); yylval="VERIFY";   return TOKNOVAL; }

<CV>NOQ(U(I(E(T)?)?)?)?                      { store_index(quiet_index); yylval="NOQUIET";   return TOKNEGATED; }

<CV>NOVER(I(F(Y)?)?)?                        { store_index(verify_index); yylval="NOVERIFY";   return TOKNEGATED; }

<RHS>NO[A-Z0-9]+                             { yylval=yytoupper(yytext);  BEGIN CV; return TOKNONOBAD;}

<RHS>0[xX][0-9A-F]+                           { yylval=yytoupper(yytext);  BEGIN CV; return TOKHEXNUM;}

<RHS>%[xXdD][0-9A-F]+                           { yylval=yytoupper(yytext);  BEGIN CV; return TOKHEXNUM;}

<RHS>[0-9A-F]+                           { yylval=yytoupper(yytext);  BEGIN CV; return TOKHEXNUM;}

<RHS>[A-Z0-9\.\-\_\~]+                           { yylval=yytoupper(yytext);  BEGIN CV; return TOKMSTRING;}

[0-9A-F]+                           { yylval=yytoupper(yytext);  return TOKHEXNUM;}

[A-Z0-9_]+                           { yylval=yytoupper(yytext);  return TOKSTRING;}

<RHS>\'P[0-9]\'?                     { yylval=yytoupper(yytext);    BEGIN CV; return TOKPARAM;}

\'P[0-9]\'?                          { yylval=yytoupper(yytext);   return TOKPARAM;}

!.*                                  return TOKCOMMENT ;

.             printf("Unknown character ");

%%

char * yytoupper(const char * token) 
{
 int i;
 int len;
 char *copy;
 len=strlen(token)+1;
 if (!(copy = malloc((u_int)len)))
      return (NULL);
 for(i=0;i<len;i++) copy[i]=toupper(token[i]);
 return (copy);
}

char * yytoupper4(const char * token) 
{
/* Uppercase only the first four characters */
 int i;
 int len;
 char *copy;
 len=strlen(token)+1;
 if (!(copy = malloc((u_int)len)))
      return (NULL);
 for(i=0;i<len;i++) 
     if(i<4) copy[i]=toupper(token[i]);
     else    copy[i]=token[i];
 return (copy);
}

int yywrap(void) {

  if(debug_level()>1) printf("Include stack Pointer: %d\n",include_stack_ptr);

  if(--include_stack_ptr<0) return 1;

  set_calling_level(include_stack_ptr);
  yy_delete_buffer(YY_CURRENT_BUFFER);
  yy_switch_to_buffer(include_stack[include_stack_ptr]);
  return 0;
}

void start_include(int phase)
{
     int calling_parameter_number=0;
     char fn[40];
     char* end;
     int param_index;
     char param_char;
     int param_number;
     int param_ref_index;

/* 0 = use camcom.tmp (transcription of direct DO command), 1 = get normal @ filename */

     if(phase==0)
        {
           strcpy(fn,"camcom.tmp");
           calling_parameter_number=0;
        }

     if(phase==1)
        {
          if(debug_level()>0) printf("Routine name = %s\n",yytext);
          strncpy(fn,yytext,38);
          calling_parameter_number=0;
          return;
        }

     param_index=(include_stack_ptr+1)*10+(calling_parameter_number++);

/* 2 = value, 3 = parameter, 4 = string */

     if(phase==2)
        {
          calling_param_values[param_index]=strtol(yytext,&end,10);
          if(debug_level()>0) printf("Parameter %d [numeric] = %d, index = %d\n",calling_parameter_number,calling_param_values[param_index],param_index);
          return;
        }
     if(phase==3)
        {
          param_char=*(yytext+2); param_number=param_char-'0';
          if (param_number > 0 )
             {
                param_ref_index=include_stack_ptr*10+param_number; 
                calling_param_values[param_index]=calling_param_values[param_ref_index];
             }
          if(debug_level()>0) printf("Parameter %d [%s] = %d, index = %d\n",calling_parameter_number,yytext,calling_param_values[param_index],param_index);
          return;
        }
     if(phase==4)
        {
          calling_param_string[param_index]=strdup(yytext);
          calling_param_values[param_index]=-99;
          if(debug_level()>0) printf("Parameter %d [string] = %s, index = %d\n",calling_parameter_number,calling_param_string[param_index],param_index);
          return;
        }

     if(include_stack_ptr>=MAX_INCLUDE_DEPTH)
       {
         printf("Includes nested too deeply processing file: %s\n",yytext);
         exit(1);
       }

     include_stack[include_stack_ptr++]=YY_CURRENT_BUFFER;
     set_calling_level(include_stack_ptr);
     yyin = fopen(fn,"r");
     if(!yyin) {
                printf("Error opening included file: %s\n",fn);
                exit(1);
             }
     else { 
               if(debug_level()>0) printf("Included file %s opened\n",fn);
               yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));  
               BEGIN(INITIAL);
             }

        if(debug_level()>1) printf("Include stack Pointer: %d\n",include_stack_ptr);
}
