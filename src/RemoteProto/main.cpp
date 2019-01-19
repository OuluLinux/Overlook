#include "Client.h"

#define IMAGECLASS Images
#define IMAGEFILE <RemoteProto/Client.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	SetIniFile(ConfigFile("Client.ini"));
	
	Session& ses = GetSession();
	ServerDialog server;
	
	if (!server.IsAutoConnect()) {
		server.Run();
	} else {
		if (!server.Connect(true))
			server.Run();
	}
	server.Close();
	
	if (ses.IsConnected()) {
		ses.Start();
		ses.DataInit();
		
		Client c;
		c.PostInit();
		c.Run();
	}
}
