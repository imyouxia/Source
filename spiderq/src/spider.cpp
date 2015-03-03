#include <stdio.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "spider.h"
#include "threads.h"
#include "qstring.h"

int g_epfd;
Config *g_conf;
extern int g_cur_thread_num;

static int set_nofile(rlim_t limit);
static void daemonize();
static void stat(int sig);
static int set_ticker(int second);

static void version()
{
    printf("Version: spiderq/1.0 by qteqpid\n");
    exit(1);
}

static void usage()
{
    printf("Usage: ./spider [Options]\n"
            "\nOptions:\n"
            "  -h\t: this help\n"
            "  -v\t: print spiderq's version\n"
            "  -d\t: run program as a daemon process\n\n");
    exit(1);
}

int main(int argc, void *argv[]) 
{
    struct epoll_event events[10];
    int daemonized = 0;
    char ch;

    /* parse opt */
    while ((ch = getopt(argc, (char* const*)argv, "vhd")) != -1) {
        switch(ch) {
            case 'v':
                version();
                break;
            case 'd':
                daemonized = 1;
                break;
            case 'h':
            case '?':
            default:
                usage();
        }
    }

    /* parse log */
    g_conf = initconfig();
    loadconfig(g_conf);

    /* set max value of fd num to 1024 */
    set_nofile(1024); 

    /* load all modules */
    vector<char *>::iterator it = g_conf->modules.begin();
    for(; it != g_conf->modules.end(); it++) {
        dso_load(g_conf->module_path, *it); 
    } 

    /* add seeds */
    if (g_conf->seeds == NULL) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "We have no seeds, Buddy!");
    } else {
        int c = 0;
        char ** splits = strsplit(g_conf->seeds, ',', &c, 0);
        while (c--) {
            Surl * surl = (Surl *)malloc(sizeof(Surl));
            surl->url = url_normalized(strdup(splits[c]));
            surl->level = 0;
            surl->type = TYPE_HTML;
            if (surl->url != NULL)
                push_surlqueue(surl);
        }
    }	

    /* daemonized if setted */
    if (daemonized)
        daemonize();

    /* change wd to download directory */
    chdir("download"); 

    /* create a thread for DNS parsing and parse seed surl to ourl */
    int err = -1;
    if ((err = create_thread(urlparser, NULL, NULL, NULL)) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Create urlparser thread fail: %s", strerror(err));
    }

    /* waiting seed ourl ready */
    int try_num = 1;
    while(try_num < 8 && is_ourlqueue_empty())
        usleep((10000 << try_num++));

    if (try_num >= 8) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "NO ourl! DNS parse error?");
    }

    /* set ticker  */
    if (g_conf->stat_interval > 0) {
        signal(SIGALRM, stat);
        set_ticker(g_conf->stat_interval);
    }

    /* begin create epoll to run */
    int ourl_num = 0;
    g_epfd = epoll_create(g_conf->max_job_num);

    while(ourl_num++ < g_conf->max_job_num) {
        if (attach_epoll_task() < 0)
            break;
    }

    /* epoll wait */
    int n, i;
    while(1) {
        n = epoll_wait(g_epfd, events, 10, 2000);
        printf("epoll:%d\n",n);
        if (n == -1)
            printf("epoll errno:%s\n",strerror(errno));
        fflush(stdout);

        if (n <= 0) {
            if (g_cur_thread_num <= 0 && is_ourlqueue_empty() && is_surlqueue_empty()) {
                sleep(1);
                if (g_cur_thread_num <= 0 && is_ourlqueue_empty() && is_surlqueue_empty())
                    break;
            }
        }

        for (i = 0; i < n; i++) {
            evso_arg * arg = (evso_arg *)(events[i].data.ptr);
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                SPIDER_LOG(SPIDER_LEVEL_WARN, "epoll fail, close socket %d",arg->fd);
                close(arg->fd);
                continue;
            }
            /* I don't know why epoll_wait return the same socket event multi times
             * so I have to del event here
             */
            epoll_ctl(g_epfd, EPOLL_CTL_DEL, arg->fd, &events[i]); /* del event */

            printf("hello epoll:event=%d\n",events[i].events);
            fflush(stdout);
            create_thread(recv_response, arg, NULL, NULL);
        }
    }

    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Task done!");
    close(g_epfd);
    return 0;
}

int attach_epoll_task()
{
    struct epoll_event ev;
    int sock_rv;
    int sockfd;
    Url * ourl = pop_ourlqueue();
    if (ourl == NULL) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Pop ourlqueue fail!");
        return -1;
    }

    /* connect socket and get sockfd */
    if ((sock_rv = build_connect(&sockfd, ourl->ip, ourl->port)) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Build socket connect fail: %s", ourl->ip);
        return -1;
    }

    set_nonblocking(sockfd);

    if ((sock_rv = send_request(sockfd, ourl)) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Send socket request fail: %s", ourl->ip);
        return -1;
    } 

    evso_arg * arg = (evso_arg *)calloc(1, sizeof(evso_arg));
    arg->fd = sockfd;
    arg->url = ourl;
    ev.data.ptr = arg;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(g_epfd, EPOLL_CTL_ADD, sockfd, &ev) == 0) {/* add event */
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Attach an epoll event success!");
    } else {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Attach an epoll event fail!");
        return -1;
    }

    g_cur_thread_num++; 
    return 0;
}

// 控制系统资源的最大使用量
static int set_nofile(rlim_t limit)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "getrlimit fail");
        return -1;
    }
    if (limit > rl.rlim_max) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "limit should NOT be greater than %lu", rl.rlim_max);
        return -1;
    }
    rl.rlim_cur = limit;
    if (setrlimit(RLIMIT_NOFILE, &rl) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "setrlimit fail");
        return -1;
    }
    return 0;
}

// 守护进程
static void daemonize()
{
    int fd;
    if (fork() != 0) exit(0);
    setsid();
    SPIDER_LOG(SPIDER_LEVEL_INFO, "Daemonized...pid=%d", (int)getpid());	

    /* redirect stdin|stdout|stderr to /dev/null */
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }

    /* redirect stdout to logfile */
    if (g_conf->logfile != NULL && (fd = open(g_conf->logfile, O_RDWR | O_APPEND | O_CREAT, 0)) != -1) {
        dup2(fd, STDOUT_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }

}

static int set_ticker(int second)
{
    struct itimerval itimer;
    itimer.it_interval.tv_sec = (long)second;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = (long)second;
    itimer.it_value.tv_usec = 0;

    return setitimer(ITIMER_REAL, &itimer, NULL);
}

static void stat(int sig)
{
    SPIDER_LOG(SPIDER_LEVEL_DEBUG, 
            "cur_thread_num=%d\tsurl_num=%d\tourl_num=%d",
            g_cur_thread_num,
            get_surl_queue_size(),
            get_ourl_queue_size());
}
