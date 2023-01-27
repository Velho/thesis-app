// Thesis app
// Velho @ 2023

#include <stdlib.h>
#include <mongoose.h>

// private variables
static const char* sListeningAddress = "https://0.0.0.0:8000";
static const char* sServeRootDir = "~";
// private definitions 
static void ServerEventHandler(struct mg_connection* c, int ev, void* ev_data, void* fn_data);

int main(int argc, char* argv[])
{
    struct mg_mgr mgr;
    struct mg_connection* c;

    mg_mgr_init(&mgr);

    if ((c = mg_http_listen(&mgr, 
                            sListeningAddress,
                            ServerEventHandler, &mgr)) == NULL)
    {
        MG_ERROR(("Failed to listen on %s.", sListeningAddress));
        exit(EXIT_FAILURE);
    }

    MG_INFO(("Listening on : %s", sListeningAddress));

    for (;;)
    {
        mg_mgr_poll(&mgr, 1000); // poll the async event handler
    }

    mg_mgr_free(&mgr); // free the mongoose manager
    return 0;
}

void ServerEventHandler(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
    if (ev == MG_EV_ACCEPT)
    {
        struct mg_tls_opts tls_opts =
        {
                .cert = "certs/cert-2048.pem",
                .certkey = "certs/pk-rsa-2048.pem"
        };

        mg_tls_init(c, &tls_opts);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_serve_opts opts =
        {
            .root_dir = "./"
        };
        mg_http_serve_dir(c, ev_data, &opts);
    }
    else if (ev == MG_EV_ERROR)
    {
        MG_INFO(("SERVER : error %s", (char*)ev_data));
    }
}
