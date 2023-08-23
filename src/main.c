// Thesis app
// Velho @ 2023
// example,
//  ./thesis-app -cert certificate.crt -pk rsa.pem

#include <getopt.h>
#include <mongoose.h>
#include <stdio.h>
#include <stdlib.h>

#define OPTIONS_MAX_LEN 32

/**
 * command line arguments provided as arguments for the server.
 */
struct Options
{
    char cert[OPTIONS_MAX_LEN];
    char pk[OPTIONS_MAX_LEN];
};

/// private variables
static const char *sListeningAddress = "https://0.0.0.0:8888";
static const char *sServeRootDir = "~";
static int sDebugLevel = MG_LL_VERBOSE;
struct Options opts = { 0 };

int ParseOptions(int argc, char *argv[], struct Options *opts, int *s_count);

/// private definitions
static void ServerEventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

int main(int argc, char *argv[])
{
    struct mg_mgr mgr;
    struct mg_connection *c;
    int opts_count = 0;

    ParseOptions(argc, argv, &opts, &opts_count);

    if (opts_count < 2)
    {
        printf("usage: ./thesis-app --cert /path/to/cert --pk /path/to/pk");
        exit(EXIT_FAILURE);
    }

    printf("%s, %s\n", opts.cert, opts.pk);

    mg_log_set("5");
    mg_mgr_init(&mgr);

    if ((c = mg_http_listen(&mgr, sListeningAddress, ServerEventHandler, &mgr)) == NULL)
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

void ServerEventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_ACCEPT)
    {
        struct mg_tls_opts tls_opts = {.certkey = opts.pk, .cert = opts.cert};

        mg_tls_init(c, &tls_opts);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = ev_data, tmp = {0};
        struct mg_str unk = mg_str_n("?", 1), *cl;
        struct mg_http_serve_opts opts = {.root_dir = "."};
        mg_http_serve_dir(c, ev_data, &opts);
        mg_http_parse((char *)c->send.buf, c->send.len, &tmp);
        cl = mg_http_get_header(&tmp, "Content-Length");

        if (cl == NULL)
        {
            cl = &unk;
        }

        MG_INFO(("%.*s %.*s %.*s %.*s", (int)hm->method.len, hm->method.ptr, (int)hm->uri.len, hm->uri.ptr,
                 (int)tmp.uri.len, tmp.uri.ptr, (int)cl->len, cl->ptr));
    }
    else if (ev == MG_EV_ERROR)
    {
        MG_INFO(("SERVER : error %s", (char *)ev_data));
    }
}

int ParseOptions(int argc, char *argv[], struct Options *opts, int *count)
{
    int ret = 0;
    int c = 0;
    int index = 0;

    static struct option t_opts[] = {
        {"cert", required_argument, 0, 'a'}, {"pk", required_argument, 0, 'b'}, {0, 0, 0, 0} // null
    };

    // while (1) // parse until we have consumed all args
    // {
    index = 0;
    while ((c = getopt_long(argc, argv, "a:b:", t_opts, &index)) != -1)
    {
        switch (c)
        {
        case 'a':
            strncpy(opts->cert, optarg, OPTIONS_MAX_LEN);
            *count += 1;
            break;

        case 'b':
            strncpy(opts->pk, optarg, OPTIONS_MAX_LEN);
            *count += 1;
            break;
        case '?':
            ret = -1;
            break;
        default:
            ret = -1;
        }
    }

    return ret;
}
