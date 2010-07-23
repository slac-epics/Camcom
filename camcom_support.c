/* CAMCOM support routines */
#include "cctlwmasks.h"
#include "camcom_data.h"
#include "camcom_proto.h"
#include "camcom_parameters.h"
#include "slc_macros.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "camcom_text.h"

#define DEBUG params_numeric[debug_index]

FILE *output_file;

short params_noval[N_noval_index];

short params_valued[N_val_index];

long params_numeric[N_val_index];

long data_array[67];

char* params_values[N_val_index];

short current_index;

short current_packet;

short read_fn;

short write_fn;

char main_verb[6];

int calling_level=0;

long control_word_getter()
{
  long value=0;
  /*
  long next_value;
  */
  if(params_values[branch_index]==0) return value;
  value|=((params_numeric[module_index]<<(-CTLW_MODULE_SHIFT)) & (CTLW_MODULE_MASK));
  /*
  next_value=((params_numeric[module_index]<<(-CTLW_MODULE_SHIFT)) & (CTLW_MODULE_MASK));
  printf("MO value = 0x%x\n",next_value);
  */
  value|=((params_numeric[crate_index]<<(-CTLW_CRATE_SHIFT)) & (CTLW_CRATE_MASK));
  /*
  next_value=((params_numeric[crate_index]<<(-CTLW_CRATE_SHIFT)) & (CTLW_CRATE_MASK));
  printf("CR value = 0x%x\n",next_value);
  */
  value|=((params_numeric[fn_index]<<(-CTLW_FUNCTION_SHIFT)) & (CTLW_FUNCTION_MASK));
  /*
  next_value=((params_numeric[fn_index]<<(-CTLW_FUNCTION_SHIFT)) & (CTLW_FUNCTION_MASK));
  printf("FN value = 0x%x\n",next_value);
  */  
  value |= (params_numeric[address_index] & (CTLW_SUBADDR_MASK));

  if(params_noval[p24_index]==1)      value|=CTLW_P24;
  if(params_noval[p8_index]==1)       value|=CTLW_P8;
  if(params_noval[re_pack_index]==1)  value|=CTLW_RE_PACK;
  if(params_noval[qm1_index]==1)      value|=CTLW_QM1;
  if(params_noval[qm2_index]==1)      value|=CTLW_QM2;
  if(params_noval[xm1_index]==1)      value|=CTLW_XM1;
  if(params_noval[xm2_index]==1)      value|=CTLW_XM2;
  if(params_noval[sa_index]==1)       value|=CTLW_SA;
  if(params_noval[sn_index]==1)       value|=CTLW_SM;
  if(params_noval[sc_index]==1)       value|=CTLW_SC;
  if(params_noval[ilq_index]==1)      value|=CTLW_ILQ;
  return value;
}

void issue_prompt(int which)
{
  if(DEBUG>0)
    {
      if(params_values[prompt_index]==0) printf("%d: CAMCOM>",which);
      else printf("%d: %s>",which,params_values[prompt_index]);
    }
  else
    {
      if(params_values[prompt_index]==0) printf("CAMCOM>");
      else printf("%s>",params_values[prompt_index]);
    }
}

long debug_level()
{
  return DEBUG;
}

void check_for_action()
{
  if(  (strncmp(main_verb,"GO",2)==0 || strncmp(main_verb,"DOIT",4)==0) )
    {
        ca_activity();
    }

  return;
}

