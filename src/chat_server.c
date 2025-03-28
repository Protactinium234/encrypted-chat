#include <stdio.h>
#include <libwebsockets.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

#define MAX_MESSAGE_LEN 512
#define USER_MAX 30
#define USERNAME_LEN_MAX 32

// this program is FULL of security holes (and probably bugs) (memory management is hard)
// The user could DoS but sending in malformed JSON, by spamming the API with dead users.
// They could impersonate others cause there's no login verification system, 
// Can also impersonate bc supplying the sender username is client-side
// it's all bad
// if this is ever used in production, the CISO should be jailed

// TODO: valgrind the hell out of this
// TODO: this would be fun to fuzz!

// TODO: Encrypt chats with OpenSSL

void create_user(struct lws *wsi, char *un);
void remove_user(struct lws *wsi);
void send_message(char *sender, char *receiver, char *message);

struct UserConnection {
    char username[USERNAME_LEN_MAX];
    struct lws *wsi;
    
};

// Rather than iterating through this list, there's probably a better way I can do this with hashtables or something
// It also may be better as some sort of linked list rather than an array so I can quickly remove users
struct UserConnection user_list[USER_MAX];
int user_count = 0;

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("New WebSocket connection established\n");
            break;

        case LWS_CALLBACK_RECEIVE:
            // printf("Received: %.*s\n", (int)len, (char *)in);
            char recvd[MAX_MESSAGE_LEN];
            snprintf(recvd, MAX_MESSAGE_LEN, "%.*s", (int)len, (char *)in);

            cJSON *call = cJSON_Parse(recvd); 
            if (call == NULL) { 
                const char *error_ptr = cJSON_GetErrorPtr(); 
                if (error_ptr != NULL) { 
                    printf("Error: %s\n", error_ptr); 
                } 
                cJSON_Delete(call); 
                return 1; 
            }

            cJSON *type = cJSON_GetObjectItemCaseSensitive(call, "type"); 
            cJSON *un = cJSON_GetObjectItemCaseSensitive(call, "username"); 
            cJSON *sender = cJSON_GetObjectItemCaseSensitive(call, "sender"); 
            cJSON *receiver = cJSON_GetObjectItemCaseSensitive(call, "receiver"); 
            cJSON *message = cJSON_GetObjectItemCaseSensitive(call, "message"); 
            // if it's a login message
            if (cJSON_IsString(type) && !strcmp(type->valuestring, "login") &&
                cJSON_IsString(un) && strcmp(un->valuestring, "")) { 
                create_user(wsi,un->valuestring);
                // putting this outside the conditional crashes it sometimes. but putting it here may not always free it and cause a memory leak
                // TODO: Check if there's actually a leak
                cJSON_Delete(call);
            }
            else if (cJSON_IsString(type) && !strcmp(type->valuestring, "chat") &&
                     cJSON_IsString(sender) && strcmp(sender->valuestring, "") &&
                     cJSON_IsString(receiver) && strcmp(receiver->valuestring, "") &&
                     cJSON_IsString(message) && strcmp(message->valuestring, "")
                    ) {
                send_message(sender->valuestring, receiver->valuestring, message->valuestring);
                // cJSON_Delete(call);
            }
            break;

        case LWS_CALLBACK_CLOSED:
            remove_user(wsi);
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
    // TODO: Check for memory leak by not freeing json_str
    // cJSON_free(json_str);
    return;
}

void send_message(char *sender, char *receiver, char *message) {
    printf("Sending message from %s to %s: %s\n", sender, receiver, message);
    cJSON *send = cJSON_CreateObject();
    char *json_str;
    // Find reciever socket 
    for (int i = 0; i < user_count; i++) {
        if (!strcmp(user_list[i].username, receiver)) {
            cJSON_AddStringToObject(send, "type", "chat");
            cJSON_AddStringToObject(send, "sender", sender);
            cJSON_AddStringToObject(send, "message", message);
            json_str = cJSON_Print(send);
            lws_write(user_list[i].wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
            break;
        }
    }
    cJSON_Delete(send);
    return;
}

void remove_user(struct lws *wsi) {
    // For each user, check if that was the one that disconnected
    for (int i = 0; i < user_count; i++) {
        if (wsi == user_list[i].wsi) {
            char *json_str;
            cJSON *send = cJSON_CreateObject();
            // Remove user from list of users
            printf("Removing User %s\n",user_list[i].username);
            for (int j = i; j < user_count; j++) {
                user_list[j] = user_list[j+1];
            }
            user_count--;
            // Update everyone's active user list
            cJSON_AddStringToObject(send, "type", "userList"); 
            cJSON *users = cJSON_CreateArray();
            cJSON_AddItemToObject(send, "users", users);
            for (int i = 0; i < user_count; i++) {
                cJSON_AddItemToArray(users, cJSON_CreateString(user_list[i].username));
                printf("User #%i:%s\n",i,user_list[i].username);
            }
            json_str = cJSON_Print(send);
            for (int i = 0; i < user_count; i++) {
                lws_write(user_list[i].wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
            }
            cJSON_Delete(send);
        }
    }
    return;
}