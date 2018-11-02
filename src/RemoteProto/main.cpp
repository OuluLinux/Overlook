#include "Client.h"

#define IMAGECLASS Images
#define IMAGEFILE <RemoteProto/Client.iml>
#include <Draw/iml_source.h>


GUI_APP_MAIN {
	SetIniFile(ConfigFile("Client.ini"));
	
	Client c;
	ServerDialog server(c);
	
	if (!server.IsAutoConnect()) {
		server.Run();
	} else {
		if (!server.Connect(true))
			server.Run();
	}
	server.Close();
	
	if (c.IsConnected()) {
		c.PostInit();
		c.Start();
		c.Run();
	}
}