void prepare_real_packet()
{
  int i,j;

  short* mptr;
  long* lptr;

  short bcnt,wcnt;

  int out_index =0;

  if (DEBUG>0) printf("Current Packet = %d\n",current_packet);

  camblk_tok_p->control_block.iop=current_packet+1;
  packet_ctlw[current_packet].l=control_word_getter();
  packet_read[current_packet]=read_fn=(params_numeric[fn_index] & 24)==0;
  packet_write[current_packet]=write_fn=(params_numeric[fn_index] & 24)==16;
  packet_bcnt[current_packet]=params_numeric[bcnt_index];
  packet_pack8[current_packet]=params_noval[re_pack_index];

  if(params_noval[p32_index]||params_noval[p24_index]||params_noval[longword_index])  packet_32bitdata[current_packet]=2;
  else packet_32bitdata[current_packet]=1;

  if (DEBUG>0) printf("Ctlw 0x%lx, Func code %ld for packet %d, read: %d, write: %d\n",
    packet_ctlw[current_packet].l,params_numeric[fn_index],
    current_packet,packet_read[current_packet],packet_write[current_packet]);

  if(  (strncmp(main_verb,"GO",2)==0||strncmp(main_verb,"DOIT",4)==0) &&
     (params_values[branch_index]==0||
     params_numeric[module_index]==0||
      params_numeric[crate_index]==0))
    { printf(" Undefined Branch, Crate, or Module \n"); return;}

  /* DOIT or GO */

  get_data_if_needed();

  for(i=0;i<current_packet+1;i++)
    {
  if (DEBUG>0) printf("Control word: 0x%lx, wcmax: %d,32bit mult: %d, read: %d,write: %d\n",packet_ctlw[i].l,
      packet_wcmax[i],packet_32bitdata[i],packet_read[i],packet_write[i]);

      camblk_tok_p->packet[i].ctlw=packet_ctlw[i].l;
      camblk_tok_p->packet[i].wc_max=packet_wcmax[i]*((packet_bcnt[i]!=0)?1:packet_32bitdata[i]);
      if(packet_pack8[i]!=0)
	{
          bcnt=packet_wcmax[i];
          wcnt=(bcnt+1)/2;
	}
      else
	{
          wcnt=packet_wcmax[i];
          bcnt=2*wcnt;
	}
      out_index=(DATA_START_OFFSET+i*STAT_DATA_LENGTH)/2;
      if(DEBUG>0) printf("Packet, offset, wcmax, bcnt, wcnt = %d %d %d %d %d\n",i,out_index,packet_wcmax[i],bcnt,wcnt);

      /* Find the status longword and set it to zero */

       camblk_tok_p->packet[i].statdata=out_index;      
       mptr=&camblk_tok_p->control_block.key;
       lptr=(long *)mptr;

       if(DEBUG>1) printf("Block.key address (long) = 0x%lx\n",(long)lptr); 
       /*
       if(DEBUG>1) printf("Block.key address (short) = %lx\n",(long)mptr); 
       */
       mptr+=out_index+2;
       lptr+=out_index/2;
       *lptr=0xbadbad12;
       if(DEBUG>1) printf("Status set to 0xbadbad12 for packet %d at address 0x%lx\n",i,(long)lptr);
       lptr++;

       if(packet_write[i]!=0)
	{
               for(j=0;j<wcnt;j++) 
	    {
              if(packet_32bitdata[i]==2) *lptr = packet_data[i][j+1];
                                    else *mptr = packet_data[i][j+1];  
              if(DEBUG>1) 
		{
                  if(packet_32bitdata[i]==2) 
                     printf("Data copy (packet, index, addr, dec, hex) -- %d %d 0x%lx %ld 0x%lx\n",i,j,(long)lptr,*lptr,*lptr);
                  else printf("Data copy (packet, index, addr, dec, hex) -- %d %d 0x%lx %d 0x%x\n",i,j,(long)mptr,*mptr,*mptr);
		}
              mptr++; lptr++;
	    }
	}
    }
  camblk_tok_p->control_block.tbytes=2*(DATA_START_OFFSET+current_packet*STAT_DATA_LENGTH);
  out_buffer_len=out_index+STAT_DATA_LENGTH;
  return;
}

void print_token_table()
{
  int i;

  printf("\nOff/On parameters:\n   ");
  for (i=0;i<N_noval_index;i++) { printf(" %s:%d ",noval_names[i],params_noval[i]); if (i%6==5) printf("\n   ");}
  printf("\n\nParameters with Data:\nIndex\tName\t\tNumber\tString \n");
  for(i=0;i<N_val_index;i++)
    printf("%d\t%s\t\t%ld\t%s\n",i,val_names[i],params_numeric[i],params_values[i]);
  printf("Func code %ld, read: %d, write: %d\n",params_numeric[fn_index],read_fn,write_fn);
  if (params_numeric[data_index]>0)  
   for(i=0;i<params_numeric[data_index];i++)
     {
	if(params_noval[hex_index]==0) printf("Decimal data element %d = %ld\n",i,data_array[i]);
	else                           printf("Hex data element %d = 0x%lx\n",i,data_array[i]);
     }
  return;
}

