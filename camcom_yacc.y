/* CAMCOM2 grammar file */

%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "camcom_proto.h"
  #include "camcom_data.h"
  int yylex(void);
  void yyerror(char *);
  #define DEBUG1 debug_level()>0
  #define DEBUG2 debug_level()>1
%}

%token  TOKHEXNUM
%token  TOKVALREQ
%token  TOKNONO
%token  TOKEQUAL
%token  TOKTERM
%token  TOKSLASH
%token  TOKNONOBAD
%token  TOKDEP
%token  TOKVERB1
%token  TOKVERB2
%token  TOKVERB3
%token  TOKVERB4
%token  TOKVERB5
%token  TOKFOREVER
%token  TOKSTRING
%token  TOKNOVAL
%token  TOKNEGATED
%token  TOKCOMMENT
%token  TOKPARAM
%token  TOKMSTRING
%token  TOKDATA
%token  TOKNYI

%%

commands:
   | commands command
;

command:
        error TOKTERM {yyclearin; yyerrok; printf("Unrecognized token %s\n",(char*)$1); issue_prompt(0);}
     |  verb_type1
     | TOKCOMMENT TOKTERM {issue_prompt(2);}
     | TOKSLASH modifiers
     | verb_type2 {issue_prompt(4);}
     | verb_type3 {issue_prompt(6);}
     | verb_type4 {issue_prompt(7);}
     | verb_type5 {issue_prompt(8);}
     | TOKTERM {issue_prompt(9);}
;

verb_type5:
       TOKVERB5 TOKTERM {if(DEBUG2)printf("SLEEP\n"); camcom_sleep(1);}
     | TOKVERB5 TOKHEXNUM TOKTERM {if(DEBUG2)printf("SLEEP %s\n",(char*)$2); camcom_sleep_num((char*)$2);}
     | TOKVERB5 TOKPARAM TOKTERM {if(DEBUG2)printf("SLEEP %s \n",(char*)$2); camcom_sleep_param((char*)$2);}
;
verb_type4:
       TOKVERB4 TOKTERM {if(DEBUG2)printf("GO \n"); prepare_real_packet(); ca_activity();}
     | TOKVERB4 TOKFOREVER TOKTERM {if(DEBUG2)printf("%s %s\n",(char*)$1,(char*)$2);}
;
verb_type3:
       TOKVERB3 TOKTERM {if(DEBUG1)printf("%s\n",(char*)$1); help_type_param((char*)$1);}
;
verb_type2:
       TOKVERB2 TOKTERM {if(show_reset_exit((char*)$1)==1) YYACCEPT;}
;

verb_type1:
       TOKVERB1 {if(DEBUG2)printf("Complex verb %s found\n",(char*)$1);}
;

modifiers:
       | modifiers modifier TOKSLASH
       | modifiers modifier TOKTERM {printf("All modifiers processed \n"); prepare_real_packet(); issue_prompt(1);}
;

modifier:
       TOKVALREQ TOKEQUAL TOKMSTRING {if(DEBUG2)printf("Modifier %s with required string %s \n",(char*)$1,(char*)$3); token_with_string((char*)$1,(char*)$3);}
     | TOKVALREQ TOKEQUAL TOKHEXNUM {if(DEBUG2)printf("Modifier %s with required number %s \n",(char*)$1,(char*)$3); token_with_number((char*)$1,(char*)$3);}
     | TOKVALREQ TOKEQUAL TOKPARAM {if(DEBUG2)printf("Modifier %s with required parameter %s \n",(char*)$1,(char*)$3); token_with_parameter((char*)$1,(char*)$3);}
     | TOKVALREQ {printf("Modifier %s needs a value\n",(char*)$1);}
     | TOKDATA TOKEQUAL TOKMSTRING {if(DEBUG2)printf("DATA: %s \n",(char*)$3); token_with_string((char*)$1,(char*)$3);}
     | TOKDEP {printf("Deprecated modifier: %s \n",(char*)$1);}
     | TOKNYI {printf("Modifier %s not yet implemented; ignored.\n",(char*)$1);}
     | TOKNOVAL TOKEQUAL TOKMSTRING {printf("String %s illegal for modifier %s \n",(char*)$3,(char*)$1);}
     | TOKNOVAL TOKEQUAL TOKHEXNUM {printf("Value %s illegal for modifier %s \n",(char*)$3,(char*)$1);}
     | TOKNOVAL TOKEQUAL TOKPARAM {printf("Parameter %s illegal for modifier %s \n",(char*)$3,(char*)$1);}
     | TOKNOVAL {token_noval(); if(DEBUG2)printf("Modifier %s recognized\n",(char*)$1);}
     | TOKNEGATED {token_noval_no(); if(DEBUG2)printf("Negated modifier %s recognized\n",(char*)$1);}
     | TOKNONO  {printf("Modifier %s may not be negated\n",(char*)$1);}
     | TOKNONOBAD {printf("Negated modifier %s not recognized\n",(char*)$1);}
