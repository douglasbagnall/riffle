#include "ccan/opt/opt.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned long option_bytes;
static unsigned long option_seed;

static struct opt_table opts[] = {
	OPT_WITH_ARG("--bytes|-b", 
		     opt_set_ulongval, opt_show_ulongval, 
		     &option_bytes, "Emit around this many bytes (accepts k, M suffixes; default: inf)."),
	OPT_WITH_ARG("--seed|-s", 
		     opt_set_ulongval, opt_show_ulongval, 
		     &option_seed, "seed (32 bits)"),
  	OPT_WITHOUT_ARG("--usage|--help|-h", opt_usage_and_exit,
  			"args...\nEmit random bytes",
 			"Print this message."),
	OPT_ENDTABLE
};

void parse_args(int argc, char *argv[])
{
	opt_register_table(opts, NULL);
	if (!opt_parse(&argc, argv, opt_log_stderr))
		exit(1);
	return;
}