void dump_packet()
{
  int i,j;
  char* key;
  int ctl_buffer_len;
  short* mptr_s;
  long* mptr_l;
  char* mptr_c;
  key=strdup((char*)&camblk_tok_p->control_block.key);
  key[2]='\0';
  prepare_real_packet();

  ctl_buffer_len=current_packet*6+12;
      mptr_c=(char *)&camblk_tok_p->control_block.key;
      printf("Buffer Dump (chars), length = %d bytes\n",ctl_buffer_len*2);
      for(i=0;i<ctl_buffer_len*2;i++)
	{
	   if(i%10==0) printf("%3d:",i);
	   printf("  %2x",*mptr_c++);
           if(i%10==9) printf("\n");
	}
      printf("\n");

      mptr_s=&camblk_tok_p->control_block.key;
      printf("Buffer Dump (shorts), length = %d short words\n",ctl_buffer_len);
      for(i=0;i<ctl_buffer_len;i++)
	{
	   if(i%10==0) printf("%3d:",i);
	   printf("  %4x",*mptr_s++);
           if(i%10==9) printf("\n");
	}
      printf("\n");

      mptr_l=(long *)&camblk_tok_p->control_block.key;
      printf("Buffer Dump (longs), length = %d long words\n",ctl_buffer_len/2);
      for(i=0;i<ctl_buffer_len/2;i++)
	{
	   if(i%5==0) printf("%3d:",i);
	   printf("  0x%8lx",*mptr_l++);
           if(i%5==4) printf("\n");
	}
      printf("\n");

  printf("CAMAC Packet Header\n Key: %s\n nops: %d\n iop: %d\n tbytes: %d\n bits: 0x%x\n",
	 key,
         camblk_tok_p->control_block.nops,
         camblk_tok_p->control_block.iop,
         camblk_tok_p->control_block.tbytes,
         camblk_tok_p->control_block.bit_summary);
         mptr_s=&camblk_tok_p->control_block.key;
         printf(" Camac buffer base addr = 0x%lx\n",(long)mptr_s);

  for(i=0;i<current_packet+1;i++)
    {
      printf("CAMAC Block %d:\n ctlw: 0x%lx\n data offset: 0x%lx words\n wc_max: %d\n WC Mult: %d\n",i,
	     camblk_tok_p->packet[i].ctlw,
             camblk_tok_p->packet[i].statdata,
	     camblk_tok_p->packet[i].wc_max,
             packet_32bitdata[i]);

      if(camblk_tok_p->packet[i].wc_max>0)
	{
         mptr_l=(long *)&camblk_tok_p->control_block.key;
         mptr_s=&camblk_tok_p->control_block.key;
         mptr_s+=camblk_tok_p->packet[i].statdata;
         printf(" Data buffer base addr = 0x%8.8lx\n",(long)mptr_s);
         printf(" Data Buffer: \n");
         mptr_s+=2;
         mptr_l+=camblk_tok_p->packet[i].statdata/2+1;
         if(packet_32bitdata[i]==1)
          for(j=0;j<camblk_tok_p->packet[i].wc_max;j++)
            {
              if(j%10==0) printf("shortwords (@ 0x%8.8lx) %3d:",(long)mptr_s,j);
              printf("  0x%x",*mptr_s++);
              if(j%10==9) printf("\n");
	    }
         if(packet_32bitdata[i]==2)
          for(j=0;j<camblk_tok_p->packet[i].wc_max/2;j++)
            {
              if(j%5==0) printf("longwords (@ 0x%8.8lx) %3d:",(long)mptr_l,j);
              printf("  0x%lx",*mptr_l++);
              if(j%5==4) printf("\n");
	    } 
	}
      else 
	{
           if(!packet_read[i]) printf("No data for this control packet");
           else printf("No data -- no word count returned for READ");
	}
      printf("\n");  
    }       
  return;
}

void store_index(int index_value)
{
  current_index=index_value;
  if(DEBUG>1) printf("Current_index set to %d \n",current_index);
  return;
}

long show_reset_exit(const char* token_p)
{
if(DEBUG>1)printf("Simple verb %s found\n",token_p);
 if(strncmp(token_p,"EXIT",4)==0) return 1;
 if(strncmp(token_p,"SHOW",4)==0) print_token_table();
 if(strncmp(token_p,"RESET",5)==0) camcom_packet_clean_all();
 if(strncmp(token_p,"DUMP",4)==0) dump_packet(); 
return 0;
}

void camcom_packet_clean_all()
{
  int i;
  int camblk_bc;
  camblk_bc = sizeof(camac_ctlb_ts)+NOPS*(sizeof(camac_pkt_ts)+4);
  for(i=0;i<N_noval_index;i++) params_noval[i]=0;
  for(i=0;i<N_val_index;i++)
    {
      if(i!=debug_index) params_numeric[i]=0;
      params_values[i]=0;
    }
  current_packet=0;

  for(i=0;i<NOPS;i++)
    {
	 memset(&packet_data,0,256*NOPS);
	 memset(&packet_wcmax,0,2*NOPS);
	 memset(&packet_read,0,2*NOPS);
	 memset(&packet_write,0,2*NOPS);
	 memset(&packet_pack8,0,2*NOPS);
         memset(&packet_32bitdata,0,2*NOPS);
	 memset(&packet_bcnt,0,2*NOPS);
         memset(&packet_ctlw,0,4*NOPS);
    }
  /*
  memset(camblk_tok_p,0,camblk_bc);
  */
  memset(camblk_tok_p,0,8192);
  memcpy(&camblk_tok_p->control_block.key,"FR",2);
  camblk_tok_p->control_block.nops=NOPS;
  return;
}

void camcom_packet_clean_sticky()
{
  params_noval[exit_index]=0;
  params_noval[p32_index]=0;
  params_noval[p24_index]=0;
  params_noval[p16_index]=0;
  params_noval[p8_index]=0;
  params_noval[longword_index]=0;

  params_values[data_index]=0;
  params_numeric[data_index]=0;
  
}

