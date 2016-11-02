#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "libcue.h"
#include "minunit.h"

int tests_run;

static char* cue_test()
{
   FILE *cue = fopen("issue10.cue", "r");
   assert(cue);
   Cd *cd = cue_parse_file(cue);
   assert(cd);
   cd_delete(cd);

   return NULL;
}

static char* run_tests()
{
   mu_run_test (cue_test);
   mu_run_test (cue_test);
   return NULL;
}

int main (int argc, char **argv)
{
   char *result = run_tests();
   if (result != NULL)
      printf ("%s\n", result);
   else
      printf ("All tests passed!\n");

   printf ("Tests run: %d\n", tests_run);

   return result != NULL;
}
