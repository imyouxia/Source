/*
 * (C) Radim Kolar 1997-2004
 * This is free software, see GNU Public License version 2 for
 * details.
 *
 * Simple forking WWW Server benchmark:
 *
 * Usage:
 *   webbench --help
 *
 * Return codes:
 *    0 - sucess
 *    1 - benchmark failed (server is not on-line)
 *    2 - bad param
 *    3 - internal error, fork failed
 * 
 */ 
#include "socket.c"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* values */
// 判断压力测试是否到达设定时间
volatile int timerexpired=0;
int speed=0; // 记录进程成功得到服务器相应的数量，即成功数
int failed=0; // 记录失败的数目
int bytes=0;  // 记录进程成功读取的字节数
/* globals */
// HTTP协议版本，默认为HTTP 1.0
int http10=1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
/* Allow: GET, HEAD, OPTIONS, TRACE */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"
// HTTP 默认请求方式为GET，也支持HEAD，OPTION，TRACE
int method=METHOD_GET;
int clients=1; // 并发数目，-c N 参数N表示并发数，默认为1个进程
int force=0;   // 是否等待读取从Server返回的数据，0表示要等待读取
int force_reload=0;  // 是否使用缓存，1表示不缓存，0表示缓存
int proxyport=80;    // (代理)服务器端口
char *proxyhost=NULL;  // 代理服务器地址
int benchtime=30;   // 压力测试时间，通过-t参数设置，默认为30s
/* internal */
// 管道，用于父子进程通信
int mypipe[2];
char host[MAXHOSTNAMELEN]; // 目标主机地址 
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];   // 发送的构造的HTTP请求
 
// 长选项，getopt_long的参数 
static const struct option long_options[]=
{
 {"force",no_argument,&force,1},
 {"reload",no_argument,&force_reload,1},
 {"time",required_argument,NULL,'t'},
 {"help",no_argument,NULL,'?'},
 {"http09",no_argument,NULL,'9'},
 {"http10",no_argument,NULL,'1'},
 {"http11",no_argument,NULL,'2'},
 {"get",no_argument,&method,METHOD_GET},
 {"head",no_argument,&method,METHOD_HEAD},
 {"options",no_argument,&method,METHOD_OPTIONS},
 {"trace",no_argument,&method,METHOD_TRACE},
 {"version",no_argument,NULL,'V'},
 {"proxy",required_argument,NULL,'p'},
 {"clients",required_argument,NULL,'c'},
 {NULL,0,NULL,0}
};

/* prototypes */
static void benchcore(const char* host,const int port, const char *request);
static int bench(void);
static void build_request(const char *url);

static void alarm_handler(int signal)
{
    timerexpired=1;
}	

static void usage(void)
{
    fprintf(stderr,
	"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
}

int main(int argc, char *argv[])
{
    // getopt_long 的返回字符
    int opt=0;
    // getopt_long 的第五个参数，一般为0
    int options_index=0;
    char *tmp=NULL;

    if(argc==1)
    {
	    usage();
        return 2;
    } 
    
    // 依次读取命令行参数，并通过optarg返回值赋值
    while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?h",long_options,&options_index))!=EOF )
    {
        switch(opt)
        {
            case  0 : break;
            case 'f': force=1;break;
            case 'r': force_reload=1;break; 
            case '9': http10=0;break;
            case '1': http10=1;break;
            case '2': http10=2;break;
            case 'V': printf(PROGRAM_VERSION"\n");exit(0);
            // -t 后跟压力测试时间，optarg返回，使用atoi转换成整数
            case 't': benchtime=atoi(optarg);break;	     
            case 'p': 
	        /* proxy server parsing server:port */
	        tmp=strrchr(optarg,':'); // server:Port
            proxyhost=optarg;
	        if(tmp==NULL)
	        {
		         break;
	        }
	        if(tmp==optarg)
	        {
		        fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
		        return 2;
	        }
	        if(tmp==optarg+strlen(optarg)-1)
	        {
		        fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
		        return 2;
	        }
	        *tmp='\0'; // 获得代理地址
	        proxyport=atoi(tmp+1);break; // 获得代理端口
            
            case ':':
            case 'h':
            case '?': usage();return 2;break;
            // 并发数目 -c N
            case 'c': clients=atoi(optarg);break;
        }
    }

    // optind 返回第一个不包含选项的命令行参数，此处为URL值 
    if(optind==argc) 
    {
        fprintf(stderr,"webbench: Missing URL!\n");
        usage();
        return 2;
    }

    // 此处多做一次判断，可预防BUG，因为上文并发数目用户可能写0
    if(clients==0) clients=1;
    // 压力测试时间默认为30S，如果用户写成0，则默认为60S
    if(benchtime==0) benchtime=60;
    /* Copyright */
    fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
	 "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n"
	 );
    // 构造HTTP请求头
    build_request(argv[optind]); // 参数为URL值
    /* print bench info */
    printf("\nBenchmarking: ");
    
    /*
     * 以下打印压力测试各种参数信息，如HTTP协议，请求方式，并发个数，请求时间等。
     */
    
    switch(method)
    {
        case METHOD_GET:
        default:
            printf("GET");break;
        case METHOD_OPTIONS:
            printf("OPTIONS");break;
        case METHOD_HEAD:
            printf("HEAD");break;
	    case METHOD_TRACE:
            printf("TRACE");break;
    }
    printf(" %s",argv[optind]);
 
    switch(http10)
    {
        case 0: printf(" (using HTTP/0.9)");break;
        case 2: printf(" (using HTTP/1.1)");break;
    }
    printf("\n");
    if(clients==1) printf("1 client");
    else
        printf("%d clients",clients);

    printf(", running %d sec", benchtime);
    if(force) printf(", early socket close");
    if(proxyhost!=NULL) printf(", via proxy server %s:%d",proxyhost,proxyport);
    if(force_reload) printf(", forcing reload");
    printf(".\n");
    // bench() 压力测试的核心代码
    return bench();
}