void setup_whole_packet()
{
  int camblk_bc;
  camblk_bc = sizeof(camac_ctlb_ts)+NOPS*(sizeof(camac_pkt_ts)+4);
  camblk_tok_p = (camac_pkg_ts*) malloc (camblk_bc);
  memset(camblk_tok_p,0,camblk_bc);
  memcpy(&camblk_tok_p->control_block.key,"FR",2);
  camblk_tok_p->control_block.nops=NOPS;
  current_packet=0;
}

void ca_activity()
{
#include <cadef.h>
  chid id[10];
  int state=0;
  short active_token, new_token, status, in_len;
  int i;

#define BUFLEN 4096
#define IOC_NAME params_values[branch_index]
#define ID_AT 0
#define ST_AT "%s:CAMCOM:ACTIVE_TOKEN"
#define ID_NT 1
#define ST_NT "%s:CAMCOM:NEW_TOKEN"
#define ID_INB 2
#define ST_INB "%s:CAMCOM:INBUFF"
#define ID_INL 3
#define ST_INL "%s:CAMCOM:INBUFF:LEN"
#define ID_OUTB 4
#define ST_OUTB "%s:CAMCOM:OUTBUFF"
#define ID_OUTL 5
#define ST_OUTL "%s:CAMCOM:OUTBUFF:LEN"
#define ID_ST 6
#define ST_ST "%s:CAMCOM:STATUS"
#define ID_NOTE 7
#define ST_NOTE "%s:CAMCOM:NOTE"
#define ID_VER 8
#define ST_VER "%s:CAMCOM:SEQ:VER"
#define ID_NTP 9
#define ST_NTP "%s:CAMCOM:NEW_TOKEN.PROC"
#define CAMCOM_FREE 0
#define CAMCOM_TAKEN 1
#define CAMCOM_HOST_DONE 2
#define CAMCOM_BUSY 3
#define CAMCOM_DONE 4

  char pv_name[40];
  char note_name[40];

  sprintf(pv_name,ST_ST,IOC_NAME);
  if(DEBUG>1) printf("PV STATUS name = %s\n",pv_name);

  if (ECA_NORMAL != ca_task_initialize()) 
    {
      printf("Channel Access not initializing -- wrong BRANCH??\n");
      return;
    }

  /* Connect to STATUS and get the value */
 
  status = CAMCOM_TAKEN; i=0;
  do
    {
      if(i>0) 
        { printf("CAMCOM busy for %s, trying again\n",IOC_NAME);sleep(1);}
  if (ECA_NORMAL == ca_search_and_connect(pv_name, &id[ID_ST], NULL, NULL)) {
    if (ECA_NORMAL == ca_pend_io(1.0)) {
      if (ECA_NORMAL == ca_get(DBR_SHORT, id[ID_ST], &status)) {
        if (ECA_NORMAL != ca_pend_io(1.0)) {
          printf("ca_pend_io failed for get of %s \n",pv_name); return; }
      } else {
	printf("ca_get of PV %s failed\n",pv_name); return; }
    } else {
      printf("ca_pend_io failed connection for %s\n",pv_name); return; }
  } else {
    printf("ca_search_and_connect failed for %s\n",pv_name); return; }

    } while (++i<2 && (status == CAMCOM_TAKEN));
  /* Check for FREE -- if not, communicate and  bail out */

  sprintf(pv_name,ST_NOTE,IOC_NAME);
  if(DEBUG>1) printf("PV NOTE name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_NOTE], NULL, NULL); ca_pend_io(1.0);
  ca_get(DBR_STRING, id[ID_NOTE], &note_name);
  if(status!=CAMCOM_FREE) 
         {printf("IOC %s CAMCOM is serving %s\n",IOC_NAME,note_name); return;} 


  /* Mark the packet "in use by me" */

  status = CAMCOM_TAKEN;
  state=ca_put(DBR_SHORT, id[ID_ST], &status); ca_pend_io(1.0);
  sprintf(note_name,"Ron testing on %s",IOC_NAME);
  state=ca_put(DBR_STRING, id[ID_NOTE],&note_name);

  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}

  /* Poke the token generator in the database */

  sprintf(pv_name,ST_NTP,IOC_NAME);
  if(DEBUG>1)  printf("PV NEW TOKEN poke name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_NTP], NULL, NULL); ca_pend_io(1.0);
  i=1;
  state=ca_put(DBR_SHORT, id[ID_NTP], &i); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}

  /* Get the new token value */

  sprintf(pv_name,ST_NT,IOC_NAME);
  if(DEBUG>1) printf("PV NEW TOKEN name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_NT], NULL, NULL); ca_pend_io(1.0);
  state=ca_get(DBR_SHORT, id[ID_NT], &new_token); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}
  if(DEBUG>1) printf("New token = %d\n",new_token);

  /* Get the current active token value */

  sprintf(pv_name,ST_AT,IOC_NAME);
  if(DEBUG>1) printf("PV ACTIVE TOKEN name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_AT], NULL, NULL); ca_pend_io(1.0);
  state=ca_get(DBR_SHORT, id[ID_AT], &active_token); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}
  if(DEBUG>1) printf("Active token = %d\n",active_token);  

  if(active_token != new_token) {printf("IOC %s CAMCOM is otherwise in use\n",IOC_NAME); return;}

  /* Enter the reservation note */

  sprintf(pv_name,ST_NOTE,IOC_NAME);
  if(DEBUG>1) printf("PV NOTE name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_NOTE], NULL, NULL); ca_pend_io(1.0);

  /* Set the buffer length PV value */

  sprintf(pv_name,ST_INL,IOC_NAME);
  if(DEBUG>1) printf("PV BUFLEN name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_INL], NULL, NULL); ca_pend_io(1.0);
  in_len=out_buffer_len*2;
  ca_put(DBR_SHORT, id[ID_INL], &in_len); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}

  /* Write the buffer out */

  sprintf(pv_name,ST_INB,IOC_NAME);
  if(DEBUG>1) printf("PV BUFFER name = %s\n",pv_name);
  ca_search_and_connect(pv_name, &id[ID_INB], NULL, NULL); ca_pend_io(1.0);
  /*
  in_len=BUFLEN;
  */
  ca_array_put(DBR_CHAR, in_len, id[ID_INB], &camblk_tok_p->control_block.key); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}

  /* Set the status such that the IOC wakes up to do its work */

  status = CAMCOM_HOST_DONE;
  ca_put(DBR_SHORT, id[ID_ST], &status); ca_pend_io(1.0);

  /* Watch the status for 10 seconds */

  for(i=0;i<20;i++)
    {
      usleep(1000);
      /*
      printf("One iteration of sleep\n");

      */      ca_get(DBR_SHORT, id[ID_ST], &status); ca_pend_io(1.0);
      if(status == CAMCOM_DONE)
        {
	  printf("Camcom operation completed \n");
          break;
	}
    }

  /* Mark CAMCOM as useable by others */

  status = CAMCOM_FREE;
  ca_put(DBR_SHORT, id[ID_ST], &status); ca_pend_io(1.0);

  /* Read the buffer back in */

  in_len=BUFLEN;
  ca_array_get(DBR_CHAR, in_len, id[ID_INB], &camblk_tok_p->control_block.key); ca_pend_io(1.0);
  if(state==cs_never_conn) {printf("PV name <%s> not found\n",pv_name); return;}
 
  process_returned_data();

  return;
}

