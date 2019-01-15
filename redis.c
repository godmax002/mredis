# include "dict.h"
# include "adlist.h"
# include "sds.h"
# include "ae.h"
# include "anet.h"
# include "zmalloc.h"
# include <time.h>
# include <errno.h>
# include <signal.h>
# include <stdio.h>


# define REDIS_MAX_ARGS 16
# define REDIS_CMD_BULK 1
# define REDIS_CMD_INLINE 2
# define REDIS_DEBUG 0
# define REDIS_NOTICE 1
# define REDIS_WARNING 2
# define REDIS_MAXIDLETIME (60*5)
# define REDIS_DEFAULT_DBNUM 16
// replication
# define REDIS_REPL_NONE 0
# define REDIS_REPL_CONNECT 1
# define REDIS_REPL_CONNECTED 2

# define ANET_ERR_LEN 1024
# define REDIS_CONFIGLINE_MAX 1024
# define REDIS_QUERYBUF_LEN 1024
# define REDIS_NOTUSED(v) ((void) v)
// type
# define REDIS_STRING 0
# define REDIS_LIST 1
# define REDIS_SET 2
# define REDIS_HASH 3
# define REDIS_OK 0 
# define REDIS_OK -1 


typedef struct redisObj {
    int type;
    void *ptr;
    int refcount;
} robj;
struct redisClient;

typedef void redisCommandProc(struct redisClient *c);
struct redisCommand {
    char *name;
    redisCommandProc *proc;
    int arity;
    int flags;
};

typedef struct redisClient {
    int fd;
    dict *dict;
    int dictid;
    sds querybuf;
    robj *argv[REDIS_MAX_ARGS];
    int argc;
    int bulklen; // if in bulk read mode
    list *reply;

    int sentlen;
    time_t lastinteraction;
    int flags;
    int slaveseldb; // slave select db
} redisClient;


struct redisServer {
    int port;
    int fd;
    dict *dict;
    
    list *clients;
    list *slaves;
    aeEventLoop *el;

    // stat
    char neterr[ANET_ERR_LEN];
    long long dirty;
    int cronloops;
    list *objfreelist;
    time_t lastsave;
    int usedmemory;

    time_t stat_starttime;
    long long stat_numcommands;
    long long stat_numconnections;

    // conf
    int verbosity;
    int glueoutputbuf;
    int maxidletime;
    int dbnum;
    int daemonize;
    int bgsaveinprogress;
    struct saveparam *saveparams;
    int saveparamslen;
    char *logfile;
    char *bindaddr;
    char *dbfilename;

    // rep
    int isslave;
    char *masterhost;
    int masterport;
    redisClient *master;
    int replstate;

    // sort
    int sort_desc;
    int sort_alpha;
    int sort_bypattern;
};

static void freeStringObject(robj *o);
static void freeListObject(robj *o);
static void freeSetObject(robj *o);
static void decrRefCount(void *o);
static robj *createObject(int type, void *ptr);
static void freeClient(redisClient *c);
static int loadDb(char *filename);
static void addReply(redisClient *c, robj *obj);
static void addReplySds(redisClient *c, sds s);
static void incrRefCount(robj *o);
static int saveDbBackground(char *filename);
static robj *createStringObject(char *ptr, size_t len);
static void replicationFeedSlaves(struct redisCommand *cmd, int dictid, robj **argv, int argc);
static int syncWithMaster(void);

