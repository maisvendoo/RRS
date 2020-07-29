#include "udp-client.h"
#include "udp-server.h"

int main()
{
    udp_server_data_t data;

    data.time = 40;
    data.msgCount = 10;
    data.vehicleCount = 5;
    data.vehicles[0].coord = 200;
    data.vehicles[0].DebugMsg = "Debug";
    data.vehicles[0].velocity = 60;
    data.vehicles[0].routePath = "Path";
    data.vehicles[0].analogSignal = {};

    UdpClient *client = new UdpClient();
    UdpServer *server = new UdpServer();

    QString cfg_path = "../../../cfg/udp-server.xml";

    server->init(cfg_path);
    server->setServerData(data);

    client->init(cfg_path);

    server->receive();

    client->receive();

    return 0;
}