void process_returned_data()
{
  short i,j;
  short* mptr_s;
  long* mptr_l;
  long stat_long;
  float* mptr_f;
  short x_bit,q_bit;

  if(DEBUG>1)
    {
  printf("CAMAC Packet Header\n nops: %d\n iop: %d\n tbytes: %d\n bits: 0x%x\n",
         camblk_tok_p->control_block.nops,
         camblk_tok_p->control_block.iop,
         camblk_tok_p->control_block.tbytes,
         camblk_tok_p->control_block.bit_summary);
         mptr_s=&camblk_tok_p->control_block.key;
         printf(" Camac buffer base addr = 0x%lx\n",(long)mptr_s);
    }
  for(i=0;i<current_packet+1;i++)
    {
      mptr_l=(long *)&camblk_tok_p->control_block.key;
      mptr_l+=camblk_tok_p->packet[i].statdata/2;
      x_bit=(*mptr_l&STAT_X)>0;
      q_bit=(*mptr_l&STAT_Q)>0;
      stat_long=*mptr_l;
      if(DEBUG>1)
	{  
      printf("CAMAC Block %d:\n ctlw: 0x%lx\n data offset: 0x%lx words\n wc_max: %d\n Status: 0x%lx\n Data buffer:\n",i,
	     camblk_tok_p->packet[i].ctlw,
             camblk_tok_p->packet[i].statdata,
	     camblk_tok_p->packet[i].wc_max,
             *mptr_l);
	}
      if(camblk_tok_p->packet[i].wc_max>0)
	{
         mptr_s=&camblk_tok_p->control_block.key;
         mptr_s+=camblk_tok_p->packet[i].statdata+2;
         mptr_l++;
         if(packet_32bitdata[i]==1)
          for(j=0;j<camblk_tok_p->packet[i].wc_max;j++)
            {
              if(DEBUG>1 && j%10==0) printf("shortwords (@ 0x%lx) %3d:",(long)mptr_s,j);
              if(DEBUG>1) printf("  0x%x",*mptr_s++);
              if(!params_valued[output_index]&&packet_read[i])
                printf(" %i\t%d\t%4x\n",j+1,*mptr_s,*mptr_s);
              else if(packet_read[i])
                fprintf(output_file," %i\t%d\t%4x\n",j+1,*mptr_s,*mptr_s);
              if(j%10==9) 
	        {      
		  if(DEBUG>1) printf("\n");
                }
	    }

         if(packet_32bitdata[i]==2)
          for(j=0;j<camblk_tok_p->packet[i].wc_max/2;j++)
            {
              if(DEBUG>1 && j%5==0) printf("longwords (@ 0x%lx) %3d:",(long)mptr_l,j);
              if (DEBUG>1) printf("  0x%lx",*mptr_l);
              mptr_l++;
              mptr_f=(float *)mptr_l;
              if(!params_valued[output_index]&&packet_read[i])
                printf(" %2.2i\t%ld\t%8lx\t%#10.4g\n",j,*mptr_l,*mptr_l,*mptr_f);
              else if(packet_read[j])
                fprintf(output_file," %2.2i\t%ld\t%8lx\t%#10.4g\n",j,*mptr_l,*mptr_l,*mptr_f);
              if(j%5==4) 
		{
		  if(DEBUG>1) printf("\n");
		}
	    } 
	}
       	if(DEBUG>1) printf("\n"); 
        if(!params_valued[quiet_index])
	    {
	      printf(" %2i: Q=%1i  X=%1i  BCNT= %i  STAT=0x%8.8lx\n",i+1,q_bit,x_bit, camblk_tok_p->packet[i].wc_max*2,stat_long);
              if(params_valued[output_index])
		    fprintf(output_file," %2.2i: Q=%i  X=%i  BCNT= %i  STAT=0x%8.8lx\n",i+1,q_bit,x_bit, 
                           camblk_tok_p->packet[i].wc_max*2,stat_long);
	    }
    }
  return;

}