;

%%

void yyerror(char *s) {fprintf(stderr,"%s\n",s);}

extern FILE *yyin;

int main(int argc, char *argv[]) {

  int i;
  char* end;

  if(argc>1)
    {
  if(strncmp(argv[1],"-h",2)==0)
    {
      printf("caCAMCOM command summary:\n");
      printf("\tSET - set one or more parameters in a CAMAC packet\n");
      printf("\tADD - start another CAMAC packet\n");
      printf("\tDOIT or GO - execute the packets\n");
      printf("\tRESET - clear all the packets\n");
      printf("\tSHOW - show all the current parameters\n");
      printf("\tEXIT - quit the program\n");
      printf("\tDUMP - show more detailed packet information\n");
      printf("\tSLEEP ms - wait the specified number of milliseconds\n");
      printf("caCAMCOM parameters:\n");
      printf("\t/INPUT=file containing data\n");
      printf("\t/OUTPUT=file for data output\n");
      printf("\t/BCNT=nbytes - specify space for nbytes of output\n");
      printf("\t/DATA=(data1,data2,..) - specify data for write functions\n");
      printf("\t/DATA=dataword - specify one data word\n");
      printf("\t/HEX - from now on data are hex\n");
      printf("\tF(NCODE)=nn CAMAC function\n");
      printf("\tCR(ATE)=nn CAMAC crate\n");
      printf("\tMO(DULE)=nn CAMAC module number\n");
      printf("\tA(DDRESS)=nn CAMAC subaddress\n");
      printf("\tBR(ANCH)=AAnn Branch (IOC name - LI21, for example)\n");
      printf("\tP24 - pack 24 bits into each data word\n");
      printf("\tLONGWORD - use 32 bit data words\n");
      printf("\tQUIET - don't report back status\n");
      printf("\tPR(OMPT)=new CAMCOM prompt\n");
      return 0;
    }

  if(strncmp(argv[1],"-ex",3)==0)
    {
        printf("caCAMCOM command examples\n");
        printf(" set/br=li21/cr=2/mo=3/fn=16/data=4\n");
	printf(" Branch(IOC) = LI21, Crate=2, Module=3,Function=16\n");
        printf(" go\n");
        printf(" execute the command\n");
	  printf(" add/hex/p24/data=(abc,1a86,12f4)\n");
        printf(" add a packet with three pieces of 24-bit data\n");
        printf(" go\n");
        printf(" execute both packets\n");
        printf(" reset\n");
        printf(" set/br=li21/cr=2/mo=3/fn=0/bcnt=32\n");
        printf(" set the function to read and expect 32 bytes of data\n");

      return 0;
    }
    }

  printf("\nCommand-line arguments:\n");
  for(i=0;i<argc;i++)
    {
      printf("\n argv[%d] = %s",i,argv[i]);
      if(i>1) 
       {
         calling_param_string[i-2]=strdup(argv[i]);
         calling_param_values[i-2]=strtol(argv[i],&end,10);
         printf(" : parameter %d, numeric value = %d",i-2,calling_param_values[i-2]);
         if(argv[i]=='\0' || *end!='\0') 
           {
             calling_param_values[i-2]=-99;
             printf("\nParameter stored as string, not number");
           }
       } 
    }

  yyin = fopen(argv[1],"r"); 

  printf("\nCAMCOM>");
  setup_whole_packet();
  camcom_packet_clean_all();
  set_calling_level(0);
  yyparse();
  fclose(yyin);
  return 0;
}