static void pingCommand(redisClient *c);
static void echoCommand(redisClient *c);
static void setCommand(redisClient *c);
static void setnxCommand(redisClient *c);
static void getCommand(redisClient *c);
static void delCommand(redisClient *c);
static void existsCommand(redisClient *c);
static void incrCommand(redisClient *c);
static void decrCommand(redisClient *c);
static void incrbyCommand(redisClient *c);
static void decrbyCommand(redisClient *c);
static void selectCommand(redisClient *c);
static void randomkeyCommand(redisClient *c);
static void keysCommand(redisClient *c);
static void dbsizeCommand(redisClient *c);
static void lastsaveCommand(redisClient *c);
static void saveCommand(redisClient *c);
static void bgsaveCommand(redisClient *c);
static void shutdownCommand(redisClient *c);
static void moveCommand(redisClient *c);
static void renameCommand(redisClient *c);
static void renamenxCommand(redisClient *c);
static void lpushCommand(redisClient *c);
static void rpushCommand(redisClient *c);
static void lpopCommand(redisClient *c);
static void rpopCommand(redisClient *c);
static void llenCommand(redisClient *c);
static void lindexCommand(redisClient *c);
static void lrangeCommand(redisClient *c);
static void ltrimCommand(redisClient *c);
static void typeCommand(redisClient *c);
static void lsetCommand(redisClient *c);
static void saddCommand(redisClient *c);
static void sremCommand(redisClient *c);
static void sismemberCommand(redisClient *c);
static void scardCommand(redisClient *c);
static void sinterCommand(redisClient *c);
static void sinterstoreCommand(redisClient *c);
static void syncCommand(redisClient *c);
static void flushdbCommand(redisClient *c);
static void flushallCommand(redisClient *c);
static void sortCommand(redisClient *c);
static void lremCommand(redisClient *c);
static void infoCommand(redisClient *c);

// ============================ global =====================
static struct redisServer server;
static struct redisCommand cmdTable[] = {
    {"get",getCommand,2,REDIS_CMD_INLINE},
    {"set",setCommand,3,REDIS_CMD_BULK},
    {"setnx",setnxCommand,3,REDIS_CMD_BULK},
    {"del",delCommand,2,REDIS_CMD_INLINE},
    {"exists",existsCommand,2,REDIS_CMD_INLINE},
    {"incr",incrCommand,2,REDIS_CMD_INLINE},
    {"decr",decrCommand,2,REDIS_CMD_INLINE},
    {"rpush",rpushCommand,3,REDIS_CMD_BULK},
    {"lpush",lpushCommand,3,REDIS_CMD_BULK},
    {"rpop",rpopCommand,2,REDIS_CMD_INLINE},
    {"lpop",lpopCommand,2,REDIS_CMD_INLINE},
    {"llen",llenCommand,2,REDIS_CMD_INLINE},
    {"lindex",lindexCommand,3,REDIS_CMD_INLINE},
    {"lset",lsetCommand,4,REDIS_CMD_BULK},
    {"lrange",lrangeCommand,4,REDIS_CMD_INLINE},
    {"ltrim",ltrimCommand,4,REDIS_CMD_INLINE},
    {"lrem",lremCommand,4,REDIS_CMD_BULK},
    {"sadd",saddCommand,3,REDIS_CMD_BULK},
    {"srem",sremCommand,3,REDIS_CMD_BULK},
    {"sismember",sismemberCommand,3,REDIS_CMD_BULK},
    {"scard",scardCommand,2,REDIS_CMD_INLINE},
    {"sinter",sinterCommand,-2,REDIS_CMD_INLINE},
    {"sinterstore",sinterstoreCommand,-3,REDIS_CMD_INLINE},
    {"smembers",sinterCommand,2,REDIS_CMD_INLINE},
    {"incrby",incrbyCommand,3,REDIS_CMD_INLINE},
    {"decrby",decrbyCommand,3,REDIS_CMD_INLINE},
    {"randomkey",randomkeyCommand,1,REDIS_CMD_INLINE},
    {"select",selectCommand,2,REDIS_CMD_INLINE},
    {"move",moveCommand,3,REDIS_CMD_INLINE},
    {"rename",renameCommand,3,REDIS_CMD_INLINE},
    {"renamenx",renamenxCommand,3,REDIS_CMD_INLINE},
    {"keys",keysCommand,2,REDIS_CMD_INLINE},
    {"dbsize",dbsizeCommand,1,REDIS_CMD_INLINE},
    {"ping",pingCommand,1,REDIS_CMD_INLINE},
    {"echo",echoCommand,2,REDIS_CMD_BULK},
    {"save",saveCommand,1,REDIS_CMD_INLINE},
    {"bgsave",bgsaveCommand,1,REDIS_CMD_INLINE},
    {"shutdown",shutdownCommand,1,REDIS_CMD_INLINE},
    {"lastsave",lastsaveCommand,1,REDIS_CMD_INLINE},
    {"type",typeCommand,2,REDIS_CMD_INLINE},
    {"sync",syncCommand,1,REDIS_CMD_INLINE},
    {"flushdb",flushdbCommand,1,REDIS_CMD_INLINE},
    {"flushall",flushallCommand,1,REDIS_CMD_INLINE},
    {"sort",sortCommand,-2,REDIS_CMD_INLINE},
    {"info",infoCommand,1,REDIS_CMD_INLINE},
    {NULL,NULL,0,0}
};


