#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include <dpp/dpp.h>

/* commands */
std::string command_pwd(std::vector<std::string> args);
std::string command_cd(std::vector<std::string> args);
std::string command_ls(std::vector<std::string> args);

std::string command_readfile(std::vector<std::string> args, const dpp::message_create_t evnt);

std::string command_openfile(std::vector<std::string> args, const dpp::message_create_t& evnt);

bool folder_exists(std::string path);
void init_fsys();