void verb_do_set_add(const char *token_p)
{
  strcpy(main_verb,token_p);
  if(DEBUG>1) printf("Main verb stored as %s \n",main_verb);
  if(strncmp(main_verb,"ADD",3)==0) 
   {
     camcom_packet_clean_sticky();
     current_packet++;
     if(DEBUG>1) printf("ADD seen, current packet now %d\n",current_packet);
   }

  return;
}

void token_noval()
{
  if ( current_index <  N_noval_index) params_noval[current_index]=1; 
  if(DEBUG>1) printf("Setting token %d on\n",current_index); 
  return;
}

void token_noval_no()
{
  if ( current_index <  N_noval_index )   params_noval[current_index]=0;
  if(DEBUG>1) printf("Setting token %d off\n",current_index);
  if ( current_index==noinput_index) params_valued[input_index]=0;
  if ( current_index==nooutput_index) 
     {
       if(params_valued[output_index])
	 { 
            fclose(output_file);
            printf("Data output file <%s> closed\n",params_values[output_index]);
	 }
        params_valued[output_index]=0; 
        params_values[output_index]=0;
     } 
if (current_index==nodebug_index) params_numeric[debug_index]=0; 
  if (current_index==noprompt_index) params_values[prompt_index]=0; 
  if (current_index==nodata_index) {params_values[data_index]=0;params_valued[data_index]=0;} 
  if (current_index==nobcnt_index) params_numeric[bcnt_index]=0; 
  if (current_index==nofill_index) {params_values[fill_index]=0; params_numeric[fill_index]=0;} 
  return ;
}

void help_type_param(const char* token_p)
   {
     if(!strncmp(token_p,"TYPE",4))
       {
         printf("%s\n",token_p+4);
       }
     if (!strncmp(token_p,"$",1))
      {
        printf("Executing [%s]\n",token_p+1);
        system(token_p+1);
      }
     if (!strncmp(token_p,"HELP",4))
       {
         printf("Executing man camcom.1\n");
         system("man $EPICS_EXTENSIONS/src/Camcom/camcom.1");
       }
     return;
     }

void camcom_sleep(int i)
{
  printf("Sleeping %d millisecond(s)\n",i);
  usleep(1000*i);
}

void camcom_sleep_num(char* token_p)
{
  current_index=sleep_index;
  token_with_number("SLEEP",token_p);  
  camcom_sleep(params_numeric[sleep_index]);
}

void camcom_sleep_param(char* token_p)
{
  current_index=sleep_index;
  token_with_parameter("SLEEP",token_p);
  camcom_sleep(params_numeric[sleep_index]);
}