static void ResetServerSaveParams() {
    zfree(server.saveparams);
    server.saveparams = NULL;
    server.saveparamslen = 0;
}

static void appendServerSaveParams(time_t seconds, int change) {

}

static void initServerConfig() {
    server.verbosity = REDIS_DEBUG;
    server.glueoutputbuf = 1;
    server.maxidletime = REDIS_MAXIDLETIME;
    server.dbnum = REDIS_DEFAULT_DBNUM;
    server.daemonize = 0; 
    // server.bgsaveinprogress;
    // server.saveparam *saveparams;
    // server.saveparamslen;
    // stdout
    server.logfile = NULL;
    server.bindaddr = NULL;
    server.dbfilename = "dump.rdb";

    // save para
    ResetServerSaveParams();
    // save 1hour and 1 change
    appendServerSaveParams(60*60, 1);
    // 5 min and 100 change
    appendServerSaveParams(300, 100);
    // 1 min and 10000 change
    appendServerSaveParams(60, 10000);

    // rep
    server.isslave = 0 ;
    server.masterhost = NULL;
    server.masterport = 6379;
    server.master = NULL;
    server.replstate = REDIS_REPL_NONE;
}

// todo: not finished
static void loadServerConfig(char *filename) {
    char *fp  = open(filename, "r");
    char buf[REDIS_CONFIGLINE_MAX+1], *err;
    sds line = NULL;
    int linenum = 0;

    while(fgets(buf, REDIS_CONFIGLINE_MAX+1, fp) != NULL){
        sds *argv;
        int argc;

        linenum++;
        line = sdsnew(buf);
        line = sdstrim(line, "\t\r\n");

        // skip comment and blank line
        if(line[0] == '#' || line[0] == '\0'){
            sdsfree(line);
            continue;
        }

        // load config
        argv = sdssplitlen(line, sdslen(line), " ", 1, &argc);
        sdstolower(argv[0]);

        if(!strcmp(argv[0], "timeout") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.maxidletime < 1){
                err = "Invalid timeout value";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "port") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "bind") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "save") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "dir") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "loglevel") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }else if(!strcmp(argv[0], "save") && argc == 2){
            server.maxidletime = atoi(argv[1]);
            if (server.port < 1 || server.port > 65535){
                err = "Invalid port ";
                goto loaderr;
            }
        }
    }

    loaderr:
    fprintf(stderr, "\n*** FATAL CONFIG FILE ERROR ***\n");
    fprintf(stderr, "Reading the configuration file, at line %d\n", linenum);
    fprintf(stderr, ">>> '%s'\n", line);
    fprintf(stderr, "%s\n", err);
    exit(1);
}

// hash type : todo

static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask){
    redisClient *c = (redisClient*)privdata;
    char buf[REDIS_QUERYBUF_LEN];
    int nread;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);

    nread = read(fd, buf, REDIS_QUERYBUF_LEN);
    if(nread == -1){
        if(errno == EAGAIN){
            nread = 0;
        }else {
            redisLog(REDIS_DEBUG, "Reading from client: %s", strerr(errno));
            freeClient(c);
            return;
        }
    }else if (nread == 0){
            redisLog(REDIS_DEBUG, "Client closed connection");
            freeClient(c);
            return;
    }

    if(nread){
        c->querybuf = sdscatlen(c->querybuf, buf, nread);
        c->lastinteraction = time(NULL);
    } else {
        return ;
    }

