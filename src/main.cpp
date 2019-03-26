#include "document.h"
#include "utilities.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <map>


auto main(int argc, char **argv) -> int {
	namespace po = boost::program_options;
	namespace fs = boost::filesystem;
	using std::function;
	using std::string;
	using std::vector;
	using std::map;
	using std::shared_ptr;
	using subman::document;

  bool is_recursive;
  vector<document> inputs;
  map<string, document> outputs;

  po::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "output,o", po::value<vector<string>>()->multitoken(), "Output file path")(
      "recursive,r", po::value<bool>(&is_recursive)->default_value(false),
      "Recursively looking for input files.")(
      "merge,m", po::value<bool>()->default_value(false),
      "Merge subtitles into one subtitle")(
      "italic,i", po::value<vector<bool>>()->multitoken(), "Italic font")(
      "bold,b", po::value<vector<bool>>()->multitoken(), "Bold font")(
      "color,c", po::value<vector<bool>>()->multitoken(),
      "Colors")("output-format,f", po::value<string>()->default_value("auto"),
                "Output format");
  po::positional_options_description inputs_desc;
  inputs_desc.add("inputs", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
	  .positional(inputs_desc)
	  .options(desc)
	  .run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (!vm.count("inputs")) {
	  std::cerr << "Please specify input files. Use --help for more information." << std::endl;
	  return EXIT_FAILURE;
  }
  auto inputFiles = vm["inputs"].as<vector<string>>();
  auto outputFiles = !vm.count("output") ? inputFiles : vm["output"].as<vector<string>>();

  if (vm.count("merge")) {

    // this function will load and add subtitles to the "documents" variable:
    function<void(string const&)> recursive_handler;
    recursive_handler = [&](string const &input_path) {
      // check if the path is a directory and handle file loading:
      // we will go in trouble of checking if the user has passed "recursive"
      // option to the command line
      if (is_recursive && fs::is_directory(input_path)) {
        for (auto &child : fs::directory_iterator(input_path)) {
          if (fs::is_directory(child)) {
            recursive_handler(child.path().string()); // handle subdirectories
            continue;
          }
        }
      }
      if (!fs::is_regular_file(input_path)) {
        std::cerr << "Path '" << input_path << "' is not a regular file or directory."
                  << std::endl;
        return;
      }

      // it's a regular file so we load it:
      try {
        inputs.emplace_back(subman::load(input_path));
      } catch (std::exception const &) {
        std::cerr << "Error loading '" << input_path << "'" << std::endl;
      }
    };

    // read every input files/folders:
    for (string const &input_path : inputFiles) {
      if (!fs::exists(input_path)) {
        std::cerr << "File '" << input_path << "' does not exists." << std::endl;
        continue;
      }
      recursive_handler(input_path);
    }
  } else {
    std::cerr << "Nothing to do. type --help for usage." << std::endl;
    return EXIT_FAILURE;
  }


  auto format = vm["output-format"].as<string>();
  for (auto const& output : outputs) {
	  try {
		  auto &path = output.first;
		  auto &doc = output.second;
		  if (format == "auto") {
			  format = fs::extension(path);
		  }
		  subman::write(doc, path, format);
	  }
	  catch (std::invalid_argument const &err) {
		  std::cerr << err.what() << std::endl;
	  }
  }

  return EXIT_SUCCESS;
}
