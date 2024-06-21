#include <stdio.h>
#include <libwebsockets.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

#define MAX_MESSAGE_LEN 512
#define USER_MAX 30
#define USERNAME_LEN_MAX 32

void create_user(struct lws *wsi, char *un);

struct UserConnection {
    char username[USERNAME_LEN_MAX];
    struct lws *wsi;
    
};

struct UserConnection user_list[USER_MAX];
int user_count = 0;

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("WebSocket connection established\n");
            break;

        case LWS_CALLBACK_RECEIVE:
            printf("Received: %.*s\n", (int)len, (char *)in);
            char recvd[MAX_MESSAGE_LEN];
            snprintf(recvd, MAX_MESSAGE_LEN, "%.*s", (int)len, (char *)in);

            cJSON *username = cJSON_Parse(recvd); 
            if (username == NULL) { 
                const char *error_ptr = cJSON_GetErrorPtr(); 
                if (error_ptr != NULL) { 
                    printf("Error: %s\n", error_ptr); 
                } 
                cJSON_Delete(username); 
                return 1; 
            }

            cJSON *type = cJSON_GetObjectItemCaseSensitive(username, "type"); 
            cJSON *un = cJSON_GetObjectItemCaseSensitive(username, "username"); 
            if (cJSON_IsString(type) && !strcmp(type->valuestring, "login") &&
                cJSON_IsString(un) && un->valuestring != NULL && strcmp(un->valuestring, "")) { 
                create_user(wsi,un->valuestring);
            }
            cJSON_Delete(username);
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            // remove_user(wsi);
            break;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    { "chat-protocol", callback_chat, 0, MAX_MESSAGE_LEN },
    { NULL, NULL, 0, 0 }
};

int main() {

    // for (int i = 0; i < USER_MAX; i++) {
    //     user_list[i].username[0] = NULL;
    // }

    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        printf("Error creating WebSocket context\n");
        return 1;
    }

    printf("WebSocket server listening on port 8080...\n");

    while (1) {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
    return 0;
}

void create_user(struct lws *wsi, char *un) {
    printf("Adding user: %s\n", un); 
    cJSON *send = cJSON_CreateObject(); 
    char *json_str;
    // Check if server is full
    if (user_count >= USER_MAX) {
        printf("Couldn't add user: server is full"); 
        cJSON_AddStringToObject(send, "type", "error"); 
        cJSON_AddStringToObject(send, "error", "Server is full, please try again later");
        json_str = cJSON_Print(send);
        lws_write(wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
    }
    else {
        // this section of the code iterates over the list thrice... probably a better way to do this

        bool taken = false;
        for (int i = 0; i < user_count; i++) {
            if (!strcmp(user_list[i].username,un)) {
                taken = true;
                break;
            }
        }
        if (taken) {
            printf("Couldn't add user: username taken\n"); 
            cJSON_AddStringToObject(send, "type", "error"); 
            cJSON_AddStringToObject(send, "error", "Username taken, please choose another");
            json_str = cJSON_Print(send);
            lws_write(wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
        }
        else {
            // Add info to new entry in user_list
            user_list[user_count].wsi = wsi;
            strncpy(user_list[user_count].username,un,USERNAME_LEN_MAX);
            user_count++;
            
            // Update everyone's active user list
            cJSON_AddStringToObject(send, "type", "userList"); 
            cJSON *users = cJSON_CreateArray();
            cJSON_AddItemToObject(send, "users", users);
            for (int i = 0; i < user_count; i++) {
                cJSON_AddItemToArray(users, cJSON_CreateString(user_list[i].username));
                printf("User #%i:%s\n",i,user_list[i].username);
            }
            json_str = cJSON_Print(send);
            // printf("JSON:\n%s\n",json_str);
            for (int i = 0; i < user_count; i++) {
                lws_write(user_list[i].wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
            }
        }
    }
    cJSON_Delete(send); 
    // cJSON_free(json_str);
}