again:
    if(c->bulklen == -1){
        char *p = strchr(c->querybuf, '\n');
        size_t querylen;
        if(p) {
            sds query, *argv;
            int argc;

            query = c->querybuf;
            c->querybuf = sdsempty();
            querylen = 1 + (p-query);
            if(sdslen(query) > querylen){
                c->querybuf = sdscatlen(c->querybuf, query+querylen, sdslen(query) - querylen);
            }

            *p = '\0';
            if(*(p-1) == '\r') *(p-1) = '\0';
            sdsupdatelen(query);
            if(sdslen(query) == 0){
                sdsfree(query);
                return;
            }

            argv = sdssplitlen(query, sdslen(query), " ", 1, &argc);
            sdsfree(query);
            for(int i = 0; i < argc && i < REDIS_MAX_ARGS; i++) {
                if(sdslen(argv[i])){
                    c->argv[c->argc] = createObject(REDIS_STRING, argv[i]);
                    c->argc++;
                }else
                    sdsfree(argv[i]);
            }
            zfree(argv);
            if(processCommand(c) && sdslen(c->querybuf)) goto again;
            return;
        } else if (sdslen(c->querybuf) >= 1024){
            redisLog(REDIS_DEBUG, "Client protocol error");
            freeClient(c);
            return;
        }
    }else{
        // bulk read
        int qbl = sdslen(c->querybuf);
        if(c->bulklen <= qbl){
            c->argv[c->argc] = createStringObject(c->querybuf, c->bulklen-2);
            c->argc++;
            c->querybuf = sdsrange(c->querybuf, c->bulklen, -1);
            processCommand(c);
            return;
        }
    }
}

static void acceptHandler(aeEventLoop *el, int fd, void *privData, int mask){
    char buf[100];
    printf("file event %d \n", fd);
    read(fd, buf, 100);
    printf("file content %s \n", buf);
}

static int selectDb(redisClient *c, int id){
    c->dict = &server.dict[id];
    c->dictid = id;
    return REDIS_OK;
}

static redisClient *createClient(int fd){
    redisClient *c = zmalloc(sizeof(*c));

    anetNonBlock(NULL, fd);
    anetTcpNoDelay(NULL, fd);

    selectDb(c, 0);
    c->fd = fd;
    c->querybuf = sdsempty();
    robj *argv[REDIS_MAX_ARGS];
    c->argc = 0 ;
    c->bulklen = -1;
    c->reply = listCreate();
    listSetFreeMethod(c->reply, decrRefCount);

    c->sentlen = 0;
    c->lastinteraction = time(NULL);
    c->flags = 0;

    aeFileEvent fe;
    fe.fd = c->fd;
    fe.mask = AE_READABLE;
    fe.fileProc = readQueryFromClient;
    fe.finalizerProc = NULL;
    fe.clientData = c;
    aeCreateFileEvent(server.el, &fe);
    listAddNodeTail(server.clients, c);
    return c;
}

static void initServer() {
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // int port;
    server.fd = anetTcpServer(server.neterr, server.port, server.bindaddr);
    server.dict = zmalloc(sizeof(dict*) * server.dbnum);
    for(int i=0; i<server.dbnum; i++){
        // server.dict[i] = dictCreate(&hashDictType, NULL);
    }
    server.clients = listCreate();
    server.slaves = listCreate();
    server.el = aeCreateEventLoop();

    // stat
    // char neterr[ANET_ERR_LEN];
    server.dirty  = 0;
    server.cronloops = 0;
    server.objfreelist = listCreate();
    server.lastsave = time(NULL);
    server.usedmemory = 0;

    server.stat_starttime = time(NULL);
    server.stat_numcommands = 0 ;
    server.stat_numconnections = 0 ;
}

int main(int argc, char **argv) {
    initServerConfig();
    // loadServerConfig();
    initServer();
    aeFileEvent fe;
    fe.fd = server.fd;
    fe.mask = AE_READABLE;
    fe.fileProc = acceptHandler;
    fe.finalizerProc = NULL;
    fe.clientData = NULL;
    aeCreateFileEvent(server.el, &fe);
    printf("hello4");
    fflush(stdout);
    aeMain(server.el);
}





























            
