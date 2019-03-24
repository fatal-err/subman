#include "document.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using std::function;
using std::string;
using std::vector;

auto main(int argc, char **argv) -> int {
  bool is_recursive;
  vector<subman::document> subtitles;

  po::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "output,o", po::value<vector<string>>(), "Output file path")(
      "recursive,r", po::value<bool>(&is_recursive)->default_value(false),
      "Recursively looking for input files.")(
      "merge,m", po::value<vector<string>>()->multitoken(),
      "Merge subtitles into one subtitle")(
      "italic,i", po::value<vector<bool>>()->multitoken(), "Italic font")(
      "bold,b", po::value<vector<bool>>()->multitoken(), "Bold font")(
      "color,c", po::value<vector<bool>>()->multitoken(),
      "Colors")("output-format,f", po::value<string>()->default_value("auto"),
                "Output format");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("merge")) {
    auto inputFiles = vm["merge"].as<vector<string>>();
    subtitles.reserve(inputFiles.size());

    // this function will load and add subtitles to the "subtitles" variable:
    function<void(string const &)> recursive_handler;
    recursive_handler = [&](string const &path) {
      // check if the path is a directory and handle file loading:
      // we will go in trouble of checking if the user has passed "recursive"
      // option to the command line
      if (is_recursive && fs::is_directory(path)) {
        for (auto &child : fs::directory_iterator(path)) {
          if (fs::is_directory(child)) {
            recursive_handler(child.path().string()); // handle subdirectories
            continue;
          }
        }
      }
      if (!fs::is_regular_file(path)) {
        std::cerr << "Path '" << path << "' is not a regular file or directory."
                  << std::endl;
        return;
      }

      // it's a regular file so we load it:
      try {
        subtitles.push_back(subman::load(path));
      } catch (std::exception const &) {
        std::cerr << "Error loading '" << path << "'" << std::endl;
      }
    };

    // read every input files/folders:
    for (auto path : inputFiles) {
      if (!fs::exists(path)) {
        std::cerr << "File '" << path << "' does not exists." << std::endl;
        continue;
      }
      recursive_handler(path);
    }
  } else {
    std::cerr << "Nothing to do. type --help for usage." << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("output")) {
    auto format = vm["output-format"].as<string>();
    auto output = vm["output"].as<string>();

    if (format == "auto") {
      format = fs::extension(output);
    }

    try {
      std::ofstream stream(output, std::ios_base::out);
      subman::write_subtitle_to(format, stream, sub);
      stream.close();
    } catch (std::invalid_argument const &err) {
      std::cerr << err.what() << std::endl;
    }

  } else {
  }

  return EXIT_SUCCESS;
}