// 构造HTTP请求头
void build_request(const char *url)
{
    char tmp[10];
    int i;

    bzero(host,MAXHOSTNAMELEN);
    bzero(request,REQUEST_SIZE);

    if(force_reload && proxyhost!=NULL && http10<1) http10=1;
    if(method==METHOD_HEAD && http10<1) http10=1;
    if(method==METHOD_OPTIONS && http10<2) http10=2;
    if(method==METHOD_TRACE && http10<2) http10=2;

    switch(method)
    {
        default:
        case METHOD_GET: strcpy(request,"GET");break;
        case METHOD_HEAD: strcpy(request,"HEAD");break;
        // 该请求方法的相应不能缓存
        case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
        case METHOD_TRACE: strcpy(request,"TRACE");break;
    }
		  
    strcat(request," ");

    if(NULL==strstr(url,"://"))
    {
        fprintf(stderr, "\n%s: is not a valid URL.\n",url);
        exit(2);
    }
  
    if(strlen(url)>1500)
    {
        fprintf(stderr,"URL is too long.\n");
        exit(2);
    }
    
    if(proxyhost==NULL)
        if (0!=strncasecmp("http://",url,7)) // 未使用代理服务器的情况下，只允许HTTP协议 
        { 
            fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
            exit(2);
        }
    /* protocol/host delimiter */
    // 指向"://"后的第一个字母
    i=strstr(url,"://")-url+3;
    /* printf("%d\n",i); */
    // URL后必须得'/'
    if(strchr(url+i,'/')==NULL) 
    {
        fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
        exit(2);            
    }
    // 如果未使用代理服务器，就表示肯定是HTTP协议
    if(proxyhost==NULL)
    {
        /* get port from hostname */
        // 如果是server : port 形式，解析主机和端口
        if(index(url+i,':')!=NULL &&
                index(url+i,':')<index(url+i,'/'))
        {
            // 获取主机地址
            strncpy(host,url+i,strchr(url+i,':')-url-i);
            bzero(tmp,10);
            strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);
            /* printf("tmp=%s\n",tmp); */
            // 目标端口
            proxyport=atoi(tmp);
            if(proxyport==0) proxyport=80;
        }     
        else
        {
            strncpy(host,url+i,strcspn(url+i,"/"));
        }
        // printf("Host=%s\n",host);
        // url + i + strcspn(url+i,"/") 得到域名后面的目标地址
        strcat(request+strlen(request),url+i+strcspn(url+i,"/"));

    } 
    else
    {
        // printf("ProxyHost=%s\nProxyPort=%d\n",proxyhost,proxyport);
        // 如若使用代理服务器
        strcat(request,url);
    }
  
    if(http10==1)
        strcat(request," HTTP/1.0");
    else if (http10==2)
        strcat(request," HTTP/1.1");
    // 完成如 GET / HTTP1.1 后，添加"\r\n"
    strcat(request,"\r\n");
    if(http10>0)
        strcat(request,"User-Agent: WebBench "PROGRAM_VERSION"\r\n");
    if(proxyhost==NULL && http10>0)
    {
        strcat(request,"Host: ");
        strcat(request,host);
        strcat(request,"\r\n");
    }
    // force_reload = 1 和存在代理服务器，则不缓存 
    if(force_reload && proxyhost!=NULL)
    {
        strcat(request,"Pragma: no-cache\r\n");
    }
    // 如果为HTTP1.1，则存在长连接，应将Connection置为close
    if(http10>1)
        strcat(request,"Connection: close\r\n");
    /* add empty line at end */
    // 别忘记在请求后添加"\r\n"
    if(http10>0) strcat(request,"\r\n"); 
    // printf("Req=%s\n",request);
}

