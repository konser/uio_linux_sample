/*
 * added and modified by LiMing Wu (19092205@suning.com) 
 * 
 * example: inteval is 1 count 10 thread-id is $tid
 * 	./get_rundelay 1 10 $tid
 * 	thread-id is mapping to a vcpu
 *
 * schedstats also adds a new /proc/<pid>/schedstat file to include some of
 * the same information on a per-process level.  There are three fields in
 * this file correlating for that process to:
 *      1) time spent on the cpu
 *      2) time spent waiting on a runqueue
 *      3) # of timeslices run on this cpu
 * base on http://eaglet.pdxhosts.com/rick/linux/schedstat/
 */
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

extern char *index(), *rindex();
char procbuf[512];
char statname[64];
char *Progname;
FILE *fp;

void usage()
{
    fprintf(stderr,"Usage: %s [-s sleeptime ] [-c count] <pid>\n", Progname);
    exit(-1);
}

/*
 * get_stats() -- we presume that we are interested in the first three
 *  fields of the line we are handed, and further, that they contain
 *  only numbers and single spaces.
 */
void get_stats(char *buf, unsigned long long *run_ticks, unsigned long long *wait_ticks,
    unsigned long long *nran)
{
    char *ptr;

    /* sanity */
    if (!buf || !run_ticks || !wait_ticks || !nran)
    return;

    /* leading blanks */
    ptr = buf;
    while (*ptr && isblank(*ptr))
    ptr++;

    /* first number -- run_ticks */
    *run_ticks = atoll(ptr);
    while (*ptr && isdigit(*ptr))
    ptr++;
    while (*ptr && isblank(*ptr))
    ptr++;

    /* second number -- wait_ticks */
    *wait_ticks = atoll(ptr);
    while (*ptr && isdigit(*ptr))
    ptr++;
    while (*ptr && isblank(*ptr))
    ptr++;

    /* last number -- nran */
    *nran = atoll(ptr);
}

/*
 * get_id() -- extract the id field from that /proc/<pid>/stat file
 */
void get_id(char *buf, char *id)

{
    char *ptr;

    /* sanity */
    if (!buf || !id)
    return;

    ptr = index(buf, ')') + 1;
    *ptr = 0;
    strcpy(id, buf);
    *ptr = ' ';

    return;
}

main(int argc, char *argv[])
{
    int c;
    unsigned int sleeptime = 1, count=10, pid = 0, verbose = 0;
    char id[32];
    unsigned long long run_ticks, wait_ticks, nran;
    unsigned long long first_orun_ticks=0, orun_ticks=0, first_owait_ticks=0, owait_ticks=0, oran=0, i=0;

    Progname = argv[0];
    id[0] = 0;
    while ((c = getopt(argc,argv,"s:c:v")) != -1) {
    switch (c) {
        case 's':
        sleeptime = atoi(optarg);
        break;
        case 'c':
        count = atoi(optarg);
        break;
        case 'v':
        verbose++;
        break;
        default:
        usage();
    }
    }
    printf("interval is %d count is %d \n", sleeptime, count);
    if (optind < argc) {
    pid = atoi(argv[optind]);
    }

    if (!pid)
    usage();

    sprintf(statname,"/proc/%d/stat", pid);
    if (fp = fopen(statname, "r")) {
    if (fgets(procbuf, sizeof(procbuf), fp))
        get_id(procbuf,id);
        fclose(fp);
    }

    /*
     * now just spin collecting the stats
     */
    sprintf(statname,"/proc/%d/schedstat", pid);
    while (fp = fopen(statname, "r")) {
        for (i=0; i<count+1; i++) {
        if (!fgets(procbuf, sizeof(procbuf), fp))
            break;

        get_stats(procbuf, &run_ticks, &wait_ticks, &nran);
        if (orun_ticks) {
            if (verbose)
            printf("%s %d(%d) %d(%d) %d(%d) avgrun=%4.2f avgwait%4.2f\n",
                id, run_ticks, run_ticks - orun_ticks,
                wait_ticks, wait_ticks - owait_ticks,
                nran, nran - oran,
                nran - oran ?
                (double)(run_ticks-orun_ticks)/(nran - oran) : 0,
                nran - oran ?
                (double)(wait_ticks-owait_ticks)/(nran - oran) : 0);
            else
            printf("%s deltarun=%4.2fms deltawait=%4.2fms deltawait/interval=%4.3f\n",
                id,
                (double)(run_ticks-orun_ticks)/1000000,
                (double)(wait_ticks-owait_ticks)/1000000,
                (double)(wait_ticks-owait_ticks)/(sleeptime * 1000000000));
            if (i == count) {
            printf("----------------------------averag-----------------------------------\n"); 
            long long unsigned delta = wait_ticks-first_owait_ticks;
            printf("%s deltarun=%4.2fms deltawait=%4.2fms deltawait/elaspe=%4.2f%\n",
                id,
                (double)(run_ticks-first_orun_ticks)/1000000,
                (double)(wait_ticks-first_owait_ticks)/1000000,
                (double)(wait_ticks-first_owait_ticks)/(sleeptime * 10000000 * count));
            }
        } 
        fclose(fp);
        oran = nran;
        orun_ticks = run_ticks;
        owait_ticks = wait_ticks;
        if (i == 0) {
            first_orun_ticks = run_ticks;
            first_owait_ticks = wait_ticks;
        }
        sleep(sleeptime);
        fp = fopen(statname,"r");
        if (!fp)
            break;
        }
        exit(0);
    }
    if (id[0])
    printf("Process %s has exited.\n", id);
    else 
    printf("Process %d does not exist.\n", pid);
    exit(0);
}
