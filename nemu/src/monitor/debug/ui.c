#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  /* Added, implementation not finished*/
  {"si", "Execution step by step", cmd_si},
  {"info", "Show status", cmd_info},
  {"p", "Expression value", cmd_p},
  {"x", "Scan memory", cmd_x},
  //{"w", "Set watchpoint", cmd_w},
  //{"d", "Delete watchpoint", cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  /*extract the first argument*/
  char *arg = strtok(NULL, "");
  if (arg == NULL) {
    cpu_exec(1);	//no argument given
  }
  else {
    int count = atoi(arg);
    cpu_exec(count);
  }
  return 0;
} 

static int cmd_info(char *args){
  /*extract the first argument*/
  char *arg = strtok(NULL, "");
  if (arg == NULL) {
    printf("Wrong argument!\n");
    return 0;
  }
  else if (!strcmp(arg, "r")) {
    printf("Register Status:\n");
    for(int i = R_EAX; i <= R_EDI; i++){
      printf("%s\t\t0x%x\n", reg_name(i, 4), reg_l(i));
    }
    printf("eip\t\t0x%x\n", cpu.eip);
    return 0;
  }
  else if (!strcmp(arg, "w")) {
    /*TODO: Add watchpoints*/
    printf("Unimplemented.\n");
    return 0;
  }
  else {
    printf("Wrong argument!\n");
    return 0;
  }
}

static int cmd_p(char *args){
  bool success;
  uint32_t res = expr(args, &success);
  if(success){
    printf("%u\n", res);  
  }
  printf("Invalid input!\n");
  return 0;
}

static int cmd_x(char *args){
  /*char *arg1 = strtok(NULL, " ");
  char *arg2 = arg1 + strlen(arg1) + 1;
 // int num = atoi(arg1);
  bool success;
 // vaddr_t addr = expr(arg2, &success);
  if(!success){
    printf("Invalid input!\n");
	return 0;
  }
  for(int i = 0; i < num; i++){
    vaddr_t addr_temp = addr + 4 * i;
	uint32_t data = vaddr_read(addr_temp, 4);
  }*/
  return 0;
}


void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
