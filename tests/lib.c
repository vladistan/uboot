#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <mockio.h>


struct {
  void * cmd_usage;


} verify;

/* LIFTED FROM  ../tools/updater/utils.c ... check with the source once in a while */
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
        unsigned long result = 0,value;

        if (*cp == '0') {
                cp++;
                if ((*cp == 'x') && isxdigit(cp[1])) {
                        base = 16;
                        cp++;
                }
                if (!base) {
                        base = 8;
                }
        }
        if (!base) {
                base = 10;
        }
        while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
            ? toupper(*cp) : *cp)-'A'+10) < base) {
                result = result*base + value;
                cp++;
        }
        if (endp)
                *endp = (char *)cp;
        return result;
}



/** TEST HELPERS **/

int cmd_usage(void * p )
{
   verify.cmd_usage = p;
   return 0;
}




void reset_verify()
{
   memset(&verify,0,sizeof(verify));
}


int verify_cmd_usage(void * p )
{
  return verify.cmd_usage == p;

}