void token_with_parameter(const char* token_p, const char *param_p)
{
  char param_char;
  int param_number;

  if(DEBUG>1) printf("Parameter <%s> found for index %d, modifier %s \n",
    param_p,current_index, token_p);

  param_char=*(param_p+2); param_number=param_char-'0';

  if (param_number > 0 )
    {
      param_number+=10*calling_level-1;  /* nested parameters */
      params_valued[current_index]=1;
      if(calling_param_values[param_number]==-99)
	{
          params_values[current_index]=strdup(calling_param_string[param_number]);
          if(DEBUG>1) printf("Value %s found for index %d, modifier %s \n",
	     params_values[current_index],current_index, token_p);
        }
      else
        {
          params_numeric[current_index]=calling_param_values[param_number];
          if(DEBUG>1) printf("Value %ld found for index %d, modifier %s \n",
	     params_numeric[current_index],current_index, token_p);
        }

      if(current_index==bcnt_index) packet_wcmax[current_packet]=params_numeric[bcnt_index]/2;
      if(current_index==crate_index)
	{
          if(params_numeric[crate_index]<MIN_CRATE_ADR) 
             {
               params_numeric[crate_index]=MIN_CRATE_ADR;
               printf("Changing crate to MINIMUM value of %ld\n",params_numeric[crate_index]);
	     }
          if(params_numeric[crate_index]>MAX_CRATE_ADR) 
             {
               params_numeric[crate_index]=MAX_CRATE_ADR;
               printf("Changing crate to MAXIMUM value of %ld\n",params_numeric[crate_index]);
	     }
	}
      if(current_index==module_index)
	{
          if(params_numeric[module_index]<MIN_CRATE_SLOT) 
             {
               params_numeric[module_index]=MIN_CRATE_SLOT;
               printf("Changing module to MINIMUM value of %ld\n",params_numeric[module_index]);
	     }
          if(params_numeric[module_index]>MAX_CRATE_SLOT) 
             {
               params_numeric[module_index]=MAX_CRATE_SLOT;
               printf("Changing module to MAXIMUM value of %ld\n",params_numeric[module_index]);
	     }
	}
    }

  return;
}

void get_data_if_needed()
{
  char temp_buffer[128]={0};
  FILE *data_input;
  int save_index;
  if(!write_fn) return;
  if(params_valued[data_index]==1) return;

  if(params_valued[input_index])
    {
      printf("\nInput specification found: %s \n",params_values[input_index]);
      data_input=fopen(params_values[input_index],"r");
      if(NULL != data_input)
        {
	  fgets(temp_buffer,120,data_input);
          fclose(data_input);
          printf("\n Data from %s = [%s]\n",params_values[input_index],temp_buffer);
        }
      else 
        {
          printf("\n File <%s> not found, prompting for data\n",params_values[input_index]);
          params_valued[input_index]=0;
	}
    }
  if(!params_valued[input_index])
    {
       if(params_noval[hex_index]==0) printf("\n DATA (Decimal) :");
       else                           printf("\n DATA (Hex) :");
       fgets(temp_buffer,120,stdin);
       temp_buffer[strlen(temp_buffer)-1]=0;
       printf("\n Data from prompt = [%s]\n",temp_buffer);
    }
  save_index=current_index;
  current_index=data_index;
  token_with_string("EXT DATA",temp_buffer);
  current_index=save_index;
}

void token_with_string(const char* token_p, const char *param_p)
{
  int i;
  if (current_index <= N_val_index )
    {
      params_valued[current_index]=1;
      params_values[current_index]=strdup(param_p);
      if(DEBUG>1) printf("Parameter %s found for index %d, modifier %s \n",
	 params_values[current_index],current_index, token_p);
    }
  if (current_index == output_index)
    {
      output_file=fopen(params_values[current_index],"w");
      if(NULL!=output_file)
        printf("File <%s> opened for data recording\n",params_values[current_index]);
      else
        {
          printf("Failed to open file %s for output\n",params_values[current_index]);
          params_valued[current_index]=0;
	}
    }
  if (current_index == data_index)
    {
      unpack_data(params_values[data_index],data_array); 
        for(i=0;i<params_numeric[data_index];i++)
	  {
            packet_data[current_packet][i+1]=data_array[i];
            if(DEBUG>1)
	      {
	         if(params_noval[hex_index]==0) printf("Decimal data element %d = %ld\n",i,data_array[i]);
	         else                           printf("Hex data element %d = 0x%lx\n",i,data_array[i]);
	      }
	  }
    }
  /* Check for BCNT specified */
  if(params_numeric[bcnt_index]==0)
    {
        packet_wcmax[current_packet]=params_numeric[data_index];
        if(DEBUG>1) 
            printf("Setting wcmax of packet %d to %d in TOKEN_WITH_STRING\n",current_packet,packet_wcmax[current_packet]);
    }
  return;
}

