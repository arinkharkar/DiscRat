#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

/* commands */
std::string command_pwd(std::vector<std::string> args);
std::string command_cd(std::vector<std::string> args);
std::string command_ls(std::vector<std::string> args);

bool folder_exists(std::string path);
void init_fsys();