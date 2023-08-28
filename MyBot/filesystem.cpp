#include "filesystem.h"

using namespace std::filesystem;

directory_entry cur_dir;

void init_fsys() {
	cur_dir = directory_entry(path("C:"));
}

/* returns current directory we are in */
std::string command_pwd(std::vector<std::string> args) {
	return cur_dir.path().generic_string() + ">";
}

std::string command_cd(std::vector<std::string> args) {
	if (args.size() == 1) {
		cur_dir = directory_entry(path("C:"));
		return cur_dir.path().generic_string();
	}

	/* if we want to go back a dir */
	if (args[1] == "..") {
		cur_dir = directory_entry(cur_dir.path().parent_path());
		return cur_dir.path().string();
	}

	std::string desired_path = cur_dir.path().generic_string() + '/' + args[1];

	/* if the folder doesnt exist, return with an error */
	if (!folder_exists(desired_path))
		return "Folder Not Found!";

	/* set the current dir to the one we CD'd to */
	cur_dir = directory_entry(path(desired_path));

	return desired_path;
}

std::string command_ls(std::vector<std::string> args) {
	std::string _path;
	/* for some reason a dir called C: needs to be called C:/ */
	if (cur_dir.path().generic_string() == "C:")
		_path = "C:/";
	else
		_path = cur_dir.path().generic_string();

	directory_iterator it(_path);
	/* the string to output*/
	std::string output;

	for (directory_entry file : it) {
		output += file.path().filename().generic_string();
		output += '\n';
	}

	return output;
}

/* checking if a folder exists given its path */
bool folder_exists(std::string _path) {
	/* first check if the path exists or not, could be a file or dir */
	if (!std::filesystem::exists(_path))
		return false;
	/* if it exists, then check if its a directory */
	return std::filesystem::is_directory(_path);
}