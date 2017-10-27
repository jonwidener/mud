#ifndef PROMPTS_H_
#define PROMPTS_H_

typedef void (*func_ptr)(void*);
typedef func_ptr (*prompt_type)(char*);

func_ptr prompt_enter_name(char*);
func_ptr prompt_verify_name(char*);
func_ptr prompt_enter_password(char*);
func_ptr prompt_create_password(char*);
func_ptr prompt_verify_password(char*);


#endif