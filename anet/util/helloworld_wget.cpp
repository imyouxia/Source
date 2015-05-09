/**
 * "helloworld_wget" is a tiny tool to fetch web page just like a 
 * subset of wget. 
 * "$ ./helloworld_wget http://www.xxx.com/you/path/page.html"
 * or 
 * "$ ./helloworld_wget http://www.xxx.com:9999/you/path/page.html"
 * or 
 * "$ ./helloworld_wget www.xxx.com:9999/you/path/page.html"
 */

#include <anet/log.h>
#include <anet/anet.h>
using namespace anet;
class HandleResult : public IPacketHandler {
  HPRetCode handlePacket(Packet *packet, void *args) {
    if (packet->isRegularPacket()) {
      HTTPPacket *httpPacket = 
	dynamic_cast<HTTPPacket*>(packet);
      printf("%s %d %s\r\n", HTTPPacket::HTTP_1_0 == httpPacket->getVersion() ? 
	     "HTTP\\1.0" : "HTTP\\1.1",  httpPacket->getStatusCode(), 
	     httpPacket->getReasonPhrase());
      HTTPPacket::ConstHeaderIterator it = httpPacket->headerBegin();
      while (it!=httpPacket->headerEnd()) {
	printf("%s: %s\r\n", it->first, it->second);
	it++;
      }
      printf("\r\n");
      printf("%s\r\n", httpPacket->getBody());
      httpPacket->free();
      exit(0);
    } else {
      ControlPacket *cmd = 
	dynamic_cast<ControlPacket*>(packet);
      ANET_LOG(ERROR, "Control Packet (%s) received!", cmd->what());
      exit(1);
    }
    return IPacketHandler::FREE_CHANNEL;
  }  
};

class Wget {
public:
  Wget(const char *sepc, const char *path) {
    assert(sepc);
    assert(path);
    _sepc = strdup(sepc);
    _path = strdup(path);
  }
  
  ~Wget() {
    free(_sepc);
    free(_path);
  }
  
  bool doFetch() {
    HTTPPacket *packet = new HTTPPacket;
    packet->setMethod(HTTPPacket::HM_GET);
    packet->setURI(_path);
    packet->addHeader("Accept", "*/*");
    packet->addHeader("Connection", "Keep-Alive");
    packet->addHeader("Host", (const char*)(_sepc+4));

    Transport tran;
    tran.start(true);
    HTTPPacketFactory factory;
    HTTPStreamer streamer(&factory);
    Connection *conn = tran.connect(_sepc, &streamer, false);
    conn->postPacket(packet, &handler, NULL);
    while(true) {
      usleep(1000000);
    }
    return true;
  }
  
protected:
  char *_sepc;
  char *_path;
  HandleResult handler;
};

int main(int argc, char *argv[]) {
  if(2 > argc) {
    printf("Less address!\n");
    return 1;
  }
  Logger::logSetup();
  Logger::setLogLevel(0);  
  char *address = argv[1];
  char host[100] = {0};
  char path[1024*1024] = {0};
  char sepc[200] = {0};
  int port = -1;
  //  printf("host:%s port:%d path%s\n", host, port, path);
  sscanf(address, "http://%[-a-zA-Z0-9.]:%d%s", host, &port, path);
  //  printf("host:|%s| port:%d path%s\n", host, port, path);

  if(0 == host[0] || -1 == port) {
    sscanf(address, "%[-a-zA-Z0-9.]:%d%s", host, &port, path);
    //    printf("host:|%s| port:%d path:%s\n", host, port, path);

  }
  if(0 == host[0] || -1 == port) {
    port = 80;
    sscanf(address, "http://%[-a-zA-Z0-9.]%s", host, path);
    //    printf("host:%s port:%d path:%s\n", host, port, path);

  }
  if(0 == host[0] || -1 == port) {
    sscanf(address, "%[-a-zA-Z0-9.]%s", host, path);
    //    printf("host:%s port:%d path:%s\n", host, port, path);
  }
  if(0 == host[0] || -1 == port) {
    printf("Wrong address!\n");
    return 1;
  }
  if(0 == path[0]) {
    path[0] = '/';
    path[1] = 0;
  }
  //  printf("host:%s port:%d path:%s\n", host, port, path);
  //  printf("Parse success!\n");
  printf("  ############ result  from: %s ########## \r\n", address);
  sprintf(sepc, "tcp:%s:%d", host, port);
  //  printf("sepc: %s\n", sepc);
  //  printf("path: %s\n", path);
  Wget wget(sepc, path);
  wget.doFetch();
}