/* vraci system rc error kod */
static int bench(void)
{
    int i,j,k;	
    pid_t pid=0;
    FILE *f;

    /* check avaibility of target server */
    i=Socket(proxyhost==NULL?host:proxyhost,proxyport);
    if(i<0) 
    { 
        fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
        return 1;
    }
    close(i);
    /* create pipe */
    if(pipe(mypipe))
    {
        perror("pipe failed.");
	    return 3;
    }

    /* not needed, since we have alarm() in childrens */
    /* wait 4 next system clock tick */
    /*
    cas=time(NULL);
    while(time(NULL)==cas)
        sched_yield();
    */

    /* fork childs */
    // 根据并发数创建子进程
    for(i=0;i<clients;i++)
    {
        pid=fork();
        if(pid <= (pid_t) 0)
        {
		   /* child process or error*/
            // 注意这里子进程sleep(1)
            sleep(1); /* make childs faster */
            break;
	    }
    }

    if( pid< (pid_t) 0)
    {
        fprintf(stderr,"problems forking worker no. %d\n",i);
        perror("fork failed.");
        return 3;
    }

    // 子进程则调用benchcore函数
    if(pid== (pid_t) 0)
    {
        /* I am a child */
        if(proxyhost==NULL)
        benchcore(host,proxyport,request);
        else
        benchcore(proxyhost,proxyport,request);

        /* write results to pipe */
	    f=fdopen(mypipe[1],"w");
	    if(f==NULL)
	    {
            perror("open pipe for writing failed.");
            return 3;
	    }
	    /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
	    fprintf(f,"%d %d %d\n",speed,failed,bytes);
	    fclose(f);
	    return 0;
    } 
    else
    {
        f=fdopen(mypipe[0],"r");
        if(f==NULL) 
	    {
            perror("open pipe for reading failed.");
            return 3;
	    }
	    setvbuf(f,NULL,_IONBF,0);
	    speed=0;
        failed=0;
        bytes=0;

	    while(1)
	    {
            pid=fscanf(f,"%d %d %d",&i,&j,&k);
		    if(pid<2)
            {
                fprintf(stderr,"Some of our childrens died.\n");
                break;
            }
		    speed+=i;
		    failed+=j;
		    bytes+=k;
		    /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
		    if(--clients==0) break;
	    } 
	    fclose(f);

        printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
		  (int)((speed+failed)/(benchtime/60.0f)),
		  (int)(bytes/(float)benchtime),
		  speed,
		  failed);
    }
    return i;
}

// 子进程进行压力测试，每个子进程皆会调用
void benchcore(const char *host,const int port,const char *req)
{
    int rlen;
    char buf[1500];
    int s,i;
    struct sigaction sa;

    /* setup alarm signal handler */
    // 设置alarm定时器处理函数
    sa.sa_handler=alarm_handler;
    sa.sa_flags=0;
    // sigaction 成功则返回0，失败则返回-1
    // 超时会产生信号SIGALRM，用sa中的指定函数处理
    if(sigaction(SIGALRM,&sa,NULL)) 
        exit(3);
    alarm(benchtime); // 开始计时

    rlen=strlen(req);
    nexttry:while(1)
    {
        // 超时则返回
        if(timerexpired)
        {
            if(failed>0)
            {
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                failed--;
            }
            return;
        }
        s=Socket(host,port);                          
        if(s<0) { failed++;continue;} // 连接失败，failed数加一 
        if(rlen!=write(s,req,rlen)) {failed++;close(s);continue;} // header大小与发送的不相等，则失败
        if(http10==0)  // 针对HTTP0.9的特殊处理，关闭s的写功能，成功则返回0，错误则返回-1
	    if(shutdown(s,1)) { failed++;close(s);continue;}
        if(force==0)  // force = 0表示等待从Server返回的数据
        {
            /* read all available data from socket */
	        while(1)
	        {
                if(timerexpired) break; // timerexpired默认为0，当为1时表示 
	            i=read(s,buf,1500); // 从socket读取返回数据
                /* fprintf(stderr,"%d\n",i); */
	            if(i<0) 
                { 
                    failed++;
                    close(s);
                    goto nexttry;
                }
	            else
		            if(i==0) break;
		        else
			        bytes+=i;
	        }
        }
        if(close(s)) {failed++;continue;}
        speed++;
    }
}

