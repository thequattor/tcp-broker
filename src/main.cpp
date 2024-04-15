#include <QDebug>
#include <QObject>
#include <QCoreApplication>

//#include "version.h"
#include "tcp_server.h"
#include "tcp_client.h"

int main(int argc, char *argv[]) {
    //tcp_server *srv = nullptr;
    //tcp_client *client = nullptr;

    TcpServer *srv = nullptr;
    TcpClient *client = nullptr;

    QString param_name;
    QString host_ip_str;
    QString host_port_str = "5533";
    QString remote_ip_str;
    QString remote_port_str = "5533";

    QCoreApplication app(argc, argv);

    bool disconnect_event = 0;
    bool client_mode = 0;

    //qDebug() << "\n";
    //qDebug() << "TCP BROKER v" << TCP_BRIDGE_VERSION << "." <<TCP_BRIDGE_SUBVERSION;
    //qDebug() << "\n";

    //args = a.arguments();

    QStringList args = app.arguments();

    args.takeFirst();

    qDebug() << "args: " << args;

    int arg_sz = args.size();

    for(int i = 0; i < arg_sz; i++){
        if(args.at(i).contains('-')){
            param_name = args.at(i);
            param_name.section('-', 1, 1);

            if (param_name.contains('p') || param_name.contains("port")) {
                if (i+1 < arg_sz) {
                    host_port_str = args.at(i+1);
                }
            } else if(param_name.contains('i') || param_name.contains("ip_addr")){
                if (i+1 < arg_sz) {
                    host_ip_str = args.at(i+1);
                }
            } else if(param_name.contains('d') || param_name.contains("disconnect_event")) {
                disconnect_event = 1;
            } else if(param_name.contains('c') || param_name.contains("client_mode")) {
                client_mode = 1;
            } else if(param_name.contains('r') || param_name.contains("remote_server_ip")) {
                if ( i+1 < arg_sz) {
                    remote_ip_str = args.at(i+1);
                }
            } else if(param_name.contains('o') || param_name.contains("remote_server_port")){
                if (i+1 < arg_sz) {
                    remote_port_str = args.at(i+1);
                }
            }
        }
    }

    if (args.isEmpty()) {
        qDebug()<<"Usage: ./tcp_broker [OPTIONS] where [OPTIONS] is one of the following: ";
        qDebug()<<"-p or -port [0 - 65535]: the port of the host server to listen to for incoming connections.";
        qDebug()<<"-i or -ip_addr [xxx.xxx.xxx.xxx]: the ip address of the host server and its network interface.";
        qDebug()<<"-c or -client_mode: enable a client that will connect to a remote server";
        qDebug()<<"-r or -remote_server_ip: the ip address of the remote server (use with -c)";
        qDebug()<<"-o or -remote_server_port: the port of the remote server (use with -c)";
        qDebug()<<"-d or -disconnect_event: enable the bridging of a disconnect events from either of the hosts";

        return EXIT_FAILURE;
    }

    srv = new TcpServer();

    if (srv) {
        srv->start_server(host_ip_str, host_port_str, disconnect_event, client_mode);
    }

    if (client_mode) {
        client = new TcpClient();

        if (client) {
            client->start_client(remote_ip_str, remote_port_str, disconnect_event);
        }

        if (srv && client) {
            QObject::connect(client, SIGNAL(recv_from_remote_server(QByteArray &)), srv, SLOT(on_recv_from_remote_server(QByteArray &)));
            QObject::connect(srv, SIGNAL(send_to_remote_server(QByteArray &)), client, SLOT(on_send_to_remote_server(QByteArray &)));
            QObject::connect(client, SIGNAL(server_closed_the_connection()), srv, SLOT(on_server_closed_the_connection()));
            QObject::connect(srv, SIGNAL(client_closed_the_connection()), client, SLOT(on_client_closed_the_connection()));
        }
    }

    return app.exec();
}