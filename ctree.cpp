#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class args {
public:
  bool help = false;
  bool version = false;
  bool showHidden = false;
  bool unsort = false;
  bool summary = false;
  std::string dir = ".";

  void parseArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];
      if (arg == "-h" or arg == "--help") {
        help = true;
      } else if (arg == "-v" or arg == "--version") {
        version = true;
      } else if (arg == "-s" or arg == "--show-hidden") {
        showHidden = true;
      } else if (arg == "-u" or arg == "--unsort") {
        unsort = true;
      } else if (arg == "-m" or arg == "--summary") {
        summary = true;
      } else if (dir == ".") {
        dir = arg;
      } else {
        throw std::invalid_argument("Invalid argument: " + arg);
      }
    }
  }

private:
};

class ctree {
private:
  std::unordered_map<std::string, std::string> icons = {
      {".c", "\033[34m\033[0m"},    {".cpp", "\033[34m\033[0m"},
      {".html", "\033[33m\033[0m"}, {".css", "\033[34m\033[0m"},
      {".js", "\033[33m\033[0m"},   {".py", "\033[36m\033[0m"},
      {".sh", "\033[0m"},           {".java", "\033[0m"},
      {".ino", "\033[34m\033[0m"},  {".rs", "\033[33m\033[0m"},
      {".go", "\033[36m\033[0m"},   {".txt", "\033[0m"},
      {".png", "\033[0m"},          {".jpg", "\033[0m"},
      {".jpeg", "\033[0m"},         {".gif", "󰵸\033[0m"},
      {".mp4", "\033[0m"},          {".mov", "\033[0m"},
      {".mp3", "\033[0m"},          {".xfc", "\033[0m"},
      {".zip", "󰛫\033[0m"},         {".gz", "󰛫\033[0m"},
      {".o", "󰆧\033[0m"},           {".obj", "󰆧\033[0m"},
      {".out", "\033[0m"},          {"", "\033[0m"},
      {".bin", "\033[0m"},          {".h", "\033[35m\033[0m"},
      {".hpp", "\033[35m\033[0m"},  {"directory", "\033[34;1m\033[0m"},
      {"other", "\033[1m\033[0m"},  {"symlink", "\033[0m"}};

  size_t dirs = 0;
  size_t files = 0;

  std::vector<std::string> inner_pointers = {"├── ", "│   "};
  std::vector<std::string> final_pointers = {"└── ", "    "};

public:
  void walk(const std::string &directory, const std::string &prefix,
            const args &argv) {
    std::vector<std::filesystem::directory_entry> entries;

    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
      if (argv.showHidden) {
        entries.push_back(entry);
        continue;
      }
      if (entry.path().filename().string()[0] != '.') {
        entries.push_back(entry);
      }
    }

    if (!argv.unsort) {
      std::sort(entries.begin(), entries.end(),
                [](const std::filesystem::directory_entry &left,
                   const std::filesystem::directory_entry &right) -> bool {
                  return left.path().filename() < right.path().filename();
                });
    }

    for (size_t index = 0; index < entries.size(); index++) {
      std::filesystem::directory_entry entry = entries[index];
      std::vector<std::string> pointers =
          index == entries.size() - 1 ? final_pointers : inner_pointers;

      std::cout << prefix << pointers[0];
      if (entry.is_directory()) {
        std::cout << icons["directory"];
      } else if (entry.is_symlink()) {
        std::cout << icons["symlink"];
      } else if (entry.is_regular_file()) {
        if (icons.find(entry.path().extension()) != icons.end()) {
          std::cout << icons[entry.path().extension()];
        } else {
          std::cout << icons["other"];
        }
      }

      if (entry.is_directory()) {
        std::cout << "\033[34;1m";
      }
      std::cout << ' ' << entry.path().filename().string();
      std::cout << '\n';

      if (!entry.is_directory()) {
        files++;
      } else {
        dirs++;
        walk(entry.path(), prefix + pointers[1], argv);
      }
    }
  }

  void summary() {
    std::cout << "\n"
              << dirs << " directories,"
              << " " << files << " files\n";
  }
};

int main(int argc, char *argv[]) {

  constexpr char version[] = "ctree: v 0.1.0";

  args arg;

  try {
    arg.parseArgs(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  if (arg.version) {
    std::cout << version << '\n';
    return 0;
  }

  if (arg.help) {
    std::cout << version << '\n';
    std::cout << "Shows a tree of files.\n";
    std::cout << "Arguments: \n";
    std::cout << "\t-h, --help: show this help menu\n";
    std::cout << "\t-v, --version: show version\n";
    std::cout << "\t-s, --show-hidden: show hidden files too\n";
    std::cout << "\t-u, --unsort: don't sort files by name\n";
    std::cout << "\t-m, --summary: show number of files and directories\n";
    return 0;
  }

  ctree tree;

  std::cout << arg.dir << '\n';
  try {
    tree.walk(arg.dir, "", arg);
  } catch (const std::exception &e) {
    std::cout << e.what() << '\n';
  }

  if (arg.summary) {
    tree.summary();
  }

  return 0;
}
