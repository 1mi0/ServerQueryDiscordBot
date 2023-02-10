#include "../pch.hpp"
#include "../Secret.hpp"
#include "Sql.hpp"

SQL::SQL()
    : _con(SQL_TOKEN) {
}

// TODO: synchronize the database thread with the server thread
// NOTE: the server thread queries from the servers then can send
//       to the bot and the sql thread to update the discord and
//       the database
