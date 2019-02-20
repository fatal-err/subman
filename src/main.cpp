#include "subtitle.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

auto main(int argc, char **argv) -> int {
  bool is_recursive;
  std::vector<subman::subtitle> subtitles;

  boost::program_options::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "output,o", boost::program_options::value<std::vector<std::string>>(),
      "Output file.")(
      "recursive,r",
      boost::program_options::value<bool>(&is_recursive)->default_value(false),
      "Recursively looking for input files.");

  boost::program_options::positional_options_description positional_options;
  positional_options.add("input-files", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv)
          .options(desc)
          .positional(positional_options)
          .run(),
      vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("input-files")) {
    auto inputFiles = vm["input-files"].as<std::vector<std::string>>();
    subtitles.reserve(inputFiles.size());

    // this function will load and add subtitles to the "subtitles" variable:
    std::function<void(std::string const &)> recursive_handler;
    recursive_handler = [&](std::string const &path) {
      // check if the path is a directory and handle file loading:
      // we will go in trouble of checking if the user has passed "recursive"
      // option to the command line
      if (is_recursive && boost::filesystem::is_directory(path)) {
        for (auto &child : boost::filesystem::directory_iterator(path)) {
          if (boost::filesystem::is_directory(child)) {
            recursive_handler(child.path().string()); // handle subdirectories
            continue;
          }
        }
      }
      if (!boost::filesystem::is_regular_file(path)) {
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
      if (!boost::filesystem::exists(path)) {
        std::cerr << "File '" << path << "' does not exists." << std::endl;
        continue;
      }
      recursive_handler(path);
    }
  } else {
    std::cerr << "Error: no input files specified." << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("output")) {

  } else {
  }

  return EXIT_SUCCESS;
}
