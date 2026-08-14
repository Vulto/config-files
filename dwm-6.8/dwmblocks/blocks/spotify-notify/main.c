#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_trunc(const char *s, size_t max) {
    if (!s) return;
    size_t len = strlen(s);
    if (len <= max) {
        fputs(s, stdout);
        return;
    }
    for (size_t i = 0; i < max - 3; ++i)
        putchar(s[i]);
    fputs("...", stdout);
}

int main(void) {
    DBusError err;
    dbus_error_init(&err);

    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus conn error: %s\n", err.message);
        dbus_error_free(&err);
        return 1;
    }

    DBusMessage *msg = dbus_message_new_method_call(
        "org.mpris.MediaPlayer2.spotify",
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "Get");

    if (!msg) return 1;

    const char *iface = "org.mpris.MediaPlayer2.Player";
    const char *prop  = "Metadata";
    dbus_message_append_args(msg,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &prop,
        DBUS_TYPE_INVALID);

    DBusPendingCall *pending;
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
        dbus_message_unref(msg);
        return 1;
    }
    dbus_message_unref(msg);

    dbus_pending_call_block(pending);
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (!reply) {
        fprintf(stderr, "No reply\n");
        return 1;
    }

    DBusMessageIter args;
    if (!dbus_message_iter_init(reply, &args)) {
        fprintf(stderr, "Reply empty\n");
        dbus_message_unref(reply);
        return 1;
    }

    // Espera variant
    if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_VARIANT) {
        fprintf(stderr, "Not variant\n");
        dbus_message_unref(reply);
        return 1;
    }

    DBusMessageIter var_iter;
    dbus_message_iter_recurse(&args, &var_iter);

    // Agora dict entry array
    if (dbus_message_iter_get_arg_type(&var_iter) != DBUS_TYPE_ARRAY) {
        fprintf(stderr, "Not array\n");
        dbus_message_unref(reply);
        return 1;
    }

    DBusMessageIter dict_iter;
    dbus_message_iter_recurse(&var_iter, &dict_iter);

    char *title = NULL, *artist = NULL;

    do {
        DBusMessageIter entry_iter;
        dbus_message_iter_recurse(&dict_iter, &entry_iter);

        char *key;
        dbus_message_iter_get_basic(&entry_iter, &key);
        dbus_message_iter_next(&entry_iter);

        DBusMessageIter val_iter;
        dbus_message_iter_recurse(&entry_iter, &val_iter);

        if (strcmp(key, "xesam:title") == 0) {
            dbus_message_iter_get_basic(&val_iter, &title);
        } else if (strcmp(key, "xesam:artist") == 0) {
            DBusMessageIter array_iter;
            dbus_message_iter_recurse(&val_iter, &array_iter);
            if (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) {
                dbus_message_iter_get_basic(&array_iter, &artist);
            }
        }
    } while (dbus_message_iter_next(&dict_iter));
//    
    if (artist && title) {
        printf(" ");
        print_trunc(artist, 20);
        printf(" - ");
        print_trunc(title, 20);
        printf("   ");
    } else if (title) {
        print_trunc(title, 20);
        putchar('\n');
    } else {
        printf("No metadata\n");
    }

    dbus_message_unref(reply);
    dbus_connection_unref(conn);
    return 0;
}
