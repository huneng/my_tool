#include <time.h>

#include <stdio.h>

void make_timer(time_t *st, time_t *et){
    *st = time(NULL);

    struct tm *ts = localtime(st);


#if 0
    ts->tm_min += 2;
#else
    ts->tm_mon += 6;
    if(ts->tm_mon > 11){
        ts->tm_year ++;
        ts->tm_mon %= 12;
    }

    ts->tm_hour += 1;

#endif

    *et = mktime(ts);
}


time_t make_timer(){
    time_t rawtime;
    int year, month ,day;
    struct tm *timeinfo;

    const char * weekday[] = { "Sunday", "Monday",
        "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"};

    int ret;

    /* prompt user for date */
    printf ("Enter year: "); fflush(stdout);
    ret = scanf ("%d",&year);

    printf ("Enter month: "); fflush(stdout);
    ret = scanf ("%d",&month);

    printf ("Enter day: "); fflush(stdout);
    ret = scanf ("%d",&day);

    /* get current timeinfo and modify it to the user's choice */
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    timeinfo->tm_year = year - 1900;
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;

    /* call mktime: timeinfo->tm_wday will be set */
    return mktime ( timeinfo );
}



int main(int argc, char **argv){
    time_t st, et;

    //make_timer(&st, &et);
    st = time(NULL);
    et = make_timer();

    printf("%ld\n%ld\n", st, et);
    return 0;
}


