#include <stdio.h>
#include <string.h>

#include "libcue.h"
#include "minunit.h"

int tests_run;

/* Frames per second */
#define FPS (75)
#define MSF_TO_F(m,s,f) ((f) + ((m)*60 + (s))*FPS)

static char cue[] =
"PERFORMER \"Bloc Party\"\n"
"TITLE \"Silent Alarm\"\n"
"FILE \"Bloc Party - Silent Alarm.flac\" WAVE\n"
   "TRACK 01 AUDIO\n"
      "TITLE \"Like Eating Glass\"\n"
      "PERFORMER \"Bloc Party\"\n"
      "INDEX 00 00:00:00\n"
      "INDEX 01 03:22:70\n"
   "TRACK 02 AUDIO\n"
      "TITLE \"Helicopter\"\n"
      "PERFORMER \"Bloc Party\"\n"
      "INDEX 00 07:42:69\n"
      "INDEX 01 07:44:69\n";

static char* cue_test()
{
   Cd *cd = cue_parse_string (cue);
   mu_assert ("error parsing CUE", cd != NULL);

   Rem *rem = cd_get_rem (cd);
   mu_assert ("error getting REM", rem != NULL);

   Cdtext *cdtext = cd_get_cdtext (cd);
   mu_assert ("error getting CDTEXT", cdtext != NULL);

   const char *val;
   val = cdtext_get (PTI_PERFORMER, cdtext);
   mu_assert ("error getting CD performer", val != NULL);
   mu_assert ("error validating CD performer", strcmp (val, "Bloc Party") == 0);

   val = cdtext_get (PTI_TITLE, cdtext);
   mu_assert ("error getting CD title", val != NULL);
   mu_assert ("error validating CD title", strcmp (val, "Silent Alarm") == 0);

   int ival = cd_get_ntrack (cd);
   mu_assert ("invalid number of tracks", ival == 2);

   Track *track;
   /* Track 1 */
   track = cd_get_track (cd, 1);
   mu_assert ("error getting track", track != NULL);

   val = track_get_filename (track);
   mu_assert ("error getting track filename", val != NULL);
   mu_assert ("error validating track filename", strcmp (val, "Bloc Party - Silent Alarm.flac") == 0);

   cdtext = track_get_cdtext (track);
   mu_assert ("error getting track CDTEXT", cdtext != NULL);

   val = cdtext_get (PTI_PERFORMER, cdtext);
   mu_assert ("error getting track performer", val != NULL);
   mu_assert ("error validating track performer", strcmp (val, "Bloc Party") == 0);

   val = cdtext_get (PTI_TITLE, cdtext);
   mu_assert ("error getting track title", val != NULL);
   mu_assert ("error validating track title", strcmp (val, "Like Eating Glass") == 0);

   ival = track_get_zero_pre (track);
   mu_assert ("invalid track pre-gap", ival == MSF_TO_F(3,22,70));
   ival = track_get_start (track);
   mu_assert ("invalid track start", ival == MSF_TO_F(3,22,70));
   ival = track_get_length (track);
   mu_assert ("invalid track length", ival == MSF_TO_F(4,19,74));

   ival = track_get_index (track, 0);
   mu_assert ("invalid index", ival == 0);
   ival = track_get_index (track, 1);
   mu_assert ("invalid index", ival == MSF_TO_F(3,22,70));

   /* Track 2 */
   track = cd_get_track (cd, 2);
   mu_assert ("error getting track", track != NULL);

   val = track_get_filename (track);
   mu_assert ("error getting track filename", val != NULL);
   mu_assert ("error validating track filename", strcmp (val, "Bloc Party - Silent Alarm.flac") == 0);

   cdtext = track_get_cdtext (track);
   mu_assert ("error getting track CDTEXT", cdtext != NULL);

   val = cdtext_get (PTI_PERFORMER, cdtext);
   mu_assert ("error getting track performer", val != NULL);
   mu_assert ("error validating track performer", strcmp (val, "Bloc Party") == 0);

   val = cdtext_get (PTI_TITLE, cdtext);
   mu_assert ("error getting track title", val != NULL);
   mu_assert ("error validating track title", strcmp (val, "Helicopter") == 0);

   ival = track_get_zero_pre (track);
   mu_assert ("invalid track pre-gap", ival == MSF_TO_F(0,2,0));
   ival = track_get_start (track);
   mu_assert ("invalid track start", ival == MSF_TO_F(7,44,69));
   ival = track_get_length (track);
   mu_assert ("invalid track length", ival == -1);

   ival = track_get_index (track, 0);
   mu_assert ("invalid index", ival == MSF_TO_F(7,42,69));
   ival = track_get_index (track, 1);
   mu_assert ("invalid index", ival == MSF_TO_F(7,44,69));

   cd_delete (cd);

   return NULL;
}

static char* run_tests()
{
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
