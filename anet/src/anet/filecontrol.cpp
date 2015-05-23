#include <anet/filecontrol.h>
#include <fcntl.h>
namespace anet {
// http://blog.csdn.net/chrisniu1984/article/details/7050663
// 关闭子进程无用文件描述符
bool FileControl::setCloseOnExec(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return false;
    }
    flags |= FD_CLOEXEC;
    return fcntl(fd, F_SETFD, flags) != -1;
}

bool FileControl::clearCloseOnExec(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return false;
    }
    flags &= ~FD_CLOEXEC;
    return fcntl(fd, F_SETFD, flags) != -1;
}

bool FileControl::isCloseOnExec(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return false;
    }
    return flags & FD_CLOEXEC;
}

} 


