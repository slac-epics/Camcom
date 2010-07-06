/*

Abs: CAMCOM support function prototypes

Name: camcom_proto.h

Auth: 26-Jan-2010  R. Chestnut

*/

#ifndef camcom_proto_h

long debug_level();

void token_with_number(const char *token_p, const char *number_p);

void token_with_parameter(const char *token_p, const char *param_p);

int get_parameter_value(const char* param_p);

void put_parameter_value(int param_index, int param_value);

void token_with_string(const char *token_p, const char *string_p);

void print_token_table();

void store_index(int);

void issue_prompt(int);

void token_noval ();

void token_noval_no ();

void unpack_data (const char *ptr,unsigned long out_array[10]);

void verb_do_set_add(const char *token_p);

void prepare_real_packet();

void ca_activity();

long show_reset_exit(const char *token_p);

long control_word_getter();

long verb_help (const char *topic_p);

void help_type_param (const char *topic_p);

long verb_other(const char *token_p);

long modifier_forever();

void setup_whole_packet();

void dump_packet();

void camcom_packet_clean_all();

void camcom_packet_clean_sticky();

long camcom_process();

void set_calling_level(const int);

int get_calling_level();

void get_data_if_needed();

void process_returned_data();

void camcom_sleep(int);

void camcom_sleep_num(char*);

void camcom_sleep_param(char*);

#define camcom_proto_h
#endif