void token_with_number(const char* token_p, const char *param_p)
{
  char* end;
  int use_hex;
  char* local_p;
  local_p=param_p;
  if (current_index <= N_val_index)
    {
      params_valued[current_index]=1;

      if(params_noval[hex_index]) use_hex=1;
      else                        use_hex=0;
      if(strncmp(param_p, "%D",2)==0) {use_hex=0; local_p+=2;}
      if((strncmp(param_p,"0X",2)==0) || (strncmp(param_p,"%X",2)==0)) {use_hex=1; local_p+=2;}
      if(current_index==address_index) use_hex=0;

      if(use_hex) params_numeric[current_index]=strtol(local_p,&end,16);
      else        params_numeric[current_index]=strtol(local_p,&end,10);

      if(DEBUG>1) printf("Value %ld found for index %d, modifier %s \n",
	 params_numeric[current_index],current_index, token_p);
      if(current_index==bcnt_index) packet_wcmax[current_packet]=params_numeric[bcnt_index]/2;
      if(current_index==crate_index)
	{
          if(params_numeric[crate_index]<MIN_CRATE_ADR) 
             {
               params_numeric[crate_index]=MIN_CRATE_ADR;
               printf("Changing crate to MINIMUM value of %ld\n",params_numeric[crate_index]);
	     }
          if(params_numeric[crate_index]>MAX_CRATE_ADR) 
             {
               params_numeric[crate_index]=MAX_CRATE_ADR;
               printf("Changing crate to MAXIMUM value of %ld\n",params_numeric[crate_index]);
	     }
	}
      if(current_index==module_index)
	{
          if(params_numeric[module_index]<MIN_CRATE_SLOT) 
             {
               params_numeric[module_index]=MIN_CRATE_SLOT;
               printf("Changing module to MINIMUM value of %ld\n",params_numeric[module_index]);
	     }
          if(params_numeric[module_index]>MAX_CRATE_SLOT) 
             {
               params_numeric[module_index]=MAX_CRATE_SLOT;
               printf("Changing module to MAXIMUM value of %ld\n",params_numeric[module_index]);
	     }
	}
    }
  return;
}

void unpack_data (const char *ptr,unsigned long out_array[67])
{
  int array_index=0;
  char ch;
  char* local_copy;
  char* cho;
  char* end;
  int local_hex;
  int local_dec;

/*--------------------------------------------------------------------------*/

  local_copy=strdup(ptr);
  ch = *local_copy;
    while (ch == ' ' || ch == '\t')
        ch = *(++local_copy);
    cho = local_copy;
    out_array[0]=params_numeric[data_index]=0;
    /* Check for 0x preceeding number */
    if( (strncmp(local_copy,"0X",2)==0) || (strncmp(local_copy,"%X",2)==0) )
      {
        local_hex=1;
        local_copy+=2;
        cho=local_copy;
	if(DEBUG>1) printf("Found 0x or percent-X prefix\n");
      }
    else local_hex=0;

    if( strncmp(local_copy,"%D",2)==0 )
      {
        local_dec=1;
        local_copy+=2;
        cho=local_copy;
	if(DEBUG>1) printf("Found percent-D prefix\n");
      }
    else local_dec=0;

    for (;;) 
   {
      if (ch == 0 || ch == ',')
        {
          *(local_copy)=0;
	  if(DEBUG>1) printf("One entry for %d = %s\n",array_index,cho);  
	  if (local_hex) out_array[array_index++]=strtol(cho,&end,16);
	  if (local_dec) out_array[array_index++]=strtol(cho,&end,10);
          if (!local_dec&&!local_hex&&params_noval[hex_index]) out_array[array_index++]=strtol(cho,&end,16);
          if (!local_dec&&!local_hex&&!params_noval[hex_index]) out_array[array_index++]=strtol(cho,&end,10);

          params_numeric[data_index]++; /* number of data elements */
          out_array[array_index]=0; 
	  cho=local_copy+1;

       if(DEBUG>1) 
         printf("Data value %d = %ld, or 0x%lx\n",array_index,out_array[array_index-1],out_array[array_index-1]); 

    /* Check for 0x preceeding number */

       local_copy++;
       if((strncmp(local_copy,"0X",2)==0) || (strncmp(local_copy,"%X",2)==0))
         {
           local_hex=1;
           local_copy+=2;
           cho=local_copy;
	   if(DEBUG>1) printf("Found 0x or percent-X prefix after first\n");
         }
       else local_hex=0;

       if(strncmp(local_copy,"%D",2)==0)
         {
           local_dec=1;
           local_copy+=2;
           cho=local_copy;
	   if(DEBUG>1)  printf("Found percent-D prefix after first\n");
         }
       else local_dec=0;

	}
	if(ch == 0) return;

       ch = *(++local_copy);

   }
    if(params_numeric[bcnt_index]==0)
      {
      packet_wcmax[current_packet]=params_numeric[data_index];
      if(DEBUG>1) printf("WCMAX for packet %d set to %ld in UNPACK\n",current_packet,params_numeric[data_index]);
      }
}

void set_calling_level(int level)
{
  calling_level=level;
  return;
}

int get_calling_level()
{
  return calling_level;
}




