#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

constexpr char version[] = "ctree: v 0.1.1";

struct args {
  bool showHidden = false;
  bool unsort = false;
  bool summary = false;
  std::string dir = ".";

  void parseArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];
      if (arg == "-h" or arg == "--help") {
        std::cout << version << '\n';
        std::cout << "Shows a \033[32;1m󰔱 tree\033[0m of files.\n";
        std::cout << "Arguments: \n";
        std::cout << "\t\033[34m󰋖\033[0m -h, --help: show this help menu\n";
        std::cout << "\t\033[31m\033[0m -v, --version: show version\n";
        std::cout << "\t\033[36m󰘓\033[0m -s, --show-hidden: show hidden files too\n";
        std::cout << "\t󰒺 -u, --unsort: don't sort files by name\n";
        std::cout << "\t󰉻 -m, --summary: show number of files and directories\n";
        exit(0);
      } else if (arg == "-v" or arg == "--version") {
        std::cout << version << '\n';
        exit(0);
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
};

class ctree {
private:
  const std::unordered_map<std::string, std::string> icons = {
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
      {".hpp", "\033[35m\033[0m"},  {".pdf", "\033[31m\033[0m"},
      {".md", "\033[36m\033[0m"},   {"directory", "\033[34;1m\033[0m"},
      {"other", "\033[1m\033[0m"},  {"symlink", "\033[1m\033[0m"},
      {"readme", "\033[1m\033[0m"}, {"license", "\033[33;1m󰿃\033[0m"},
  };

  size_t dirs = 0;
  size_t files = 0;

  static constexpr std::pair<const char *, const char *> inner_pointers = {
      "├── ", "│   "};
  static constexpr std::pair<const char *, const char *> final_pointers = {
      "└── ", "    "};

public:
  void walk(const std::string &directory, const std::string &prefix,
            const args &argv) {
    std::vector<std::filesystem::directory_entry> entries;

    if (!argv.unsort) {
      for (const std::filesystem::directory_entry &entry :
           std::filesystem::directory_iterator(directory)) {
        entries.push_back(entry);
      }

      std::sort(entries.begin(), entries.end(),
                [](const std::filesystem::directory_entry &left,
                   const std::filesystem::directory_entry &right) -> bool {
                  return left.path().filename() < right.path().filename();
                });
    }

    size_t index = 0;
    std::function<void(const std::filesystem::directory_entry &, size_t)>
        loopLogic = [&](const std::filesystem::directory_entry &entry,
                        size_t numberOfEntries) {
          if (!argv.showHidden and
              entry.path().filename().string().front() == '.') {
            return;
          }
          index++;
          std::pair<std::string, std::string> pointers =
              index == numberOfEntries ? final_pointers : inner_pointers;

          std::cout << prefix << pointers.first;
          if (entry.is_directory()) {
            std::cout << icons.at("directory");
          } else if (entry.is_symlink()) {
            std::cout << icons.at("symlink");
          } else if (entry.is_regular_file()) {
            if (entry.path().filename().string().find("readme") !=
                    std::string::npos or
                entry.path().filename().string().find("README") !=
                    std::string::npos) {
              std::cout << icons.at("readme");
            } else if (entry.path().filename().string().find("license") !=
                           std::string::npos or
                       entry.path().filename().string().find("LICENSE") !=
                           std::string::npos) {
              std::cout << icons.at("license");
            } else if (icons.find(entry.path().extension()) != icons.end()) {
              std::cout << icons.at(entry.path().extension());
            } else {
              std::cout << icons.at("other");
            }
          }

          if (entry.is_directory()) {
            std::cout << "\033[34;1m";
          }
          std::cout << ' ' << entry.path().filename().string() << "\033[0m\n";

          if (!entry.is_directory()) {
            files++;
          } else {
            dirs++;
            walk(entry.path(), prefix + pointers.second, argv);
          }
        };

    std::function<bool(const std::filesystem::directory_entry &)> isNotHidden =
        [](const std::filesystem::directory_entry &e) {
          return e.path().filename().string().front() != '.';
        };

    if (argv.unsort) {
      for (const std::filesystem::directory_entry &entry :
           std::filesystem::directory_iterator(directory)) {
        loopLogic(
            entry,
            (argv.showHidden
                 ? std::distance(std::filesystem::directory_iterator(directory),
                                 std::filesystem::directory_iterator())
                 : std::count_if(std::filesystem::directory_iterator(directory),
                                 std::filesystem::directory_iterator(),
                                 isNotHidden)));
      }
      return;
    }

    for (const std::filesystem::directory_entry &entry : entries) {
      loopLogic(entry,
                (argv.showHidden ? entries.size()
                                 : std::count_if(entries.begin(), entries.end(),
                                                 isNotHidden)));
    }
  }

  void summary() const {
    std::cout << "\n"
              << dirs << " directories,"
              << " " << files << " files\n";
  }
};

int main(int argc, char *argv[]) {
  args arg;

  try {
    arg.parseArgs(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  ctree tree;

  std::cout << arg.dir << '\n';
  try {
    tree.walk(arg.dir, "", arg);
  } catch (const std::exception &e) {
    std::cout << e.what() << '\n';
    return 2;
  }

  if (arg.summary) {
    tree.summary();
  }

  return 0;
}
