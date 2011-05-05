/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Part of Riffle, a collection of random number generators
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", WITHOUT WARRANTY of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. in no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in
 * the Software.
 */
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
