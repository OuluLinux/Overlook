#ifndef _FTP_Ftp_h_
#define _FTP_Ftp_h_

#include <Core/Core.h>

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#ifdef PLATFORM_POSIX
#include <arpa/inet.h>
#endif

NAMESPACE_UPP

class Ftp : private NoCopy {
    
public:
    class DirEntry : Moveable<Ftp::DirEntry> {
        
        enum Type       { FILE, DIRECTORY, LINK };
        Bits            owner, group, other;
        String          user, filename, ownername, groupname;
        String          direntry;
        int64           size;
        int             type;
        int             style;
        Time            time;

        friend bool     ParseFtpDirEntry(const String& in, Vector<Ftp::DirEntry>& out);
        void            Pick(DirEntry&& e);
        
    public:
        DirEntry&       User(const String& u)                           { user = u; return *this; }

        String          GetName() const                                 { return filename;  }
        String          GetOwner() const                                { return ownername;  }
        String          GetGroup() const                                { return groupname; }
        int64           GetSize() const                                 { return size;  }
        Time            GetLastModified() const                         { return time; }    
        int             GetStyle() const                                { return style; }               
        String          GetEntry() const                                { return direntry; }

        bool            IsFile() const                                  { return type == FILE; }
        bool            IsDirectory() const                             { return type == DIRECTORY; }
        bool            IsSymLink() const                               { return type == LINK; }

        bool            IsReadable() const                              { return user.IsEmpty() || user != ownername ? other[0] : owner[0]; }                               
        bool            IsWriteable() const                             { return user.IsEmpty() || user != ownername ? other[1] : owner[1]; }
        bool            IsExecutable() const                            { return user.IsEmpty() || user != ownername ? other[2] : owner[2]; }                   

        enum Style      { UNDEFINED, UNIX, DOS };

        DirEntry()      { size = 0; type = -1; time = Null; style = UNDEFINED; }
        DirEntry(DirEntry&& e) { Pick(pick(e)); }
        void operator=(DirEntry&& e) { Pick(pick(e)); }
    };
    typedef Vector<DirEntry> DirList;
    
private:    
    enum Commands       { GET, PUT };
    enum Address        { LOCAL, PEER };
    enum Encryption     { NONE, SSL3, TLS };

    TcpSocket           control_socket, *target;
    String              ftp_host;
    int                 ftp_port;
    String              user_id;
    String              user_password;
    String              reply;
    int                 reply_code;
    int                 data_type;
    int                 transfer_mode;
    int                 chunk_size;
    bool                ftps;
    bool                progress;
    bool                aborted;

    Gate1<String>       WhenList;
    Gate2<int64, int64> WhenData;

    bool                ReplyIsWait() const                                 { return reply_code >= 100 && reply_code <= 199; }
    bool                ReplyIsSuccess() const                              { return reply_code >= 200 && reply_code <= 299; }
    bool                ReplyIsPending() const                              { return reply_code >= 300 && reply_code <= 399; }
    bool                ReplyIsFailure() const                              { return reply_code >= 400 && reply_code <= 499; }
    bool                ReplyIsError() const                                { return reply_code >= 500 || reply_code == -1;  }

    String              DecodePath(const String& path);
    String              EncodePath(const String& path);

    int                 GetRandomPort(int min, int max);
    bool                GetSockAddr(int type, int& family, String& ip, int& port);

    bool                SetError(const String& e);

    bool                InitFtps(int size);
    int                 GetReplyCode(const String& s);
    bool                PutGet(const String& s, bool nolog = false);
    bool                SetDataType(int type);
    bool                SetTransferMode(String& addr, int& port, int& family);
    bool                TransferData(int cmd, const String& request, Stream& file, int64 size, int type);
    bool                GetData(TcpSocket& socket, Stream& out, int64 sz, int type, bool log = false);
    bool                PutData(TcpSocket& socket, Stream& in, int type);
        
public:
    typedef             Ftp CLASSNAME;
    
    enum Type           { ASCII, BINARY };
    enum Mode           { ACTIVE, PASSIVE };   

    Ftp&                User(const String& user, const String& pass)        { user_id = user; user_password = pass; return *this; }
    Ftp&                SSL(bool b = true)                                  { ftps = b; return *this; }
    Ftp&                Active()                                            { transfer_mode = ACTIVE; return *this; }
    Ftp&                Passive()                                           { transfer_mode = PASSIVE; return *this; }
    Ftp&                Timeout(int ms)                                     { control_socket.Timeout(ms); return *this; }
    Ftp&                WaitStep(int ms)                                    { control_socket.WaitStep(ms); return *this; }
    Ftp&                ChunkSize(int size)                                 { if(size > 0) chunk_size = size; return *this; }
    
    bool                Connect(const String& host, int port = 21);
    void                Disconnect();

    String              GetDir();
    bool                SetDir(const String& path);
    bool                DirUp();
    bool                ListDir(const String& path, DirList& list, Gate1<String> progress = false);
    bool                MakeDir(const String& path);
    bool                RemoveDir(const String& path);

    bool                Get(const String& path, Stream& out, Gate2<int64, int64> progress = false, int type = BINARY);
    bool                Put(Stream& in, const String& path, Gate2<int64, int64> progress = false, int type = BINARY);
    bool                Info(const String& path, DirEntry& info);
    bool                Rename(const String& oldname, const String& newname);
    bool                Delete(const String& path);
    
    bool                Noop();
    void                Abort();
    int                 SendCommand(const String& cmd)                      { PutGet(cmd); return reply_code; } 

    bool                InProgress() const                                  { return progress; }

    Callback            WhenWait;
    
    TcpSocket&          GetSocket()                                         { return control_socket; }
    int                 GetCode() const                                     { return reply_code; }
    String              GetReply() const                                    { return reply; }
    String              GetReplyAsXml();  
      
    static void         Trace(bool b = true);
    
    Ftp();
    virtual ~Ftp();
    
};

int     FtpGet(const String& path, Stream& out, const String& host, int port = 21, const String& user = Null, const String& pass = Null, 
            Gate2<int64, int64> progress = false, Callback whenwait = CNULL, int type = Ftp::BINARY, int mode = Ftp::PASSIVE, bool ssl = false);
int     FtpPut(Stream& in, const String& path, const String& host, int port = 21, const String& user = Null, const String& pass = Null, 
            Gate2<int64, int64> progress = false, Callback whenwait = CNULL, int type = Ftp::BINARY, int mode = Ftp::PASSIVE, bool ssl = false);
bool    ParseFtpDirEntry(const String& in, Ftp::DirList& out);

END_UPP_NAMESPACE
#endif
