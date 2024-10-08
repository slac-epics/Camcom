/*

Abs: CAMCOM support function prototypes

Name: camcom_proto.h

Auth: 26-Jan-2010  R. Chestnut

*/

#ifndef camcom_proto_h

int debug_level();

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

void unpack_data (const char *ptr);

void verb_do_set_add(const char *token_p);

void prepare_real_packet();

void ca_activity();

void check_for_action();

int show_reset_dump(const char *token_p);

int control_word_getter();

int verb_help (const char *topic_p);

void help_type_param (const char *topic_p);

int verb_other(const char *token_p);

int modifier_forever();

void setup_whole_packet();

void dump_packet();

void camcom_packet_clean_all();

void camcom_packet_clean_sticky();

int camcom_process();

void set_calling_level(const int);

int get_calling_level();

void get_data_if_needed();

void process_returned_data();

void camcom_sleep(int);

void camcom_sleep_num(char*);

void camcom_sleep_param(char*);

#define camcom_proto_h
#endif
